# Spans

The span methods answer one question: given a position, where does the word, line or paragraph around it start and end? They give a `Span`:

```jello
typedef Span = { let start: I32; let end: I32; };
```

`start` is the first character and `end` the one after the last, as `slice` takes them, so `r.spanText(span)` is `r.slice(span.start, span.end)`. An empty span has `start == end`.

On an empty rope, every span method gives `{ start: 0, end: 0 }`.

## Words

```jello
let r = new Rope("say hello there");
r.spanText(r.word(5));   // "hello"
```

A word is a run of anything but whitespace, so punctuation is part of the word beside it: `"wait...what"` is one word. Whitespace is space, tab, newline, carriage return, form feed and vertical tab.

| The position is | `word` gives |
|-----------------|--------------|
| in a word | that word |
| on whitespace before a word | the next word |
| on whitespace with no word after it | an empty span at the position |

## Lines

`line(at)` gives the line holding `at`, with the newline that ends it. The last line has no newline unless the text ends with one:

```jello
let r = new Rope("first\nsecond");
r.spanText(r.line(0));   // "first\n"
r.spanText(r.line(8));   // "second"
```

## Paragraphs

A blank line, two newlines in a row, ends a paragraph, and belongs to the paragraph it ends. Text with no blank line is one paragraph.

```jello
let r = new Rope("para one\n\npara two");
r.spanText(r.paragraph(3));    // "para one\n\n"
r.spanText(r.paragraph(12));   // "para two"
```

## Up to a delimiter

`until(at, delimiter)` runs from `at` through the end of the next `delimiter`, or to the end of the text when no delimiter follows, or when `delimiter` is empty:

```jello
let r = new Rope("abc---def");
r.spanText(r.until(2, "---"));   // "c---"
```

## Lines with a prefix

`linePrefix(at, prefix)` gives the line holding `at`, as `line` does, when it starts with `prefix`, and otherwise an empty span at the line's start. An empty prefix matches no line. It finds headings and quoted lines without wordtree knowing anything about Markdown:

```jello
let r = new Rope("# Title\nBody");
r.spanText(r.linePrefix(2, "#"));   // "# Title\n"
r.linePrefix(9, "#");               // { start: 8, end: 8 }
```

## Positions at the end

A position at or past the end of the text is taken as the last character, so `r.line(r.length())` is the last line.
