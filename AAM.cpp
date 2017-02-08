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
    
    static void testStore(Module& M) {
      Function* mainFunc = M.getFunction("main");
      printInst(*mainFunc);
  
      std::shared_ptr<FuncValue> f1 = std::make_shared<FuncValue>(mainFunc);
      std::shared_ptr<FuncValue> f2 = std::make_shared<FuncValue>(mainFunc);
      std::shared_ptr<PrimValue> pv = std::make_shared<PrimValue>();
      assert((*f1.get() == *f2.get()));
      //errs() << "functions eq: " << (*f1.get() == *f2.get()) << "\n"; // true
      
      std::shared_ptr<ConcreteStackPtr> sp1 = std::make_shared<ConcreteStackPtr>();
      assert(isa<ConcreteStackPtr>(*sp1.get()));
      assert(isa<StackPtr>(*sp1.get()));
      
      std::shared_ptr<ConcreteStackPtr> sp2 = std::make_shared<ConcreteStackPtr>();
      assert(isa<ConcreteStackPtr>(*sp2.get()));
      assert(!(isa<HeapAddr>(*sp2.get())));
      
      std::shared_ptr<ConcreteHeapAddr> hp1 = std::make_shared<ConcreteHeapAddr>();
      assert(isa<ConcreteHeapAddr>(*hp1.get()));
      
      std::shared_ptr<ConcreteHeapAddr> hp2 = std::make_shared<ConcreteHeapAddr>();
      assert(isa<ConcreteHeapAddr>(*hp2.get()));
      assert(isa<Location>(*hp2.get()));
      
      assert(!(*sp1.get() == *sp2.get()));
      assert(!(*sp1.get() == *hp1.get()));
      assert(!(*hp1.get() == *hp2.get()));
      assert(*sp1.get() == *sp1.get());
      assert(*sp2.get() == *sp2.get());
      assert(*hp1.get() == *hp1.get());
      assert(*hp2.get() == *hp2.get());
      
      std::shared_ptr<ConcreteStackPtr> sp3 = std::make_shared<ConcreteStackPtr>();
      
      std::shared_ptr<LocalBindAddr> baddr1 = std::make_shared<LocalBindAddr>("x", sp3);
      std::shared_ptr<LocalBindAddr> baddr2 = std::make_shared<LocalBindAddr>("x", sp3);
      assert(isa<BindAddr>(*baddr1.get()));
      assert(isa<LocalBindAddr>(*baddr1.get()));
      assert(*baddr1.get() == *baddr1.get());
      assert(*baddr1 == *baddr1);
      assert(*baddr1.get() == *baddr2.get());
      assert(!(*baddr1.get() == *sp1.get()));
      assert(!(*baddr1.get() == *hp2.get()));
  
      //errs() << "stack ptrs eq: " << (sp1 == sp2) << "\n"; // false
      ConcreteStore store({{sp1, f1}, {sp2, f2}, {hp1, pv}, {hp2, pv}});
  
      AbstractValue* v1 = store.lookup(sp1);
      AbstractValue* v2 = store.lookup(sp2);
      FuncValue* f11 = static_cast<FuncValue*>(v1);
      FuncValue* f22 = static_cast<FuncValue*>(v2);
      assert(*f11 == *f22);
      //errs() << "functions(get from store) eq: " << (*f11 == *f22) << "\n"; // true
      
      AbstractValue* someV = store.lookup(sp2);
      //errs() << AbstractValue::KindToString(someV->getKind()) << "\n";
      
      PrimValue* fakepv = static_cast<PrimValue*>(store.lookup(sp2));
      assert(!(fakepv == nullptr));
  
      someV = store.lookup(hp1);
      errs() << AbstractValue::KindToString(someV->getKind()) << "\n";
      assert(isa<PrimValue>(someV));
      someV = store.lookup(hp2);
      assert(!isa<FuncValue>(someV));
  
      PrimValue* pv1 = static_cast<PrimValue*>(store.lookup(hp1));
      PrimValue* pv2 = static_cast<PrimValue*>(store.lookup(hp2));
      assert(*pv1 == *pv2);
      //errs() << "PrimVal eq: " << (*pv1 == *pv2) << "\n";
      
      for (int i = 0; i <= 2000; i++) {
        ConcreteStore store2 = store.update(sp1, pv);
        someV = store2.lookup(sp1);
        assert(store2.size() == 4);
        //errs() << "classof: " << AbstractValue::KindToString(someV->getKind()) << "\n";
        assert(isa<PrimValue>(someV));
        
        someV = store.lookup(sp1);
        assert(isa<FuncValue>(someV));
      }
    }

    bool runOnModule(Module& M) override {
      testStore(M);
      return false;
    }

  };

  char AAMPass::ID = 0;
  static RegisterPass<AAMPass> X("aam", "AAM Pass", false/*Only looks at CFG*/, false/*Analysis Pass*/);
}

