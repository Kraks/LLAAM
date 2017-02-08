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
      Function* mainFunc = M.getFunction("main");
      printInst(*mainFunc);
  
      std::shared_ptr<FuncValue> f1 = std::make_shared<FuncValue>(mainFunc);
      std::shared_ptr<FuncValue> f2 = std::make_shared<FuncValue>(mainFunc);
      std::shared_ptr<PrimValue> pv = std::make_shared<PrimValue>();
      assert((*f1.get() == *f2.get()));
      //errs() << "functions eq: " << (*f1.get() == *f2.get()) << "\n"; // true
  
      ConcreteStackPtr sp1;
      ConcreteStackPtr sp2;
      ConcreteHeapAddr hp1;
      ConcreteHeapAddr hp2;
      assert(!(sp1 == sp2));
      assert(!(hp1 == hp2));
      
      //errs() << "stack ptrs eq: " << (sp1 == sp2) << "\n"; // false
      ConcreteStore store({{sp1, f1}, {sp2, f2}, {hp1, pv}, {hp2, pv}});
  
      AbstractValue* v1 = store.lookup(sp1);
      AbstractValue* v2 = store.lookup(sp2);
      FuncValue* f11 = static_cast<FuncValue*>(v1);
      FuncValue* f22 = static_cast<FuncValue*>(v2);
      assert(*f11 == *f22);
      //errs() << "functions(get from store) eq: " << (*f11 == *f22) << "\n"; // true
      
      AbstractValue* someV = store.lookup(sp2);
      errs() << AbstractValue::valTypeToString(someV->getKind()) << "\n";
      
      PrimValue* fakepv = static_cast<PrimValue*>(store.lookup(sp2));
      assert(!(fakepv == nullptr));
      
      someV = store.lookup(hp1);
      assert(isa<PrimValue>(someV));
      someV = store.lookup(hp2);
      assert(!isa<FuncValue>(someV));
      
      PrimValue* pv1 = static_cast<PrimValue*>(store.lookup(hp1));
      PrimValue* pv2 = static_cast<PrimValue*>(store.lookup(hp2));
      assert(*pv1 == *pv2);
      //errs() << "PrimVal eq: " << (*pv1 == *pv2) << "\n";
    }

    bool runOnModule(Module& M) override {
      testStore(M);
      return false;
    }

  };

  char AAMPass::ID = 0;
  static RegisterPass<AAMPass> X("aam", "AAM Pass", false/*Only looks at CFG*/, false/*Analysis Pass*/);
}

