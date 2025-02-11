/**
 * Eva to LLVM IR compiler.
 */
#ifndef EvaLLVM_h
#define EvaLLVM_h

#include <regex>
#include <string>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

#include "Environment.h"
#include "parser/EvaParser.h"

using syntax::EvaParser;

/**
 * Environment type.
 */
using Env = std::shared_ptr<Environment>;

// Generic binary operator:
#define GEN_BINARY_OP(Op, varName)          \
  do {                                      \
    auto op1 = gen(exp.list[1], env);       \
    auto op2 = gen(exp.list[2], env);       \
    return builder->Op(op1, op2, varName);  \
  } while (false)

class EvaLLVM {
  public:
    EvaLLVM() : parser(std::make_unique<EvaParser>()) {
      moduleInit();
      setupExternFunctions();
      setupGlobalEnvironment();
    }

    /**
     * Executes a program.
     */
    void exec(const std::string& program) {
      // 1. Parse the program
      auto ast = parser->parse("(begin " + program + ")");
      
      // 2. Compile to LLVM IR:
      compile(ast);

      // Print generated code.
      module->print(llvm::outs(), nullptr);

      std::cout << "\n";
      
      // 3. Save module IR to file:
      saveModuleToFile("out.ll");
    }

  private:
    /**
     * Compiles an expression.
     */
    void compile(const Exp& ast) {
      // 1. Create main function:
      fn = createFunction(
         "main", 
         llvm::FunctionType::get(/* return type */ builder->getInt32Ty(),
                                 /* vararg */ false), 
         GlobalEnv);

      // createGlobalVar("VERSION", builder->getInt32(42));

      // 2. Compile main body:
      auto result = gen(ast, GlobalEnv);

      builder->CreateRet(builder->getInt32(0));
    }

    /** 
     * Main compile loop.
     */
    llvm::Value* gen(const Exp& exp, Env env) {

      switch (exp.type) {
        /**
         * ---------------------------------------
         * Numbers.
         */
        case ExpType::NUMBER:
          return builder->getInt32(exp.number);

        /**
         * ---------------------------------------
         * Symbols.
         */
        case ExpType::SYMBOL:
          /**
           * Boolean
           */
          if (exp.string == "true" || exp.string == "false") {
            return builder->getInt1(exp.string == "true" ? true : false);
          } else {
            // Variables:
            auto varName = exp.string;
            auto value = env->lookup(varName);
            
            // 1. Local vars: (TODO)
            if (auto localVar = llvm::dyn_cast<llvm::AllocaInst>(value)) {
              return builder->CreateLoad(localVar->getAllocatedType(), localVar,
                  varName.c_str());
            }

            // 2. Global vars:
            else if (auto globalVar = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
              return builder->CreateLoad(globalVar->getInitializer()->getType(), globalVar,
                  varName.c_str());
            }
          }
        /**
         * ---------------------------------------
         * Strings.
         */
        case ExpType::STRING: {
          // Unescape special chars. TODO: support all chars or handle in parser.
          auto re = std::regex("\\\\n");
          auto str = std::regex_replace(exp.string, re, "\n");

          return builder->CreateGlobalStringPtr(str);
        }

        /**
         * ---------------------------------------
         * Lists.
         */
        case ExpType::LIST:
          auto tag = exp.list[0];

          /**
           * -------------------------------------
           * Special cases.
           */
          if (tag.type == ExpType::SYMBOL) {
            auto op = tag.string;

            // -----------------------------------
            // Binary math operations:

            if (op == "+") {
              GEN_BINARY_OP(CreateAdd, "tmpadd");
            }

            else if (op == "-") {
              GEN_BINARY_OP(CreateSub, "tmpsub");
            }

            else if (op == "*") {
              GEN_BINARY_OP(CreateMul, "tmpmul");
            }

            else if (op == "/") {
              GEN_BINARY_OP(CreateSDiv, "tmpdiv");
            }

            // -----------------------------------
            // Compare operations: (> 5 10)
            
            // UGT - unsigned, greater than
            else if(op == ">") {
              GEN_BINARY_OP(CreateICmpUGT, "tmpcmp");
            }

            // ULT - unsigned, less  than
            else if(op == "<") {
              GEN_BINARY_OP(CreateICmpULT, "tmpcmp");
            }

            // EQ = equal
            else if(op == "==") {
              GEN_BINARY_OP(CreateICmpEQ, "tmpcmp");
            }

            // NE = not equal
            else if(op == "!=") {
              GEN_BINARY_OP(CreateICmpNE, "tmpcmp");
            }

            // UGE = greater or equal
            else if(op == ">=") {
              GEN_BINARY_OP(CreateICmpUGE, "tmpcmp");
            }

            // ULE = less or equal
            else if(op == "<=") {
              GEN_BINARY_OP(CreateICmpULE, "tmpcmp");
            }

            // -----------------------------------
            // Variable declaration: (var x (+ y 10))
            //
            // Typed: (var (x number) 42)
            //
            // Note: locals are allocated on the stack

            if (op == "var") {
              auto varNameDecl = exp.list[1];
              auto varName = extractVarName(varNameDecl);

              // Initializer:
              auto init = gen(exp.list[2], env);

              // Type:
              auto varTy = extractVarType(varNameDecl);

              // Vardiable:
              auto varBinding = allocVar(varName, varTy, env);

              // Set value:
              return builder->CreateStore(init, varBinding);
            } 

            // -----------------------------------
            // Variable update: (set x 100)

            else if (op == "set") {
              auto value = gen(exp.list[2], env);

              auto varName = exp.list[1].string;

              // Variable:
              auto varBinding = env->lookup(varName);

              // Set value:
              return builder->CreateStore(value, varBinding);
            }

            // -----------------------------------
            // printf extern function:
            //
            // (printf "Value: %d" 42)
            //

            else if (op == "printf") {
              auto printfFn = module->getFunction("printf");
              
              std::vector<llvm::Value*> args{};
              
              for (auto i = 1; i < exp.list.size(); i++) {
                args.push_back(gen(exp.list[i], env));
              }
              return builder->CreateCall(printfFn, args);
          } 

          // -----------------------------------
          // Blocks: (begin <expression>)

          else if (op == "begin") {
            // Block scope:
            auto blockEnv = std::make_shared<Environment>(
                std::map<std::string, llvm::Value*>{}, env);

            // Compile each expression within the block
            // Result is the last evaluated expression
            llvm::Value* blockRes;
            for (auto i = 1; i < exp.list.size(); i++) {
              //Generate expression code
              blockRes = gen(exp.list[i], blockEnv); // TODO local block env
            }
            return blockRes;
          }
        }
      }
      // Unreachable
      return builder->getInt32(0);
    }

