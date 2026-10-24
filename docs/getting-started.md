# Getting started

## What you need

- Jello: `jelloc` and `jello` on your `PATH`.
- A C compiler, to build wordtree's native library: `cc` on macOS and Linux, or llvm-mingw's `clang` on Windows (run `build.sh` from Git Bash there).

## Install

Add wordtree to the `dependencies` of your project's `jello.json`:

```json
{
  "name": "notes",
  "version": "0.1.0",
  "dependencies": {
    "wordtree": {"github": "Lazarus404/wordtree", "tag": "0.1.0"}
  }
}
```

Fetch it, then build its native library, `wordtree.jdll`:

```sh
jelloc deps.get
packages/wordtree/build.sh
```

`build.sh` finds Jello's C headers through `JELLO_ROOT`, if you set it, or else beside the `jello` on your `PATH`. Set `CC` to choose another compiler. The library lands in `packages/wordtree/jdll/`, where `jelloc run` and `jelloc test` look for it.

## A first rope

Put this in `src/Main.jello`:

```jello
import { Rope } from wordtree.Rope;

let r = new Rope("Hello world");
r.insert(5, ",");
r.append("!");
r.text()
```

and run it:

```sh
$ jelloc run
Hello, world!
```

`new Rope(text)` makes a rope holding `text`; `new Rope()` makes an empty one. The editing methods return the rope, so they chain: `r.insert(5, ",").append("!")` does the same as the two lines above.

When you are done with a rope, you may `close` it to free its text at once. You need not: a rope nobody holds is freed when the garbage collector finds it.

## Next

[Editing](rope-editing.md) covers every way to change a rope's text.
