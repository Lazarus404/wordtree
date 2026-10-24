# UTF-8 and positions

## A position counts characters

Every position wordtree takes or gives counts *characters*, by which it means Unicode code points, not bytes. In `"caf\u{e9}"`, which is "cafe" with an acute accent on the "e", the accented letter is position 3, though its UTF-8 takes two bytes. So:

```jello
let r = new Rope("caf\u{e9}s");
r.length();        // 5 characters
r.byteLength();    // 6 bytes
r.slice(3, 5);     // "\u{e9}s"
```

On ASCII text the two lengths are the same. Positions are `I32`s.

## Positions are held to the rope

A position below 0 means 0, and one past the end means the end. Nothing throws for a position out of range: `slice(-3, 99)` is the whole text, and `moveTo(99)` puts the cursor at the end.

## A character is a code point

librope counts code points. A letter written with a combining accent, or an emoji made of several code points, is several characters to wordtree, as it is to `Bytes.utf8Length`. wordtree does not normalise text, either: it keeps exactly the bytes it is given.

## The text must be UTF-8

`insert` and `replace` take text only when it is valid UTF-8 with no zero bytes; anything else throws, and the rope is unchanged. `Bytes.utf8Validate(text)` says ahead of time whether text will do.
