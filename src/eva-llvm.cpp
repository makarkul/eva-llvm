/**
 * Eva LLVM executable
 */
#include <string>

#include "EvaLLVM.h"

int main(int argc, char const *argv[]) {
  /**
   * Program to execute
   */
  std::string program = R"(
    (var VERSION 42)
    (begin
      (var VERSION "Hello")
      (printf "Version: %s\n\n" VERSION)
    )
    (printf "Version: %d\n\n" VERSION)
  )";

  /**
   * Compiler instance.
   */
  EvaLLVM vm;

  /** 
   * Generate LLVM IR
   */
  vm.exec(program);
  return 0;
}
