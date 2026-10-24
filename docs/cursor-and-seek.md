# The cursor

Each rope keeps a *cursor*: a position, as a caret is in an editor. Use it if it helps; every
method that finds something also takes a position of your own, so you can keep your caret
elsewhere.

## Reading and moving it

```jello
let r = new Rope("abc");
r.cursor();      // 0: a new rope's cursor is at the start
r.moveTo(2);     // 2
r.moveTo(99);    // 3: held to the end
```

`moveTo` gives where the cursor ends up.

## How edits move it

| Edit | The cursor |
|------|------------|
| `insert`, `append` | stays where it was, even when the text goes in before it |
| `delete` | inside the range, moves to its start; after it, moves back by the characters deleted |
| `replace` | moves to the start of the range |
| `clear` | moves to 0 |

`insert` leaves the cursor alone so that it does not surprise you; an editor typing at its caret moves the caret itself:

```jello
r.insert(r.cursor(), "x");
r.moveTo(r.cursor() + 1);
```

## Seeking

The seek methods move the cursor by a word, a line or a paragraph, and give its new position. They take a `Direction`, `Direction.Forward` or `Direction.Back`:

```jello
import { Rope, Direction } from wordtree.Rope;

let r = new Rope("one two three");
r.seekWord(Direction.Forward);   // 4, the start of "two"
r.seekWord(Direction.Forward);   // 8, the start of "three"
r.seekWord(Direction.Forward);   // 13, the end
r.seekWord(Direction.Back);      // 8
```

- **`seekWord`** forward goes to the start of the next word, or to the end from the last word. Back goes to the start of the word before the one the cursor is in, or to 0 when there is none; from the end of the text, the word before is the last word.
- **`seekLine`** forward goes to the start of the next line, or to the end from the last line. Back goes to the start of the line before, or to the start of the first line.
- **`seekParagraph`** moves by paragraphs as `seekLine` moves by lines.

Words, lines and paragraphs are what the [span](bounds.md) methods find.
