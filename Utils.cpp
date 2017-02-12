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

static auto printInst = [] (Instruction* inst) {
  inst->print(errs(), false);
  errs() << "\n";
};

void printInstructions(Function& F) {
  forEachInst(F, printInst);
}

void printInstructions(BasicBlock& B) {
  forEachInst(B, printInst);
}

Instruction* getEntry(BasicBlock& B) {
  auto& insts = B.getInstList();
  Instruction* inst = &insts.front();
  return inst;
}

Instruction* getEntry(Function& F) {
  inst_iterator beg = inst_begin(F);
  Instruction* inst = static_cast<Instruction*>(&*beg);
  return inst;
}

Instruction* getSyntacticNextInst(Instruction* inst) {
  return inst->getNextNode();
}

Instruction* getSemanticNextInst(Instruction* inst) {
  
}