    /**
     * Extracts var or parameter name considering type.
     *
     * x -> x
     * (x number) -> x
     */
    std::string extractVarName(const Exp& exp) {
      return exp.type == ExpType::LIST ? exp.list[0].string : exp.string;
    }

    /**
     * Extracts var or parameter type with i32 as default.
     *
     * x -> i32
     * (x number) -> number
     */
    llvm::Type* extractVarType(const Exp& exp) {
      return exp.type == ExpType::LIST ? getTypeFromString(exp.list[1].string)
        : builder->getInt32Ty();
    }

    /**
     * Returns LLVM type from string representation
     */
    llvm::Type* getTypeFromString(const std::string& type_) {
      // number -> i32
      if (type_ == "number") {
        return builder->getInt32Ty();
      }

      // string -> i8* (aka char*)
      if (type_ == "string") {
        return builder->getInt8Ty()->getPointerTo();
      }

      // default:
      return builder->getInt32Ty();
    }

    /**
     * Allocates a local variable on the stack. Result is the alloca instruction.
     */
    llvm::Value* allocVar(const std::string& name, llvm::Type* type_, Env env) {
      varsBuilder->SetInsertPoint(&fn->getEntryBlock());

      auto varAlloc = varsBuilder->CreateAlloca(type_, 0, name.c_str());

      // Add to the environment:
      env->define(name, varAlloc);

      return varAlloc;
    }
    
    /**
     * Creates a global variable.
     */
    llvm::GlobalVariable* createGlobalVar(const std::string& name,
        llvm::Constant* init) {
      module->getOrInsertGlobal(name, init->getType());
      auto variable = module->getNamedGlobal(name);
      variable->setAlignment(llvm::MaybeAlign(4));
      variable->setConstant(false);
      variable->setInitializer(init);

      return variable;
    }

    /** 
     * Define external functions (from libc++)
     */
    void setupExternFunctions() {
      // i8* to substitute for char*, void*, etc
      auto bytePtrTy = builder->getInt8Ty()->getPointerTo();

      // int printf(const char* format, ...);
      module->getOrInsertFunction("printf", 
          llvm::FunctionType::get(
            /* return type */ builder->getInt32Ty(),
            /* format arg */ bytePtrTy,
            /* vararg */ true));
    }

