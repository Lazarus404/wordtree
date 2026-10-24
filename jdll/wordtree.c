/* SPDX-License-Identifier: BSD-3-Clause */

/**
 * @file wordtree.c
 * @brief Ropes of UTF-8 text for Jello, on librope.
 *
 * A rope is a handle whose payload holds a librope rope and a cursor. Positions are character
 * (code point) indices, clamped to the rope; a span is two of them, START up to but not
 * including END, returned as eight bytes: two little-endian I32s.
 *
 * librope keeps no lock, so the handle class has no `retain`: a rope stays in the process that
 * made it.
 */

#include <jello/jdll.h>

#include "librope/rope.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

// A rope and its cursor; `rope` is NULL once the rope is closed.
typedef struct wordtree_rope {
  rope* rope;
  int32_t cursor;
} wordtree_rope;

// Frees a rope's payload, when no handle holds it any more.
static void wordtree_finalize(void* payload) {
  wordtree_rope* wt = payload;
  if (wt->rope) rope_free(wt->rope);
  free(wt);
}

static const jello_jdll_handle_class ROPE = {"wordtree rope", wordtree_finalize, NULL};

// The open rope of argument 0, or NULL once the call has failed.
static wordtree_rope* arg_rope(jdlo_ctx* c) {
  wordtree_rope* wt = jdl_arg_handle(c, 0, &ROPE);
  if (wt && !wt->rope) {
    jdl_fail(c, "the rope is closed");
    return NULL;
  }
  return wt;
}

// The number of characters in a rope, as an I32.
static int32_t char_count(const wordtree_rope* wt) {
  size_t n = rope_char_count(wt->rope);
  return n > INT32_MAX ? INT32_MAX : (int32_t)n;
}

// POS clamped to the rope, from 0 to its length.
static int32_t clamp(const wordtree_rope* wt, int32_t pos) {
  int32_t n = char_count(wt);
  return pos < 0 ? 0 : pos > n ? n : pos;
}

// Whether a UTF-8 byte is the first of its character.
static int is_lead(uint8_t b) {
  return (b & 0xC0u) != 0x80u;
}

// Whether a byte is ASCII whitespace; no other byte of UTF-8 is.
static int is_space(uint8_t b) {
  return b == ' ' || b == '\t' || b == '\n' || b == '\r' || b == '\f' || b == '\v';
}

// A rope's text, copied out once so that a scan can read it as bytes.
typedef struct text {
  uint8_t* data;
  size_t len;
  size_t chars;
} text;

// Copies out a rope's text; 0 when memory runs out, which fails the call.
static int text_of(jdlo_ctx* c, wordtree_rope* wt, text* t) {
  t->len = rope_byte_count(wt->rope);
  t->chars = rope_char_count(wt->rope);
  t->data = rope_create_cstr(wt->rope);
  if (!t->data) {
    jdl_fail(c, "out of memory");
    return 0;
  }
  return 1;
}

// The byte offset of character CHARS.
static size_t byte_at(const text* t, size_t chars) {
  size_t i = 0;
  for (size_t seen = 0; i < t->len; i++) {
    if (is_lead(t->data[i]) && seen++ == chars) break;
  }
  return i;
}

// The number of characters before byte offset AT.
static size_t char_at(const text* t, size_t at) {
  size_t chars = 0;
  for (size_t i = 0; i < at && i < t->len; i++) chars += is_lead(t->data[i]);
  return chars;
}

// A span of byte offsets.
typedef struct span {
  size_t start;
  size_t end;
} span;

// The byte offset of character POS, held to the last character, as the bounds take it.
static size_t bound_start(const text* t, int32_t pos) {
  size_t chars = (size_t)pos;
  return byte_at(t, chars >= t->chars ? t->chars - 1 : chars);
}

/**
 * The word at byte AT, or the next word when AT is on whitespace; an empty span at AT when only
 * whitespace follows. A word is a run of anything but whitespace.
 */
static span word_at(const text* t, size_t at) {
  size_t i = at;
  while (i < t->len && is_space(t->data[i])) i++;
  if (i == t->len) return (span){at, at};
  size_t start = i;
  while (start > 0 && !is_space(t->data[start - 1])) start--;
  while (i < t->len && !is_space(t->data[i])) i++;
  return (span){start, i};
}

