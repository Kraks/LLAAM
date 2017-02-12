//
// Created by WeiGuannan on 08/02/2017.
//
#include "AAM.h"

#ifndef LLVM_CONCRETEAAM_H
#define LLVM_CONCRETEAAM_H

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
  
  /******** Static initialization ********/
  
  unsigned long long ConcreteHeapAddr::id = 0;
  unsigned long long ConcreteStackPtr::id = 0;
  const static std::shared_ptr<FramePtr> initFp = std::make_shared<ConcreteFramePtr>();
  
  /********  Auxiliary functions  ********/
  
  // TODO: evalAtom
  // TODO: addrOf
  // TODO: Prim operator
  // TODO: add IntVal?
  
  // addrOf gets the location of a lvalue
  // If the variable is global, then use the initial frame pointer to
  // form the BindAddr, otherwise use current frame pointer.
  std::shared_ptr<Location> addrsOf(std::string var,
                                    std::shared_ptr<FramePtr> fp,
                                    std::shared_ptr<ConcreteConf> conf,
                                    Module& M) {
    if (M.getGlobalVariable(var)) {
      return std::make_shared<BindAddr>(var, ConcreteAAM::initFp);
    }
    else {
      return std::make_shared<BindAddr>(var, fp);
    }
  }
  // If the lvalue is a pointer(location), then query the store
  // to retrieve the location it points to.
  std::shared_ptr<Location> addrsOf(Value* lhs,
                                    std::shared_ptr<FramePtr> fp,
                                    std::shared_ptr<ConcreteConf> conf,
                                    Module& M) {
    std::shared_ptr<ConcreteStore> store = conf->getStore();
    // TODO: some assertion on lhs
    std::string var = lhs->getName();
    std::shared_ptr<Location> bind = std::make_shared<BindAddr>(var, fp);
    auto result = store->lookup(bind);
    
    if (result.hasValue()) {
      auto val = result.getValue();
      assert(isa<LocationValue>(*val) && "The result from store should be a Location");
      auto newVal = std::static_pointer_cast<LocationValue>(val);
      return newVal->getLocation();
    }
    else {
      assert(false && "Unbound variable");
      // TODO: return something?
    }
  }
  
  // If the lvalue is accessing some aggregate data,
  // TODO
  
  std::shared_ptr<ConcreteStore> getInitStore(Module& M) {
    std::shared_ptr<ConcreteStore> store = std::make_shared<ConcreteStore>();
    
    auto& funcs = M.getFunctionList();
    for (auto& f : funcs) {
      //errs() << f.getName() << "\n";
      std::shared_ptr<BindAddr> b = std::make_shared<BindAddr>(f.getName(), ConcreteAAM::initFp);
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
        std::shared_ptr<BindAddr> b = std::make_shared<BindAddr>(g.getName(), ConcreteAAM::initFp);
        std::shared_ptr<IntValue> v = std::make_shared<IntValue>(g.getInitializer()->getUniqueInteger());
        store->inplaceUpdate(b, v);
      }
      else {
        // otherwise `g` is a global declaration
      }
    }
    
    assert(store->size() == (funcs.size() + globs.size()));
    
    return store;
  }
  
  std::shared_ptr<ConcreteConf> getInitConf(Module& M) {
    std::shared_ptr<ConcreteStore> store = getInitStore(M);
    std::shared_ptr<ConcreteSucc> succ = std::make_shared<ConcreteSucc>();
    std::shared_ptr<ConcretePred> pred = std::make_shared<ConcretePred>();
    std::shared_ptr<ConcreteConf> conf = std::make_shared<ConcreteConf>(store, succ, pred);
    return conf;
  }

  class ConcreteState : public State<Stmt, FramePtr, ConcreteConf, StackPtr> {
  public:
    typedef std::shared_ptr<ConcreteState> StatePtrType;
    
    ConcreteState(CPtrType c, EPtrType e, SPtrType s, KPtrType k) :
      State(c, e, s, k) { };
    
    StatePtrType next() {
      Instruction* nextInst = getSyntacticNextInst(getControl()->getInst());
      auto stmt = Stmt::makeStmt(nextInst);
      auto state = makeState(stmt, getEnv(), getStore(), getCont());
      return state;
    }
    
    static StatePtrType inject(Module& M, std::string mainFuncName) {
      std::shared_ptr<ConcreteConf> initConf = getInitConf(M);
      Function* main = M.getFunction(mainFuncName);
      Instruction* entry = getEntry(*main);
      std::shared_ptr<Stmt> initStmt = std::make_shared<Stmt>(entry);
      std::shared_ptr<ConcreteState> initState = makeState(initStmt, initFp, initConf, initFp);
      return initState;
    }
    
    static StatePtrType makeState(CPtrType c, EPtrType e, SPtrType s, KPtrType k) {
      std::shared_ptr<ConcreteState> state = std::make_shared<ConcreteState>(c, e, s, k);
      return state;
    }
  };
}

#endif //LLVM_CONCRETEAAM_H
