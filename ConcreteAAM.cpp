//
// Created by WeiGuannan on 13/02/2017.
//
#include "ConcreteAAM.h"

namespace ConcreteAAM {
  using namespace AAM;
  
  unsigned long long ConcreteHeapAddr::id = 0;
  unsigned long long ConcreteStackPtr::id = 0;
  
  /********  Auxiliary functions  ********/
  
  // TODO: addrOf
  // TODO: Prim operator
  
  // If val is a constant integer, return IntVal(val)
  std::shared_ptr<AbstractValue> evalAtom(ConstantInt* i,
                                          std::shared_ptr<FramePtr> fp,
                                          std::shared_ptr<ConcreteConf> conf,
                                          Module& M) {
    const APInt& apInt = i->getValue();
    return IntValue::makeInt(4);
  }
  
  std::shared_ptr<AbstractValue> evalAtom(Value* val,
                                          std::shared_ptr<FramePtr> fp,
                                          std::shared_ptr<ConcreteConf> conf,
                                          Module& M) {
    // If val is a left-hand side value, return the value of its address
    auto loc = addrsOf(val, fp, conf, M);
    auto locVal = LocationValue::makeLocationValue(loc);
    return locVal;
    // If val is a primitive operation, do the operation
    // If val is a sizeof operation
    
  }
  
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
    // TODO: some assertion on lhs?
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
  
  // TODO: addrsOf, If the lvalue is accessing some aggregate data,
  
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
  
}

