// Created by WeiGuannan on 14/01/2017.
//

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/SymbolTableListTraits.h"
#include "llvm/Support/raw_ostream.h"

#include "AAM.h"
#include "Utils.h"

using namespace llvm;
using namespace AAM;
using namespace ConcreteAAM;

namespace {
  struct AAMPass : public ModulePass {
    static char ID;
    AAMPass() : ModulePass(ID) {}

    bool runOnModule(Module& M) override {
      Function* mainFunc = M.getFunction("main");
      printInst(*mainFunc);

      FuncValue f1(*mainFunc);
      FuncValue f2(*mainFunc);
      errs() << "eq?:" << (f1 == f2) << "\n";
      ConcreteStackPtr sp1;
      ConcreteStackPtr sp2;
      ConcreteStore store({{sp1, f1}, {sp2, f2}});

      FuncValue* f11 = static_cast<FuncValue*>(store.lookup(sp1));
      FuncValue* f22 = static_cast<FuncValue*>(store.lookup(sp2));

      return false;
    }

  };

  char AAMPass::ID = 0;
  static RegisterPass<AAMPass> X("aam", "AAM Pass", false/*Only looks at CFG*/, false/*Analysis Pass*/);
}