    /** 
     * Creates a function.
     */
    llvm::Function* createFunction(const std::string& fnName,
                                    llvm::FunctionType* fnType, Env env) {
      // Function prototype might already be defined
      auto fn = module->getFunction(fnName);

      // If not, allocate the function:
      if (fn == nullptr) {
        fn = createFunctionProto(fnName, fnType, env);
      }

      createFunctionBlock(fn);
      return fn;
    }

    /** 
     * Creates function prototype (defines the function, but not 
     * the body)
     */
    llvm::Function* createFunctionProto(const std::string& fnName,
                                        llvm::FunctionType* fnType, Env env) {
      auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, 
          fnName, *module);
      verifyFunction(*fn);

      // Install in the environment
      env->define(fnName, fn);

      return fn;
    }

    /** 
     * Creates function block.
     */
    void createFunctionBlock(llvm::Function* fn) {
      auto entry = createBB("entry", fn);
      builder->SetInsertPoint(entry);
    }

    /**
     * Creates a basic block. If the `fn` is passed, the block is
     * automatically appended to the parent  function. Otherwise, the block
     * should later be appended manually via
     * fn->getBasicBlockList().push_back(block);
     */
    llvm::BasicBlock* createBB(std::string name, llvm::Function* fn = nullptr) {
      return llvm::BasicBlock::Create(*ctx, name, fn);
    }

    /** 
     * Initialize the module
     */
    void moduleInit() {
      // Open a new context and module.
      ctx = std::make_unique<llvm::LLVMContext>();
      module = std::make_unique<llvm::Module>("EvaLLVM", *ctx);

      // Create a new builder for the module
      builder = std::make_unique<llvm::IRBuilder<>>(*ctx);

      // Vars builder:
      varsBuilder = std::make_unique<llvm::IRBuilder<>>(*ctx);
    }

    /** 
     * Saves IR to file.
     */
    void saveModuleToFile(const std::string& fileName) {
      std::error_code errorCode;
      llvm::raw_fd_ostream outLL(fileName, errorCode);
      module->print(outLL, nullptr);
    }

    /**
     * Sets up The Global Environment
     */
    void setupGlobalEnvironment() {
      std::map<std::string, llvm::Value*> globalObject{
        {"VERSION", builder->getInt32(42)},
      };

      std::map<std::string, llvm::Value*> globalRec{};

      for (auto& entry : globalObject) {
        globalRec[entry.first] = 
          createGlobalVar(entry.first, (llvm::Constant*)entry.second);
      }

      GlobalEnv = std::make_shared<Environment>(globalRec, nullptr);
    }

    /**
     * Parser.
     */
    std::unique_ptr<EvaParser> parser;

    /**
     * Global Environment (symbol table).
     */
    std::shared_ptr<Environment> GlobalEnv;

    /**
     * Currently compiling function.
     */
    llvm::Function* fn;

    /**
     * Global LLVM context
     * It owns and manages the core "global" data of LLVM's core
     * infrastructure, including the types and constant unique tables.
     */
    std::unique_ptr<llvm::LLVMContext> ctx;
    
    /**
     * A module instance is used to store all the information related to an 
     * LLVM modle. Modules are the top level container of all other LLVM
     * Intermediate Representation (IR) objects. Each module directly contains a
     * list of global variables, a list of functions, a list of libraries (or
     * other modules) this module depends on, a symbol table, and various data
     * about the target's characteristics.
     *
     * A module maintains a GlobalList object that is used to hold all constant
     * references to global variables in the module. When a global variable is
     * destroyed, it should have no entries in the GlobalList. The main
     * container class for the LLVM Intermediate Representation.
     */
    std::unique_ptr<llvm::Module> module;

    /**
     * Extra builder for variables declaration.
     * This builder always preprends to the begining of the function 
     * entry block
     */
    std::unique_ptr<llvm::IRBuilder<>> varsBuilder;

    /** 
     * IR Builder.
     *
     * This provides a uniform API for creating instructions and inserting them
     * into a basic block: either at the end of a BasicBlock, or at a specific
     * iterator location in a block.
     */
    std::unique_ptr<llvm::IRBuilder<>> builder;
};



#endif//EvaLLVM_h
