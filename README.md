LLAAM: Low-Level Abstracting Abstract Machine

Supported Instructions
===
LLAAM is still early work in progress. For now, it supports `ReturnInst`, `CallInst` (with special treatments for `malloc` and `free`), `LoadInst`, `StoreInst`, `AllocaInst`, `GetElementPtrInst`, `Add`, `Sub`, `Mul`, `ICmpInst`, `BranchInst`, `BitCastInst`, `SExtInst`, `ZExtInst`, `TruncInst`, and `PHINode`.

How to build
===
LLAAM works as a pass in LLVM, it has been tested on LLVM 3.9.1.

To build it:

* clone the project to `src_of_llvm/lib/Transforms/AAM`
* modify `src_of_llvm/lib/Transforms/CMakeLists.txt` file, add `add_subdirectory(AAM)` to the end of file
* rebuild the LLVM, `LLVMAAM.dylib` should appeared in `build_of_llvm/lib/` directroy

Then you can use command line tool `opt` to invoke it:

`opt -load build_of_llvm/lib/LLVMAAM.dylib -aam < code.ll > /dev/null`.

