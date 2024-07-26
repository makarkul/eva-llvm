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
    (var z 32)
    (var x (+ z 10))

    // (if (== x 42)
    //   (set x 100)
    //   (set x 200)
    // )
    (printf "Is X == 42? : %d\n" (> x 42))
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
