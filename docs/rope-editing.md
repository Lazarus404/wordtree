# Editing

Positions on this page count characters; see [UTF-8 and positions](utf8-and-indices.md). Each editing method returns the rope, so calls chain.

## Inserting

```jello
let r = new Rope("ab");
r.insert(1, "Z");       // "aZb"
r.insert(3, "cd");      // "aZbcd"
r.append("!");          // "aZbcd!"
```

`insert(at, text)` puts `text` before the character at `at`. A position below 0 means the start, and one past the end means the end, so `insert(99, ...)` appends, as `append` does. Inserting empty text changes nothing.

The text must be UTF-8 with no zero bytes, which librope cannot hold. Anything else throws, leaving the rope as it was:

```jello
let r = new Rope("ok");
let message = try { r.insert(0, "\xff"); "" } catch (e) { e as Bytes };
// message is "Rope.insert: the text is not UTF-8", and r still holds "ok"
```

## Deleting

```jello
let r = new Rope("Hello world");
r.delete(5, 11);        // "Hello"
```

`delete(start, end)` removes the characters from `start` up to, but not including, `end`. The two may come in either order, so `delete(11, 5)` does the same. An empty range, `delete(2, 2)`, changes nothing. `clear()` deletes everything.

## Replacing

```jello
let r = new Rope("Hello world");
r.replace(6, 11, "there");   // "Hello there"
```

`replace(start, end, text)` deletes the range and inserts `text` where it was. Replacing with empty text only deletes. Text that is not UTF-8 throws, as with `insert`, and the rope is left as it was.

## Reading

| Method | Gives |
|--------|-------|
| `text()` | the whole text, as `Bytes` |
| `slice(start, end)` | the text from `start` up to `end`; empty when `end` is not after `start` |
| `spanText(span)` | the text of a [span](bounds.md) |
| `length()` | the number of characters |
| `byteLength()` | the number of bytes the text takes |

`text()` copies the whole rope out, so in a loop prefer `slice` for the part you need.

## The cursor moves with edits

A rope keeps a cursor, and deletes and replaces keep it in step with the text around it. See [The cursor](cursor-and-seek.md#how-edits-move-it).
