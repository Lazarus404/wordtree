# Testing

```sh
./test.sh
```

`test.sh` builds `jdll/wordtree.jdll`, fetches the test dependency, [degust](https://github.com/Lazarus404/degust), if `packages/` lacks it, and runs `jelloc test`. That compiles every test into one binary and runs:

- **the unit tests**, in `tests/wordtree/Rope.test.jello`, on `std.testing`: one test function for each area of the API, from inserting to seeking;
- **the examples**, every program in `examples/`, which degust compiles and runs, checking each prints what its `// expect:` lines say, with the JIT and without;
- **the programs** in `tests/programs/`, for what a unit test cannot check, such as a closed rope stopping the program with a trap.

Arguments to `test.sh` go to the test binary:

```sh
./test.sh -v                      # every test's result, not only failures
./test.sh --run wordtree/Rope     # only the unit tests
./test.sh --run Programs          # only the examples and programs
```

To add an example, write a program in `examples/` that starts with `// expect:` lines giving its output; it is a test from then on.
