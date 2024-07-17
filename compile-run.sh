# Compile main:
clang++ -o eva-llvm `llvm-config --cxxflags --ldflags --system-libs --libs core` -fexceptions src/eva-llvm.cpp

# Run main:
./eva-llvm

# Execute generated IR:
lli ./out.ll
