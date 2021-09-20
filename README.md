# Examples of 3 program analysis tools for C

Example of using 3 program analysis tools for refactoring C code.
These are the tools that have some features that are necessary for refactoring and are well-supported (active authors and development).
Try it out! Leave a PR if you have any questions/suggestions!

Each tool has a unique set of benefits and drawbacks.
I chose these 3 tools because I think they are all well-designed, and want to help the community discover/use these wonderful authors' work.

Here are the 3 tools:

* [srcML: an infrastructure for the exploration, analysis, and manipulation of source code](https://www.srcml.org/)
* [Joern: The Bug Hunter's Workbench](https://joern.io/)
* [LLVM: a collection of modular and reusable compiler and toolchain technologies](https://llvm.org/)

## Task

The trial task is to convert a [for loop](https://en.cppreference.com/w/cpp/language/for) to a [while loop](https://en.cppreference.com/w/cpp/language/while).
All for loops can be converted to while loops.
This is a simple task, but it requires several non-trivial capabilities of the program analysis tool:

* Parse source code to an AST
* Granular AST nodes (required to access the init-statement, conditional, and iteration-expression)
* Multiple source code edits

As we will see, this exposes some differences between the tools in capability and ergonomics.

## Example

<table>
<tr>
<th>Original for loop</th>
<th>Expected output (converted while loop)</th>
</tr>
<tr>
<td>

```c
int main()
{
    int x = 0;
    for (int i = 0; i < 10; i ++)
    {
        x += 1;
    }
    return x;
}
```

</td>
<td>

```c
int main()
{
    int x = 0;
    int i = 0;
    while (i < 10) {
      x += 1;
      i++;
    }
    return x;
}
```

</td>
</tr>
</table>

## Organization

Tool source codes are in folders named `srcml`, `joern`, and `llvm`. One sample input program is in `data`. Expected output is as depicted above.

## Running the examples

Here are rough instructions for running the examples.
`timing.sh` runs all tools and collects timing.
Expected output is in the table above (on the right).

* `srcml_example.py` requires srcML, Python 3, and `lxml`.
  1. Install srcML according to the instructions at https://www.srcml.org/#download
  2. Assuming Python 3 is installed, install libs with `pip install -r joern/requirements.txt`
  3. Run example with `python3 srcml/srcml_example.py data/loop_exchange.c`
* `joern_example.sc` requires Joern to be installed.
  1. Install Joern according to the instructions at https://docs.joern.io/installation
  2. Run example with `~/bin/joern/joern-cli/joern --script joern/joern_example.sc --params inDirectory=data,inFile=data/loop_exchange.c`
* `llvm-example` must be built with LLVM 12.0.1. It's implemented as a Clang tool named `loop-convert` based on LibTooling.
  1. To build the example, clone [llvm-project@llvmorg-12.0.1](https://github.com/llvm/llvm-project/tree/llvmorg-12.0.1) to `llvm/llvm-project`.
  2. Add `loop-convert` to the build with `echo 'add_subdirectory(loop-convert)' >> llvm/llvm-project/clang-tools-extra/CMakeLists.txt`
  3. Build LLVM according to the instructions in the [LibASTMatchers Tutorial](https://releases.llvm.org/12.0.1/tools/clang/docs/LibASTMatchersTutorial.html#step-0-obtaining-clang)
  4. For convenience, prebuilt executable `llvm/llvm-example` is included. It was built on Ubuntu Docker
  5. Run example with `llvm/llvm-example data/loop_exchange.c --`

## Timing comparison

Here are the results from running `timing.sh`. Units are in seconds.
Timings were collected running on [Ubuntu Docker](https://hub.docker.com/_/ubuntu), version 20.04.3 LTS.

| Stat          | srcML  | Joern  | LLVM   |
|---------------|--------|--------|--------|
| Runtime (sec), averaged over 5 runs | 0.0702 | 6.2906 | 0.0230 |
