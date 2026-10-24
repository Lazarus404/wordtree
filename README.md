# wordtree

**wordtree** gives Jello programs editable UTF-8 text: a *rope*, which stays quick to change in the middle however long it grows. It suits editors, terminals and any tool that edits a document in place. Underneath is [librope](https://github.com/josephg/librope), a small C library, which wordtree wraps in a `Rope` with Jello methods, character positions and a cursor.

**Version:** 0.1.0

```jello
import { Rope, Direction } from wordtree.Rope;

let r = new Rope("Hello world");
r.insert(5, ",");                      // "Hello, world"
let word = r.spanText(r.word(8));      // "world"
r.moveTo(0);
r.seekWord(Direction.Forward);         // the cursor is now 7, at "world"
```

## Install

Add wordtree to your project's `jello.json`:

```json
"dependencies": {
  "wordtree": {"github": "Lazarus404/wordtree", "tag": "0.1.0"}
}
```

Then fetch it and build its native library, which needs a C compiler:

```sh
jelloc deps.get
packages/wordtree/build.sh
```

`jelloc run` and `jelloc test` find the library in `packages/wordtree/jdll/` on their own.

## What you get

| Method | Does |
|--------|------|
| `new Rope(text)` | a rope holding `text`, empty by default |
| `insert`, `append`, `delete`, `replace`, `clear` | edit the text |
| `length`, `byteLength`, `text`, `slice`, `spanText` | read it |
| `cursor`, `moveTo` | read and move the cursor |
| `word`, `line`, `paragraph`, `until`, `linePrefix` | find a `Span` around a position |
| `seekWord`, `seekLine`, `seekParagraph` | move the cursor by a word, line or paragraph |
| `close` | free the text now, rather than when the rope is collected |

Positions count characters, not bytes; see [UTF-8 and positions](docs/utf8-and-indices.md).

## Documentation

The [guide](docs/README.md) starts at the beginning and covers everything:

| Guide | Covers |
|-------|--------|
| [Getting started](docs/getting-started.md) | Installing, building and a first rope |
| [Editing](docs/rope-editing.md) | Inserting, deleting, replacing and reading text |
| [UTF-8 and positions](docs/utf8-and-indices.md) | Characters, bytes and what a position means |
| [The cursor](docs/cursor-and-seek.md) | The cursor, how edits move it, and seeking |
| [Spans](docs/bounds.md) | Words, lines, paragraphs and delimiters |
| [How it works](docs/architecture.md) | The native library, handles and threads |
| [Reference](docs/modules.md) | Every type and method |
| [Testing](docs/testing.md) | Running the tests |

## Development

```sh
./build.sh      # builds jdll/wordtree.jdll
./test.sh       # builds, then runs the unit tests, the examples and the programs
```

Both need Jello's `jelloc` and `jello` on `PATH`. The examples in `examples/` run as tests, so they always work as written.

## Licence

The wordtree library is under the BSD 3-Clause licence. librope, vendored in `jdll/librope/`, is Joseph Gentle's, under its own licence; see [its repository](https://github.com/josephg/librope).
