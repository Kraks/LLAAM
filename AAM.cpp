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
    
    void testStore(Module& M) {
      // test equal and update
      /*
      Function* add = M.getFunction("add");
      Function* sub = M.getFunction("sub");
      FuncValue fadd(add);
      FuncValue fsub(sub);
      
      ConcreteStackPtr sp1;
      ConcreteStackPtr sp2;
      
      ConcreteStore s1({{sp1, &fadd}});
      ConcreteStore s2({{sp1, &fsub}});
      
      errs() << "store eq: " << (s1 == s2) << "\n"; // false
       */
    }
    
    void testStoreLookup(Module& M) {
      Function* mainFunc = M.getFunction("main");
      printInst(*mainFunc);
  
      std::shared_ptr<FuncValue> f1 = std::make_shared<FuncValue>(mainFunc);
      std::shared_ptr<FuncValue> f2 = std::make_shared<FuncValue>(mainFunc);
      std::shared_ptr<PrimValue> pv = std::make_shared<PrimValue>();
      errs() << "functions eq: " << (*f1.get() == *f2.get()) << "\n"; // true
  
      ConcreteStackPtr sp1;
      ConcreteStackPtr sp2;
      ConcreteHeapAddr hp1;
      ConcreteHeapAddr hp2;
      //errs() << "stack ptrs eq: " << (sp1 == sp2) << "\n"; // false
      ConcreteStore store({{sp1, f1}, {sp2, f2}, {hp1, pv}, {hp2, pv}});
  
      AbstractValue* v1 = store.lookup(sp1);
      AbstractValue* v2 = store.lookup(sp2);
      FuncValue* f11 = static_cast<FuncValue*>(v1);
      FuncValue* f22 = static_cast<FuncValue*>(v2);
      errs() << "functions(get from store) eq: " << (*f11 == *f22) << "\n"; // true
      
      AbstractValue* someV = store.lookup(sp2);
      errs() << AbstractValue::valTypeToString(someV->getValType()) << "\n";
      
      PrimValue* fakepv = static_cast<PrimValue*>(store.lookup(sp2));
      if (fakepv == nullptr) { errs() << "fake pv null\n"; }
      else { errs() << "fake pv not null\n"; }
      
    }

    bool runOnModule(Module& M) override {
      testStoreLookup(M);
      return false;
    }

  };

  char AAMPass::ID = 0;
  static RegisterPass<AAMPass> X("aam", "AAM Pass", false/*Only looks at CFG*/, false/*Analysis Pass*/);
}

