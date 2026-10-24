# Reference

Everything is in the module `wordtree.Rope`, apart from `wordtree.Meta`.

```jello
import { Rope, Direction } from wordtree.Rope;
import type { Span } from wordtree.Rope;
```

## Types

| Type | What it is |
|------|------------|
| `Rope` | editable UTF-8 text with a cursor; made with `new Rope(text)` |
| `Span` | `{ start: I32, end: I32 }`: the characters from `start` up to, not including, `end` |
| `Direction` | an enum, `Direction.Forward` or `Direction.Back`, for the seek methods |

## Rope

Positions count characters and are held to the rope. Methods marked "rope" return the rope, so
they chain.

| Method | Result | Does |
|--------|--------|------|
| `new Rope(text: Bytes = "")` | | a rope holding `text`, cursor at 0 |
| `close()` | `Null` | frees the text now; a closed rope traps when used |
| `length()` | `I32` | the number of characters |
| `byteLength()` | `I32` | the number of bytes |
| `text()` | `Bytes` | the whole text |
| `slice(start, end)` | `Bytes` | the text from `start` up to `end` |
| `spanText(span)` | `Bytes` | the text of a span |
| `insert(at, text)` | rope | inserts `text` before position `at`; throws for text that is not UTF-8 |
| `append(text)` | rope | inserts `text` at the end |
| `delete(start, end)` | rope | deletes a range, given in either order |
| `replace(start, end, text)` | rope | replaces a range with `text`; throws for text that is not UTF-8 |
| `clear()` | rope | deletes everything |
| `cursor()` | `I32` | the cursor |
| `moveTo(at)` | `I32` | moves the cursor; where it now is |
| `word(at)` | `Span` | the word at `at`, or the next one |
| `line(at)` | `Span` | the line holding `at`, with its newline |
| `paragraph(at)` | `Span` | the paragraph holding `at`, with the blank line ending it |
| `until(at, delimiter)` | `Span` | from `at` through the next `delimiter` |
| `linePrefix(at, prefix)` | `Span` | the line holding `at` if it starts with `prefix`, else an empty span |
| `seekWord(direction)` | `I32` | moves the cursor a word; where it now is |
| `seekLine(direction)` | `I32` | moves the cursor a line |
| `seekParagraph(direction)` | `I32` | moves the cursor a paragraph |

[Editing](rope-editing.md), [The cursor](cursor-and-seek.md) and [Spans](bounds.md) give the
details.

## wordtree.Meta

| Function | Result |
|----------|--------|
| `version()` | `"0.1.0"` |
