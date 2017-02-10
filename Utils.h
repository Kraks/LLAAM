//
// Created by WeiGuannan on 06/02/2017.
//

#ifndef LLVM_UTILS_H
#define LLVM_UTILS_H

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/SymbolTableListTraits.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

using namespace llvm;

void printModuleInfo(Module& M);

void printInst(Instruction* inst);

void printInstructions(Function& F);
void printInstructions(BasicBlock& B);

Instruction* getEntry(Function& F);
Instruction* getEntry(BasicBlock& B);

Instruction* getSyntacticNextInst(Instruction* inst);

#endif //LLVM_UTILS_H
