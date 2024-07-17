; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"

@VERSION = global i32 getelementptr inbounds ([6 x i8], [6 x i8]* @0, i32 0, i32 0), align 4
@0 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@1 = private unnamed_addr constant [16 x i8] c"Version: %s\\n\\n\00", align 1
@2 = private unnamed_addr constant [16 x i8] c"Version: %d\\n\\n\00", align 1

declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %0 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @1, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @0, i32 0, i32 0))
  %1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([16 x i8], [16 x i8]* @2, i32 0, i32 0), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @0, i32 0, i32 0))
  ret i32 0
}
