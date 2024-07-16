; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"

@VERSION = global i32 42, align 4

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %0 = sext ptr @VERSION to i32
  ret i32 %0
}
