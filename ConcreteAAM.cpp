//
// Created by WeiGuannan on 13/02/2017.
//
#include "ConcreteAAM.h"

namespace ConcreteAAM {
  using namespace AAM;
  
  Module* ConcreteState::module = nullptr;
  unsigned long long ConcreteHeapAddr::id = 0;
  unsigned long long ConcreteStackAddr::id = 0;
  unsigned long long ConcreteState::id = 0;
  
  void run(ConcreteState::StatePtrType s) {
    auto state = s;
    auto nextState = state->next();
    while (!(*state == *nextState)) {
      state = nextState;
      nextState = state->next();
    }
  }
  
  /********  Auxiliary functions  ********/
  
  // TODO: addrOf
  // TODO: Prim operator
  
  // If val is a constant integer, return IntVal(val)
  std::shared_ptr<AbstractValue> evalAtom(ConstantInt* i,
                                          std::shared_ptr<FrameAddr> fp,
                                          std::shared_ptr<ConcreteConf> conf,
                                          Module& M) {
    errs() << "evalAtom constant int\n";
    return IntValue::makeInt(i->getValue());
  }
  
  std::shared_ptr<AbstractValue> evalAtom(Value* val,
                                          std::shared_ptr<FrameAddr> fp,
                                          std::shared_ptr<ConcreteConf> conf,
                                          Module& M) {
    errs() << "evalAtom value*\n";
    // If val is a left-hand side value, return the value of its address
    auto loc = addrsOf(val, fp, conf, M);
    auto locVal = LocationValue::makeLocationValue(loc);
    return locVal;
    // If val is a primitive operation, do the operation
    // If val is a sizeof operation
    
  }
  
  std::shared_ptr<Location> addrsOf(Value* lhs,
                                    std::shared_ptr<FrameAddr> fp,
                                    std::shared_ptr<ConcreteConf> conf,
                                    Module& M) {
    Type* lhsType = lhs->getType();
    std::string var = lhs->getName();
    if (M.getGlobalVariable(var)) {
      return std::make_shared<BindAddr>(lhs, ConcreteStackAddr::initFp());
    }
    else {
      errs() << "make a new bind addr\n";
      return std::make_shared<BindAddr>(lhs, fp);
    }
    
    /*
    if (lhsType->isPointerTy()) {
      // If the lvalue is a pointer(location), then query the store
      // to retrieve the location it points to.
      std::shared_ptr<ConcreteStore> store = conf->getStore();
      std::shared_ptr<Location> bind = std::make_shared<BindAddr>(lhs, fp);
      auto result = store->lookup(bind);
  
      if (result.hasValue()) {
        auto val = result.getValue();
        assert(isa<LocationValue>(*val) && "The result from store should be a Location");
        auto newVal = std::static_pointer_cast<LocationValue>(val);
        return newVal->getLocation();
      }
      else {
        assert(false && "Unbound variable");
        std::shared_ptr<Location> mt;
        return mt;
      }
    }
    else if (lhsType->isAggregateType()){
      assert(false && "TODO: lhs is an aggregate type");
    }
    else {
      // addrOf gets the location of a lhs variable name
      // If the variable is global, then use the initial frame pointer to
      // form the BindAddr, otherwise use current frame pointer.
      std::string var = lhs->getName();
      if (M.getGlobalVariable(var)) {
        return std::make_shared<BindAddr>(lhs, ConcreteAAM::initFp);
      }
      else {
        return std::make_shared<BindAddr>(lhs, fp);
      }
    }
     */
  }
  
  std::shared_ptr<ConcreteStore> getInitStore(Module& M) {
    std::shared_ptr<ConcreteStore> store = std::make_shared<ConcreteStore>();
    
    auto& funcs = M.getFunctionList();
    for (auto& f : funcs) {
      //errs() << f.getName() << "\n";
      std::shared_ptr<BindAddr> b = std::make_shared<BindAddr>(f.getName(), ConcreteStackAddr::initFp());
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
        Value* gv = dyn_cast<Value>(&g);
        std::shared_ptr<BindAddr> b = std::make_shared<BindAddr>(gv, ConcreteStackAddr::initFp());
        std::shared_ptr<IntValue> v = std::make_shared<IntValue>(g.getInitializer()->getUniqueInteger());
        store->inplaceUpdate(b, v);
      }
      else {
        assert(false && "TODO: g is a global declaration");
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
  
}

