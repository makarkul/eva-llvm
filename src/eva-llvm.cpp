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
    (printf "Value : %d\n" 42)
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
