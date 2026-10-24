# How it works

wordtree has two halves:

```text
 your program
      |
 wordtree.Rope       Jello: the Rope prototype, Span, Direction, and the checks
      |                     that throw on text librope cannot take
 wordtree.jdll       C: the rope, its cursor, and the scans that find spans
      |
 librope             C: the rope itself, vendored in jdll/librope
```

## The native library

`wordtree.jdll` is a Jello native library, built from `jdll/wordtree.c` and librope by `build.sh`. `wordtree.Rope` loads its functions and gives each a Jello type, and the `Rope` prototype wraps them in methods. A program never sees the natives themselves.

The span and seek methods copy the rope's text out once per call and scan it as bytes, so each takes time in proportion to the length of the text. librope does the inserts and deletes, which take time in proportion to the logarithm of the length.

## Handles and memory

A rope's native side, its librope rope and its cursor, is a *native handle*, held by the `Rope` in its `handle` field. To Jello the handle has an abstract type: it can only be passed back to wordtree's natives.

The garbage collector frees a rope's native side once nothing holds the `Rope`. `close()` frees the text at once instead, which is worth doing when a program opens and closes many large documents. Using a rope after closing it is a bug, and the program stops with the trap
`TYPE_MISMATCH`; closing it twice does nothing.

## Threads and processes

librope keeps no lock, so a rope belongs to the process that made it. Sending one to another process, or capturing one in a function another process runs, stops the program with the trap `UNSENDABLE`. To share a document between processes, send its text.
