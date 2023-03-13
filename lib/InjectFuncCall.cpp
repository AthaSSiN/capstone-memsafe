//========================================================================
// FILE:
//    InjectFuncCall.cpp
//
// DESCRIPTION:
//    For each function defined in the input IR module, InjectFuncCall inserts
//    a call to printf (from the C standard I/O library). The injected IR code
//    corresponds to the following function call in ANSI C:
//    ```C
//      printf("(llvm-tutor) Hello from: %s\n(llvm-tutor)   number of arguments: %d\n",
//             FuncName, FuncNumArgs);
//    ```
//    This code is inserted at the beginning of each function, i.e. before any
//    other instruction is executed.
//
//    To illustrate, for `void foo(int a, int b, int c)`, the code added by InjectFuncCall
//    will generated the following output at runtime:
//    ```
//    (llvm-tutor) Hello World from: foo
//    (llvm-tutor)   number of arguments: 3
//    ```
//
// USAGE:
//    1. Legacy pass manager:
//      $ opt -load <BUILD_DIR>/lib/libInjectFuncCall.so `\`
//        --legacy-inject-func-call <bitcode-file>
//    2. New pass maanger:
//      $ opt -load-pass-plugin <BUILD_DIR>/lib/libInjectFunctCall.so `\`
//        -passes=-"inject-func-call" <bitcode-file>
//
// License: MIT
//========================================================================
#include "InjectFuncCall.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "inject-func-call"

