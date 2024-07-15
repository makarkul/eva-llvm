# Programming Language with LLVM 
Exercise from Dmitry Soshnikov's course on YouTube

## Chapter 1 LLVM Program structure
On the root folder, compile with following command:
```
clang++ -o eva-llvm `llvm-config --cxxflags --ldflags --system-libs --libs core` src/eva-llvm.cpp
```
It should print the name of module and generate `out.ll` with the same contents
```
saankhya@araj:~/eva-llvm$ ./eva-llvm
; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"
```
