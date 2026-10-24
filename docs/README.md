# The wordtree guide

wordtree keeps editable UTF-8 text in a *rope*. These pages explain it from the beginning; read
them in order, or go straight to the one you need.

1. [Getting started](getting-started.md): install wordtree, build its native library, and make a
   first rope.
2. [Editing](rope-editing.md): insert, delete, replace and read text.
3. [UTF-8 and positions](utf8-and-indices.md): what a position counts, and why it is not a byte.
4. [The cursor](cursor-and-seek.md): the position a rope keeps for you, and moving it a word,
   line or paragraph at a time.
5. [Spans](bounds.md): finding the word, line or paragraph around a position.
6. [How it works](architecture.md): the native library underneath, handles, memory and threads.
7. [Reference](modules.md): every type and method on one page.
8. [Testing](testing.md): how wordtree is tested, and how to run the tests.

Back to the [README](../README.md).