//-----------------------------------------------------------------------------
// InjectFuncCall implementation
//-----------------------------------------------------------------------------
bool InjectFuncCall::runOnModule(Module &M) {
  bool updatedCode = false;

  auto &CTX = M.getContext();

  // STEP 1: Inject the declaration of check
  // ----------------------------------------
  // Create (or _get_ in cases where it's already available) the following
  // declaration in the IR module:
  //    declare void @check(i64*, ...) (or define if defined)
  // It corresponds to the following C declaration:
  //    void check(int *, ...)

  Type *CheckArgTy = Type::getInt64PtrTy(CTX);

  FunctionType *CheckTy = FunctionType::get(
      Type::getVoidTy(CTX),
      CheckArgTy,
      false);

  FunctionCallee Check = M.getOrInsertFunction("check", CheckTy);

  Function *CheckF = dyn_cast<Function>(Check.getCallee());
  CheckF->addParamAttr(0, Attribute::NoUndef);

  // STEP 2: Inject the declaration of tmalloc and malloc
  // ----------------------------------------
  // Create (or _get_ in cases where it's already available) the following
  // declaration in the IR module:
  //    declare ptr @tmalloc(i64 noundef) (or define if defined)
  // It corresponds to the following C declaration:
  //    void* tmalloc(size_t)

  Type *TmallocArgTy = Type::getIntNTy(CTX, sizeof(size_t) * 8);

  FunctionType *TmallocTy = FunctionType::get(
      Type::getInt8PtrTy(CTX),
      TmallocArgTy,
      false);

  FunctionCallee Tmalloc = M.getOrInsertFunction("tmalloc", TmallocTy);

  Function *TmallocF = dyn_cast<Function>(Tmalloc.getCallee());
  TmallocF->addParamAttr(0, Attribute::NoUndef);

  Type *MallocArgTy = Type::getIntNTy(CTX, sizeof(size_t) * 8);

  FunctionType *MallocTy = FunctionType::get(
      Type::getInt8PtrTy(CTX),
      MallocArgTy,
      false);

  FunctionCallee Malloc = M.getOrInsertFunction("malloc", MallocTy);

  Function *MallocF = dyn_cast<Function>(Malloc.getCallee());
  MallocF->addParamAttr(0, Attribute::NoUndef);

  // STEP 3: Inject the declaration of tfree and free
  // ----------------------------------------
  // Create (or _get_ in cases where it's already available) the following
  // declaration in the IR module:
  //    declare void @tfree(ptr noundef) (or define if defined)
  // It corresponds to the following C declaration:
  //    void tfree(void*)

  Type *TfreeArgTy = Type::getInt8PtrTy(CTX);

  FunctionType *TfreeTy = FunctionType::get(
      Type::getVoidTy(CTX),
      TfreeArgTy,
      false);

  FunctionCallee Tfree = M.getOrInsertFunction("tfree", TfreeTy);

  Function *TfreeF = dyn_cast<Function>(Tfree.getCallee());
  TfreeF->addParamAttr(0, Attribute::NoUndef);

  Type *FreeArgTy = Type::getInt8PtrTy(CTX);

  FunctionType *FreeTy = FunctionType::get(
      Type::getVoidTy(CTX),
      FreeArgTy,
      false);

  FunctionCallee Free = M.getOrInsertFunction("free", FreeTy);

  Function *FreeF = dyn_cast<Function>(Free.getCallee());
  FreeF->addParamAttr(0, Attribute::NoUndef);

  for (auto &F : M) {
    if (strcmp(F.getName().str().c_str(), "main"))
      continue;
    
    for(auto &BB : F) {
      for(auto I = BB.begin(), IE = BB.end(); I != IE; ++I) {
        if(isa<LoadInst>(I) or isa<StoreInst>(I)) {
          // errs() << I << "\n";
          IRBuilder<> Builder(&*I);
          CallInst *CallCheck = Builder.CreateCall(
            Check, 
            {I->getOperand(isa<LoadInst>(I) ? 0 : 1)}
          );
          CallCheck->addParamAttr(0, Attribute::NoUndef);
          updatedCode = true;
        }

        else if(auto *CI = dyn_cast<CallInst>(I)) {
          if (CI->getCalledFunction() == MallocF) {
            IRBuilder<> Builder(CI);

            Value *TmallocArg = Builder.CreateIntCast(
              CI->getArgOperand(0), TmallocArgTy, true
            );

            CallInst *CallTmalloc = CallInst::Create(
              Tmalloc, TmallocArg, "", CI
            );
            CallTmalloc->addParamAttr(0, Attribute::NoUndef);

            ReplaceInstWithInst(BB.getInstList(), I, CallTmalloc);
            updatedCode = true;
          }
          else if (CI->getCalledFunction() == FreeF) {
              IRBuilder<> Builder(CI);

              Value *TfreeArg = Builder.CreateIntCast(
                CI->getArgOperand(0), TfreeArgTy, true
              );

              CallInst *CallTfree = CallInst::Create(
                Tfree, TfreeArg, "", CI
              );
              CallTfree->addParamAttr(0, Attribute::NoUndef);
              // CallTfree->addParamAttr(0, Attribute::InAlloca);

              ReplaceInstWithInst(BB.getInstList(), I, CallTfree);
              updatedCode = true;
          }
        }
      }
    }

  }

  errs() << "After\n";

  for(auto &F : M)
  {
    if(!strcmp(F.getName().str().c_str(), "main"))
      errs() << F << "\n";
  }

  return updatedCode;
}

PreservedAnalyses InjectFuncCall::run(llvm::Module &M,
                                       llvm::ModuleAnalysisManager &) {
  bool Changed =  runOnModule(M);

  return (Changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

bool LegacyInjectFuncCall::runOnModule(llvm::Module &M) {
  bool Changed = Impl.runOnModule(M);

  return Changed;
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getInjectFuncCallPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "inject-func-call", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "inject-func-call") {
                    MPM.addPass(InjectFuncCall());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getInjectFuncCallPluginInfo();
}

//-----------------------------------------------------------------------------
// Legacy PM Registration
//-----------------------------------------------------------------------------
char LegacyInjectFuncCall::ID = 0;

// Register the pass - required for (among others) opt
static RegisterPass<LegacyInjectFuncCall>
    X(/*PassArg=*/"legacy-inject-func-call", /*Name=*/"LegacyInjectFuncCall",
      /*CFGOnly=*/false, /*is_analysis=*/false);