// The line holding byte AT, with its newline if it has one.
static span line_at(const text* t, size_t at) {
  size_t start = at;
  while (start > 0 && t->data[start - 1] != '\n') start--;
  size_t end = at;
  while (end < t->len && t->data[end] != '\n') end++;
  if (end < t->len) end++;
  return (span){start, end};
}

/**
 * The paragraph holding byte AT: paragraphs end with a blank line, "\n\n", which belongs to the
 * paragraph before it.
 */
static span paragraph_at(const text* t, size_t at) {
  size_t start = at;
  while (start > 0 && !(start >= 2 && t->data[start - 1] == '\n' && t->data[start - 2] == '\n') &&
         !(start == 1 && t->data[0] == '\n')) {
    start--;
  }
  size_t end = at;
  while (end < t->len && !(end + 1 < t->len && t->data[end] == '\n' && t->data[end + 1] == '\n')) {
    end++;
  }
  if (end < t->len) end += 2;
  return (span){start, end};
}

// Returns a span of byte offsets as one of characters.
static void return_span(jdlo_ctx* c, const text* t, span s) {
  int32_t pair[2] = {(int32_t)char_at(t, s.start), (int32_t)char_at(t, s.end)};
  uint8_t out[8];
  for (int i = 0; i < 2; i++) {
    uint32_t v = (uint32_t)pair[i];
    for (int b = 0; b < 4; b++) out[i * 4 + b] = (uint8_t)(v >> (8 * b));
  }
  jdl_return_bytes_copy(c, out, sizeof out);
}

// Inserts TEXT at character POS; 0 when it is not UTF-8 or holds a zero byte.
static int insert_text(jdlo_ctx* c, wordtree_rope* wt, int32_t pos, const uint8_t* data,
                       uint32_t len) {
  if (len == 0) return 1;
  if (memchr(data, 0, len)) return 0;
  uint8_t* copy = malloc((size_t)len + 1);
  if (!copy) {
    jdl_fail(c, "out of memory");
    return 1;
  }
  memcpy(copy, data, len);
  copy[len] = 0;
  ROPE_RESULT rc = rope_insert(wt->rope, (size_t)pos, copy);
  free(copy);
  return rc == ROPE_OK;
}

// Deletes the characters from START up to END, in either order, and moves the cursor with them.
static void delete_range(wordtree_rope* wt, int32_t start, int32_t end) {
  start = clamp(wt, start);
  end = clamp(wt, end);
  if (end < start) {
    int32_t swap = start;
    start = end;
    end = swap;
  }
  rope_del(wt->rope, (size_t)start, (size_t)(end - start));
  if (wt->cursor > end) {
    wt->cursor -= end - start;
  } else if (wt->cursor > start) {
    wt->cursor = start;
  }
}

// A new, empty rope.
static void wt_new(jdlo_ctx* c) {
  wordtree_rope* wt = calloc(1, sizeof *wt);
  if (wt) wt->rope = rope_new();
  if (!wt || !wt->rope) {
    free(wt);
    jdl_fail(c, "out of memory");
    return;
  }
  jdl_return_handle(c, &ROPE, wt, sizeof *wt + sizeof(rope));
}

// Frees a rope's text now, rather than when its handle is collected.
static void wt_close(jdlo_ctx* c) {
  wordtree_rope* wt = jdl_arg_handle(c, 0, &ROPE);
  if (wt && wt->rope) {
    rope_free(wt->rope);
    wt->rope = NULL;
  }
}

// Empties a rope and puts its cursor at 0.
static void wt_clear(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  if (!wt) return;
  rope_del(wt->rope, 0, rope_char_count(wt->rope));
  wt->cursor = 0;
}

// Inserts text at a position; false when the text is not UTF-8 or holds a zero byte.
static void wt_insert(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  if (!wt) return;
  int32_t pos = clamp(wt, jdl_arg_i32(c, 1));
  jdl_return_bool(c, insert_text(c, wt, pos, jdl_arg_bytes_data(c, 2), jdl_arg_bytes_len(c, 2)));
}

// Deletes a range of characters.
static void wt_delete(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  if (wt) delete_range(wt, jdl_arg_i32(c, 1), jdl_arg_i32(c, 2));
}

