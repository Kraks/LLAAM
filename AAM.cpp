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

using namespace llvm;


namespace {
  struct AAMPass : public ModulePass {
    static char ID;
    AAMPass() : ModulePass(ID) {}

    void printModuleInfo(Module& M) {
      errs() << "running AAM on module: ";
      errs().write_escaped(M.getName()) << "\n";

      Module::FunctionListType& fs = M.getFunctionList();
      for (auto &f : fs) {
        errs() << "function: " << f.getName() << "\n";
      }
    }

    void printInst(Function& F) {
      int idx = 0;
      errs() << "function: " << F.getName() << "\n";
      for (inst_iterator i = inst_begin(F), e = inst_end(F); i != e; i++, idx++) {
        Instruction* inst = static_cast<Instruction*>(&*i);
        errs() << "  inst " << idx << ": ";
        inst->print(errs(), false);
        errs() << "\n";
      }
    }

    bool runOnModule(Module& M) override {
      Function* mainFunc = M.getFunction("main");
      printInst(*mainFunc);
      return false;
    }

  };

  char AAMPass::ID = 0;
  static RegisterPass<AAMPass> X("aam", "AAM Pass", false/*Only looks at CFG*/, false/*Analysis Pass*/);
}

