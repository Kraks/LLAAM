//
// Created by WeiGuannan on 06/02/2017.
//

#include "Utils.h"

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
