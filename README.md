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
Here we learn how to use strings in LLVM language and create extern function calls. In the last commit we see how to invoke these extern functions referenced in the code. `printf` is used as exemplary function. Towards the end you should see the following output on executing the script `compile-run.sh`

```
aankhya@araj:~/eva-llvm$ compile-run.sh
; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"

@0 = private unnamed_addr constant [15 x i8] c"Hello, world!\0A\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i32 0, i32 0))
  ret i32 %0
}
Hello, world!
14
```

Notice the `getelementptr` call, this will be discussed in the next chapters

## Chapter 4 Parsing : S-expressions
In this chapter we add basic parser for the grammar of Eva and parse the following program using the same
```
(
    (printf "Value : %d" 42)
)
```
## Chapter 5 Symbols: Global variables
This demonstrates how to add global variables in the generated code from the S-expressions. At the end, one should get the following IR generated
```
; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"

@VERSION = global i32 42, align 4
@0 = private unnamed_addr constant [16 x i8] c"Version: %d\\n\\n\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i32 0, i32 0), i32* @VERSION)
  ret i32 %0
}
```