/**
 * Replaces a range of characters with text and puts the cursor at its start; false, changing
 * nothing, when the text is not UTF-8 or holds a zero byte.
 */
static void wt_replace(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  if (!wt) return;
  int32_t start = clamp(wt, jdl_arg_i32(c, 1));
  int32_t end = clamp(wt, jdl_arg_i32(c, 2));
  if (end < start) {
    int32_t swap = start;
    start = end;
    end = swap;
  }
  // Inserting after the range first leaves the rope as it was if the text is refused.
  if (!insert_text(c, wt, end, jdl_arg_bytes_data(c, 3), jdl_arg_bytes_len(c, 3))) {
    jdl_return_bool(c, 0);
    return;
  }
  delete_range(wt, start, end);
  wt->cursor = start;
  jdl_return_bool(c, 1);
}

// The number of characters.
static void wt_length(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  if (wt) jdl_return_i32(c, char_count(wt));
}

// The number of bytes of the text.
static void wt_byte_length(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  if (!wt) return;
  size_t n = rope_byte_count(wt->rope);
  jdl_return_i32(c, n > INT32_MAX ? INT32_MAX : (int32_t)n);
}

// The text from character START up to END; empty when END is before START.
static void wt_slice(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  if (!wt) return;
  int32_t start = clamp(wt, jdl_arg_i32(c, 1));
  int32_t end = clamp(wt, jdl_arg_i32(c, 2));
  text t;
  if (end <= start) {
    jdl_return_bytes_copy(c, NULL, 0);
  } else if (text_of(c, wt, &t)) {
    size_t from = byte_at(&t, (size_t)start);
    jdl_return_bytes_copy(c, t.data + from, (uint32_t)(byte_at(&t, (size_t)end) - from));
    free(t.data);
  }
}

// The whole text.
static void wt_text(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  text t;
  if (wt && text_of(c, wt, &t)) {
    jdl_return_bytes_copy(c, t.data, (uint32_t)t.len);
    free(t.data);
  }
}

// The cursor, held to the rope.
static void wt_cursor(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  if (wt) jdl_return_i32(c, clamp(wt, wt->cursor));
}

// Moves the cursor, held to the rope; where it now is.
static void wt_set_cursor(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  if (!wt) return;
  wt->cursor = clamp(wt, jdl_arg_i32(c, 1));
  jdl_return_i32(c, wt->cursor);
}

// The kinds of span the bound export finds.
enum { BOUND_WORD, BOUND_LINE, BOUND_PARAGRAPH, BOUND_UNTIL, BOUND_LINE_PREFIX };

/**
 * The span of a kind at a position: arguments are the rope, the kind, the position and, for
 * BOUND_UNTIL and BOUND_LINE_PREFIX, the delimiter or prefix.
 */
static void wt_bound(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  text t;
  if (!wt || !text_of(c, wt, &t)) return;
  int32_t kind = jdl_arg_i32(c, 1);
  int32_t pos = clamp(wt, jdl_arg_i32(c, 2));
  const uint8_t* needle = jdl_arg_bytes_data(c, 3);
  size_t needle_len = jdl_arg_bytes_len(c, 3);
  span s = {0, 0};
  if (t.chars > 0) {
    size_t at = bound_start(&t, pos);
    switch (kind) {
      case BOUND_WORD:
        s = word_at(&t, at);
        break;
      case BOUND_LINE:
        s = line_at(&t, at);
        break;
      case BOUND_PARAGRAPH:
        s = paragraph_at(&t, at);
        break;
      case BOUND_UNTIL:
        s = (span){at, t.len};
        for (size_t i = at; needle_len > 0 && i + needle_len <= t.len; i++) {
          if (memcmp(t.data + i, needle, needle_len) == 0) {
            s.end = i + needle_len;
            break;
          }
        }
        break;
      default:
        s = line_at(&t, at);
        if (needle_len == 0 || s.end - s.start < needle_len ||
            memcmp(t.data + s.start, needle, needle_len) != 0) {
          s.end = s.start;
        }
        break;
    }
  }
  return_span(c, &t, s);
  free(t.data);
}

