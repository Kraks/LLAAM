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
  
  void compute(Function& F) {
    Liveness IN;
    Liveness OUT;
    
    errs() << "initializing\n";
    
    auto& BBs = F.getBasicBlockList();
    for (auto& block : BBs) {
      for (auto& inst : block.getInstList()) {
        errs() << "init: ";
        inst.print(errs());
        errs() << "\n";
        
        IN[&inst] = std::set<Value*>();
        OUT[&inst] = std::set<Value*>();
      }
    }
    
    for (auto& block : BBs) {
      for (auto& inst : block.getInstList()) {
        errs() << "processing: ";
        inst.print(errs());
        errs() << "\n";
        
        for (auto* user : inst.users()) {
          if (Instruction* userInst = dyn_cast<Instruction>(user)) {
            live(&inst, userInst, &IN, &OUT);
          }
          else {
            assert(false && "not an instruction");
          }
        }
      }
    }
    
    errs() << "\n";
    for (auto& block : BBs) {
      for (auto& inst : block.getInstList()) {
        assert(IN.find(&inst) != IN.end());
        errs() << "IN[";
        inst.print(errs());
        errs() << "]\n";
        
        for (Value* v : IN.find(&inst)->second) {
          errs() << "  ";
          v->print(errs());
          errs() << "\n";
        }
      }
    }
    
    errs() << "\n";
    for (auto& block : BBs) {
      for (auto& inst : block.getInstList()) {
        assert(OUT.find(&inst) != IN.end());
        errs() << "OUT[";
        inst.print(errs());
        errs() << "]\n";
      
        for (Value* v : OUT.find(&inst)->second) {
          errs() << "  ";
          v->print(errs());
          errs() << "\n";
        }
      }
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
