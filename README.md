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

## Chapter 2 Basic numbers: Main function
This chapter adds infrastructure to generate functions (main function in this case). Script to run and test the implementation  is added (`compile-run.sh`)

The output of the script should give the following:
```
saankhya@araj:~/eva-llvm$ compile-run.sh
; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"

define i32 @main() {
entry:
  ret i32 42
}
42
```
## Chapter 3 Strings: Extern calls