// The kinds of unit the seek export moves by.
enum { SEEK_WORD, SEEK_LINE, SEEK_PARAGRAPH };

// The start of the unit of a kind before the one starting at byte AT, or 0 when it is first.
static size_t unit_before(const text* t, int kind, size_t at) {
  if (kind == SEEK_WORD) {
    while (at > 0 && is_space(t->data[at - 1])) at--;
    while (at > 0 && !is_space(t->data[at - 1])) at--;
    return at;
  }
  if (at == 0) return 0;
  return kind == SEEK_LINE ? line_at(t, at - 1).start : paragraph_at(t, at - 1).start;
}

/**
 * Moves the cursor by one unit of a kind: forward to the start of the next, or back to the
 * start of the one before; arguments are the rope, the kind and the direction, which is above
 * 0 for forward. Where the cursor now is.
 */
static void wt_seek(jdlo_ctx* c) {
  wordtree_rope* wt = arg_rope(c);
  text t;
  if (!wt || !text_of(c, wt, &t)) return;
  int32_t kind = jdl_arg_i32(c, 1);
  int forward = jdl_arg_i32(c, 2) > 0;
  if (t.chars > 0) {
    size_t at = bound_start(&t, clamp(wt, wt->cursor));
    span s = kind == SEEK_WORD ? word_at(&t, at) : kind == SEEK_LINE ? line_at(&t, at)
                                                                     : paragraph_at(&t, at);
    size_t to;
    if (forward) {
      to = kind == SEEK_WORD && s.end < t.len ? word_at(&t, s.end).start : s.end;
    } else if (kind == SEEK_WORD && (size_t)clamp(wt, wt->cursor) == t.chars) {
      // From the end, the word before is the last word.
      to = unit_before(&t, kind, t.len);
    } else {
      to = unit_before(&t, kind, s.start);
    }
    wt->cursor = (int32_t)char_at(&t, to);
  }
  free(t.data);
  jdl_return_i32(c, clamp(wt, wt->cursor));
}

static const jello_jdll_export EXPORTS[] = {
    {"rope_new", wt_new, JELLO_T_DYNAMIC, 0, {0}, 0},
    {"rope_close", wt_close, JELLO_T_VOID, 1, {JELLO_T_DYNAMIC}, 0},
    {"rope_clear", wt_clear, JELLO_T_VOID, 1, {JELLO_T_DYNAMIC}, 0},
    {"rope_insert", wt_insert, JELLO_T_BOOL, 3, {JELLO_T_DYNAMIC, JELLO_T_I32, JELLO_T_BYTES}, 0},
    {"rope_delete", wt_delete, JELLO_T_VOID, 3, {JELLO_T_DYNAMIC, JELLO_T_I32, JELLO_T_I32}, 0},
    {"rope_replace", wt_replace, JELLO_T_BOOL, 4,
     {JELLO_T_DYNAMIC, JELLO_T_I32, JELLO_T_I32, JELLO_T_BYTES}, 0},
    {"rope_length", wt_length, JELLO_T_I32, 1, {JELLO_T_DYNAMIC}, 0},
    {"rope_byte_length", wt_byte_length, JELLO_T_I32, 1, {JELLO_T_DYNAMIC}, 0},
    {"rope_slice", wt_slice, JELLO_T_BYTES, 3, {JELLO_T_DYNAMIC, JELLO_T_I32, JELLO_T_I32}, 0},
    {"rope_text", wt_text, JELLO_T_BYTES, 1, {JELLO_T_DYNAMIC}, 0},
    {"rope_cursor", wt_cursor, JELLO_T_I32, 1, {JELLO_T_DYNAMIC}, 0},
    {"rope_set_cursor", wt_set_cursor, JELLO_T_I32, 2, {JELLO_T_DYNAMIC, JELLO_T_I32}, 0},
    {"rope_bound", wt_bound, JELLO_T_BYTES, 4,
     {JELLO_T_DYNAMIC, JELLO_T_I32, JELLO_T_I32, JELLO_T_BYTES}, 0},
    {"rope_seek", wt_seek, JELLO_T_I32, 3, {JELLO_T_DYNAMIC, JELLO_T_I32, JELLO_T_I32}, 0},
};

JELLO_JDLL_LIBRARY(wordtree, EXPORTS);
