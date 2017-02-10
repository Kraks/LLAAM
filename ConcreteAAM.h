//
// Created by WeiGuannan on 08/02/2017.
//
#include "AAM.h"

#ifndef LLVM_CONCRETEAAM_H
#define LLVM_CONCRETEAAM_H

//TODO Using template refactor Store/Succ/Pred

namespace ConcreteAAM {
  using namespace AAM;
  
  class ConcreteHeapAddr : public HeapAddr {
  public:
    ConcreteHeapAddr() : HeapAddr(KConcreteHeapAddr) {
      myId = id++;
    }
    
    static bool classof(const Location* loc) {
      return loc->getKind() == KConcreteHeapAddr;
    }
    
    virtual bool equalTo(const Location& that) const override {
      if (!isa<ConcreteHeapAddr>(&that))
        return false;
      auto* newThat = dyn_cast<ConcreteHeapAddr>(&that);
      //errs() << "that id: " << newThat->myId << " ; this id: " << this->myId << "\n";
      return newThat->myId == this->myId;
    }
    
    virtual size_t hashValue() const override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(myId));
      seed = hash_combine(seed, hash_value("ConcreteHeapAddr"));
      return seed;
    }
  
  private:
    unsigned long long myId;
    static unsigned long long id;
  };
  
  class ConcreteStackPtr : public StackPtr {
  public:
    ConcreteStackPtr() : StackPtr(KConcreteStackPtr) {
      myId = id++;
    }
    
    static bool classof(const Location* loc) {
      return loc->getKind() == KConcreteStackPtr;
    }
    
    virtual bool equalTo(const Location& that) const override {
      if (!isa<ConcreteStackPtr>(&that))
        return false;
      auto* newThat = dyn_cast<ConcreteStackPtr>(&that);
      return newThat->myId == this->myId;
    }
    
    virtual size_t hashValue() const override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(myId));
      seed =hash_combine(seed, hash_value("ConcreteStackAddr"));
      return seed;
    }
  
  private:
    unsigned long long myId;
    static unsigned long long id;
  };
  
  typedef ConcreteStackPtr ConcreteFramePtr;
  
  typedef Store<Location, AbstractValue, LocationLess> ConcreteStore;
  
  typedef Store<Location, Location, LocationLess> ConcreteSucc;
  
  typedef Store<Location, Location, LocationLess> ConcretePred;
  
  typedef bool DummyMeasure;
  typedef Conf<ConcreteStore, ConcreteSucc, ConcretePred, DummyMeasure> ConcreteConf;
  
  typedef State<Stmt, FramePtr, ConcreteConf, StackPtr> ConcreteState;
  
  /******** Static initialization ********/
  
  unsigned long long ConcreteHeapAddr::id = 0;
  unsigned long long ConcreteStackPtr::id = 0;
  
  /********  Auxiliary functions  ********/
  
  // TODO: global var/fun into a store
  // TODO: evalAtom
  // TODO: addrOf
  // TODO: Prim operator
  // TODO: add IntVal?
  
  std::shared_ptr<ConcreteStore> getInitStore(Module& M, std::shared_ptr<FramePtr> initFp) {
    std::shared_ptr<ConcreteStore> store = std::make_shared<ConcreteStore>();
    
    auto& funcs = M.getFunctionList();
    for (auto& f : funcs) {
      //errs() << f.getName() << "\n";
      std::shared_ptr<BindAddr> b = std::make_shared<BindAddr>(f.getName(), initFp);
      std::shared_ptr<FuncValue> v = std::make_shared<FuncValue>(&f);
      store->inplaceUpdate(b, v);
    }
    
    assert(store->size() == funcs.size());
    
    auto& globs = M.getGlobalList();
    for (auto& g : globs) {
      // NOTE: By using `getUniqueInteger()` we assume Int is the only primitive type
      //errs() << g.getName() << " : ";
      //errs() << g.getInitializer()->getUniqueInteger() << "\n";
      if (g.hasInitializer()) {
        std::shared_ptr<BindAddr> b = std::make_shared<BindAddr>(g.getName(), initFp);
        std::shared_ptr<IntValue> v = std::make_shared<IntValue>(g.getInitializer()->getUniqueInteger());
        store->inplaceUpdate(b, v);
      }
      else {
        // g is a global declaration
      }
    }
    
    assert(store->size() == (funcs.size() + globs.size()));
    
    return store;
  }
  
  /*
  std::shared_ptr<ConcreteState> inject(Module& M, std::string mainFuncName) {
    Function* main = M.getFunction(mainFuncName);
    Instruction* entry = getEntry(*main);
    
  }
   */
}

#endif //LLVM_CONCRETEAAM_H
