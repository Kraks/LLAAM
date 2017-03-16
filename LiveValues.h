//
// Created by WeiGuannan on 01/03/2017.
//

#ifndef LLVM_LIVEVALUES_H
#define LLVM_LIVEVALUES_H

#include "AAM.h"

class LiveValues {
public:
  LiveValues(ModulePass* MP): MP(MP) {}
  
  typedef std::set<Value*> ValueSet;
  typedef std::map<Value*, ValueSet> Liveness;
  
  struct Result {
    Liveness IN;
    Liveness OUT;
  };
  
  std::map<Function*, Result> results;
  
  void printResult(Function& F, Result& res) {
    auto& BBs = F.getBasicBlockList();
    
    errs() << "\n";
    for (auto& block : BBs) {
      for (auto& inst : block.getInstList()) {
        assert(res.IN.find(&inst) != res.IN.end());
        errs() << "IN[";
        inst.print(errs());
        errs() << "]\n";
      
        for (Value* v : res.IN.find(&inst)->second) {
          errs() << "  ";
          v->print(errs());
          errs() << "\n";
        }
      }
    }
  
    errs() << "\n";
    for (auto& block : BBs) {
      for (auto& inst : block.getInstList()) {
        assert(res.OUT.find(&inst) != res.OUT.end());
        errs() << "OUT[";
        inst.print(errs());
        errs() << "]\n";
      
        for (Value* v : res.OUT.find(&inst)->second) {
          errs() << "  ";
          v->print(errs());
          errs() << "\n";
        }
      }
    }
    
  }
  
  Result& compute(Function& F) {
    if (results.find(&F) == results.end()) {
      Result res;
      errs() << "initializing\n";
  
      auto& BBs = F.getBasicBlockList();
      for (auto& block : BBs) {
        for (auto& inst : block.getInstList()) {
          errs() << "init: ";
          inst.print(errs());
          errs() << "\n";
      
          res.IN[&inst] = std::set<Value*>();
          res.OUT[&inst] = std::set<Value*>();
        }
      }
  
      for (auto& block : BBs) {
        for (auto& inst : block.getInstList()) {
          errs() << "processing: ";
          inst.print(errs());
          errs() << "\n";
      
          for (auto* user : inst.users()) {
            if (Instruction* userInst = dyn_cast<Instruction>(user)) {
              live(&inst, userInst, &res.IN, &res.OUT);
            }
            else {
              assert(false && "not an instruction");
            }
          }
        }
      }
      results[&F] = res;
      return results[&F];
    }
    else {
      errs() << "Already has result for function " << F.getName() << "\n";
      return results[&F];
    }
  }
  
  void live(Instruction* inst, Instruction* user,
            Liveness* IN, Liveness* OUT) {
    user->print(errs());
    errs() << " uses ";
    inst->print(errs());
    errs() << "\n";
    
    IN->at(user).insert(inst);
    BasicBlock* BB = user->getParent();
    if (&*(BB->begin()) == user) {
      for (pred_iterator piter = pred_begin(BB), end = pred_end(BB);
           piter != end;
           ++piter) {
        BasicBlock* predBB = *piter;
        Instruction* term = predBB->getTerminator();
        OUT->at(term).insert(inst);
        if (term != inst) {
          live(inst, term, IN, OUT);
        }
      }
    }
    else {
      BasicBlock::iterator iter(user);
      Instruction* pred = &*(--iter);
      
      errs() << "pred: ";
      pred->print(errs());
      errs() << "\n";
      
      OUT->at(pred).insert(inst);
      if (pred != inst) {
        live(inst, pred, IN, OUT);
      }
    }
  }
  
private:
  ModulePass* MP;
};


#endif //LLVM_LIVEVALUES_H
