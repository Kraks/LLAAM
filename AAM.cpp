// Created by WeiGuannan on 14/01/2017.
//

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
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
    
    static void test1(Module& M) {
      Function* mainFunc = M.getFunction("main");
      //Function* add = M.getFunction("add");
      //Function* sub = M.getFunction("sub");
  
      std::shared_ptr<FuncValue> f1 = std::make_shared<FuncValue>(mainFunc);
      std::shared_ptr<FuncValue> f2 = std::make_shared<FuncValue>(mainFunc);
      //std::shared_ptr<PrimValue> pv = std::make_shared<PrimValue>();
      std::shared_ptr<PrimValue> pv = PrimValue::getInstance();
      assert((*f1.get() == *f2.get()));
      //errs() << "functions eq: " << (*f1.get() == *f2.get()) << "\n"; // true
      
      std::shared_ptr<ConcreteStackAddr> sp1 = std::make_shared<ConcreteStackAddr>();
      assert(isa<ConcreteStackAddr>(*sp1.get()));
      assert(isa<StackAddr>(*sp1.get()));
      
      std::shared_ptr<ConcreteStackAddr> sp2 = std::make_shared<ConcreteStackAddr>();
      assert(isa<ConcreteStackAddr>(*sp2.get()));
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
      
      std::shared_ptr<ConcreteStackAddr> sp3 = std::make_shared<ConcreteStackAddr>();
      
      std::shared_ptr<BindAddr> baddr1 = std::make_shared<BindAddr>("x", sp3);
      std::shared_ptr<BindAddr> baddr2 = std::make_shared<BindAddr>("x", sp3);
      assert(isa<BAddr>(*baddr1.get()));
      assert(isa<BindAddr>(*baddr1.get()));
      assert(*baddr1.get() == *baddr1.get());
      assert(*baddr1 == *baddr1);
      assert(*baddr1.get() == *baddr2.get());
      assert(baddr1->hashValue() == baddr2->hashValue());
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
      assert(store2.hashValue() == store3.hashValue());
      assert(store.hashValue() != store2.hashValue());
      
      shared_ptr<AbstractValue> av = store.lookup(baddr2).getValue();
      assert(av == lv1);
      assert(*av == *lv1);
      
      // copy store
      ConcreteStore another_store = store;
      another_store.inplaceRemove(sp1);
      assert(another_store.size() == 4);
      
      // empty store
      std::shared_ptr<ConcreteStore> mt_store = std::make_shared<ConcreteStore>();
      assert(mt_store->size() == 0);
      
      shared_ptr<BindAddr> baddr3 = make_shared<BindAddr>("y", sp3);
      assert(!store.lookup(baddr3).hasValue());
      
      shared_ptr<AbstractValue> v1 = store.lookup(sp1).getValue();
      shared_ptr<AbstractValue> v2 = store.lookup(sp2).getValue();
      shared_ptr<FuncValue> f11 = static_pointer_cast<FuncValue>(v1);
      shared_ptr<FuncValue> f22 = static_pointer_cast<FuncValue>(v2);
      assert(*f11 == *f22);
      assert(f11->hashValue() == f22->hashValue());
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
      assert(pv1->hashValue() == pv2->hashValue());
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
      Instruction* i2 = getSyntacticNextInst(i1);
      
      shared_ptr<Cont> c1 = make_shared<Cont>("x", i1, sp1, sp2);
      assert(*c1 == *c1);
      shared_ptr<Cont> c2 = make_shared<Cont>("y", i2, sp1, sp2);
      assert(*c2 == *c2);
      assert(!(*c1 == *c2));
      
      shared_ptr<FrameAddr> sp1_copy = c1->getFrameAddr();
      assert(sp1.get() == sp1_copy.get());
    }
    
    static void test2(Module& M) {
      Function* mainFunc = M.getFunction("main");
      printInstructions(*mainFunc);
      BasicBlock& entry = mainFunc->getEntryBlock();
      
      Instruction* i1 = getEntry(*mainFunc);
      Instruction* i2 = getSyntacticNextInst(i1);
      Instruction* i3 = getSyntacticNextInst(i2);
  
      assert(i1 == i1);
      //assert(*i1 == *i1); //looks like there is no operator== for llvm::Instruction
      assert(i1 != i2);
      
      shared_ptr<Stmt> s1 = make_shared<Stmt>(i1);
      shared_ptr<Stmt> s2 = make_shared<Stmt>(i2);
      shared_ptr<Stmt> s3 = make_shared<Stmt>(i3);
      assert(*s1 == *s1);
      assert(!(*s1 == *s2));
      
      auto int1 = IntValue::makeInt(-5);
      auto int2 = IntValue::makeInt(5);
      auto int3 = IntValue::makeInt(5);
      assert(int2 != int3);
      assert(*int2 == *int3);
      assert(!(*int1 == *int2));
      assert(int2->hashValue() == int3->hashValue());
      assert(int2->hashValue() != int1->hashValue());
      
      std::shared_ptr<FrameAddr> fp = std::make_shared<ConcreteFrameAddr>();
      auto store = getInitStore(M);
      
      auto& args = mainFunc->getArgumentList();
      if (args.size() != 0) {
        auto& argc = args.front();
        auto* argv = args.getNext(&argc);
        errs() << "argc: " << &argc << "\n";
        long long llarg = reinterpret_cast<long long>(&argc);
        errs() << "argv: " << argv  << "\n";
        
        // This test shows that we can use Value* as unique identifier
        // of a vairable.
        forEachInst(*mainFunc, [&argc, &argv] (Instruction* inst) {
          if (isa<StoreInst>(inst)) {
            auto* sinst = dyn_cast<StoreInst>(inst);
            errs() << "  ";
            sinst->print(errs(), false);
            Value* val = sinst->getValueOperand();
            Value* ptr = sinst->getPointerOperand();
            errs() << "\n";
            errs() << "    inst name: " << sinst << "\n";
            errs() << "    val: " << val << "\n";
            errs() << "         val == argc: " << (val == &argc) << "\n";
            errs() << "         val == argv: " << (val ==  argv) << "\n";
            errs() << "    ptr: " << ptr << "\n";
          }
        });
      }
  
      StateSet<ConcreteState> todo;
      std::shared_ptr<ConcreteState> state = ConcreteState::inject(M, "main");
      todo.inplaceInsert(state);
      assert(todo.contains(state));
      assert(*state == *state);
  
      auto state1 = state->next();
      assert(!(*state1 == *state));
      
      auto state1_copy = state->next();
      assert(*state1 == *state1_copy);
      //assert(state1->hashValue() == state1_copy->hashValue());
      
      // state1 and state1_copy are the same, so the set should only have one of them
      todo.inplaceInsert(state1);
      todo.inplaceInsert(state1_copy);
      //errs() << "todo size: " << todo.size() << "\n";
      //todo.dump();
      assert(todo.size() == 2);
      
      auto head = todo.inplacePop();
      assert(todo.size() == 1);
      assert((head == state) || (head == state1));
      assert((*head == *state) || (*head == *state1));
      if (*head == *state) { assert(todo.contains(state1)); }
      if (head == state) { assert(todo.contains(state1)); }
      if (*head == *state1) { assert(todo.contains(state)); }
      if (head == state1) { assert(todo.contains(state)); }
      
      auto state2 = state1->next();
      todo.inplaceInsert(state2);
      assert(todo.size() == 2);
    }
    
    static void test3(Module& M) {
      std::shared_ptr<ConcreteState> state = ConcreteState::inject(M, "main");
      auto nextStaate = state->next();
      
    }

    bool runOnModule(Module& M) override {
      ConcreteState::setModule(&M);
      //test1(M);
      //test2(M);
      test3(M);
      return false;
    }

  };

  char AAMPass::ID = 0;
  static RegisterPass<AAMPass> X("aam", "AAM Pass", false/*Only looks at CFG*/, false/*Analysis Pass*/);
}

