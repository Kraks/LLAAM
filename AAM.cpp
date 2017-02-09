// Created by WeiGuannan on 14/01/2017.
//

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/SymbolTableListTraits.h"
#include "llvm/Support/raw_ostream.h"

#include "ConcreteAAM.h"
#include "Utils.h"

using namespace std;
using namespace llvm;
using namespace AAM;
using namespace ConcreteAAM;

namespace {
  struct AAMPass : public ModulePass {
    static char ID;
    AAMPass() : ModulePass(ID) {}
    
    static void testStore(Module& M) {
      Function* mainFunc = M.getFunction("main");
      Function* add = M.getFunction("add");
      Function* sub = M.getFunction("sub");
  
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
      
      shared_ptr<LocationValue> lv1 = make_shared<LocationValue>(sp1);
      shared_ptr<LocationValue> lv2 = make_shared<LocationValue>(sp2);
      
      //errs() << "stack ptrs eq: " << (sp1 == sp2) << "\n"; // false
      ConcreteStore store({{sp1, f1}, {sp2, f2}, {hp1, pv}, {hp2, pv}, {baddr1, lv1}, {baddr2, lv2}});
      assert(store == store);
      assert(store.size() == 5);
      ConcreteStore store2({{sp1, f1}});
      assert(store2.size() == 1);
      ConcreteStore store3({{sp1, f2}});
      assert(store3.size() == 1);
      assert(store2 == store3);
      
      shared_ptr<AbstractValue> av = store.lookup(baddr2).getValue();
      assert(av == lv1);
      assert(*av == *lv1);
      
      shared_ptr<LocalBindAddr> baddr3 = make_shared<LocalBindAddr>("y", sp3);
      assert(!store.lookup(baddr3).hasValue());
      
      shared_ptr<AbstractValue> v1 = store.lookup(sp1).getValue();
      shared_ptr<AbstractValue> v2 = store.lookup(sp2).getValue();
      shared_ptr<FuncValue> f11 = static_pointer_cast<FuncValue>(v1);
      shared_ptr<FuncValue> f22 = static_pointer_cast<FuncValue>(v2);
      assert(*f11 == *f22);
      //errs() << "functions(get from store) eq: " << (*f11 == *f22) << "\n"; // true
      
      shared_ptr<AbstractValue> someV = store.lookup(sp2).getValue();
      //errs() << AbstractValue::KindToString(someV->getKind()) << "\n";
      
      shared_ptr<PrimValue> fakepv = static_pointer_cast<PrimValue>(store.lookup(sp2).getValue());
      assert(!(fakepv == nullptr));
  
      someV = store.lookup(hp1).getValue();
      //errs() << AbstractValue::KindToString(someV->getKind()) << "\n";
      assert(isa<PrimValue>(*someV));
      someV = store.lookup(hp2).getValue();
      assert(!isa<FuncValue>(*someV));
  
      shared_ptr<PrimValue> pv1 = static_pointer_cast<PrimValue>(store.lookup(hp1).getValue());
      shared_ptr<PrimValue> pv2 = static_pointer_cast<PrimValue>(store.lookup(hp2).getValue());
      std::shared_ptr<AbstractValue> ap(pv1);
      assert(*ap == *pv1);
      assert(*pv1 == *pv2);
      //errs() << "PrimVal eq: " << (*pv1 == *pv2) << "\n";
      
      for (int i = 0; i <= 2000; i++) {
        ConcreteStore store4 = store.update(sp1, pv);
        someV = store4.lookup(sp1).getValue();
        assert(store4.size() == 5);
        assert(store.size() == 5);
        //errs() << "classof: " << AbstractValue::KindToString(someV->getKind()) << "\n";
        assert(isa<PrimValue>(*someV));
        
        someV = store.lookup(sp1).getValue();
        assert(isa<FuncValue>(*someV));
      }
      
      ConcreteStore store5 = store.remove(baddr1);
      assert(store5.size() == 4);
      ConcreteStore store6 = store.remove(baddr2);
      assert(store6.size() == 4);
      ConcreteStore store7 = store5.update(baddr1, pv);
      shared_ptr<AbstractValue> another_pv = store7.lookup(baddr2).getValue();
      assert(isa<PrimValue>(*another_pv));
      
      Instruction* i1 = getEntry(*mainFunc);
      Instruction* i2 = getNextInst(i1);
      
      shared_ptr<Cont> c1 = make_shared<Cont>("x", i1, sp1, sp2);
      assert(*c1 == *c1);
      shared_ptr<Cont> c2 = make_shared<Cont>("y", i2, sp1, sp2);
      assert(*c2 == *c2);
      assert(!(*c1 == *c2));
      
      shared_ptr<FramePtr> sp1_copy = c1->getFramePtr();
      assert(sp1.get() == sp1_copy.get());
    }
    
    static void testLLVM(Module& M) {
      Function* mainFunc = M.getFunction("main");
      Instruction* inst = getEntry(*mainFunc);
    }

    bool runOnModule(Module& M) override {
      testStore(M);
      return false;
    }

  };

  char AAMPass::ID = 0;
  static RegisterPass<AAMPass> X("aam", "AAM Pass", false/*Only looks at CFG*/, false/*Analysis Pass*/);
}

