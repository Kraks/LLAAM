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

using namespace llvm;

void printModuleInfo(Module& M);
void printInst(Function& F);
Instruction* getEntry(Function& F);

#endif //LLVM_UTILS_H
