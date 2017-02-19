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
    errs() << ConcreteState::getModule()->getSourceFileName() << "\n";
    auto state = s;
    auto nextState = state->next();
    while (!(*state == *nextState)) {
      state = nextState;
      nextState = state->next();
      nextState->getConf()->getStore()->print();
    }
  }
  
  /********  Auxiliary functions  ********/
  
  // TODO: addrOf
  // TODO: Prim operator
  
  std::shared_ptr<AbstractValue> evalAtom(Value* val,
                                          std::shared_ptr<FrameAddr> fp,
                                          std::shared_ptr<ConcreteConf> conf,
                                          Module& M) {
    if (ConstantInt* ci = dyn_cast<ConstantInt>(val)) {
      return IntValue::makeInt(ci->getValue());
    }
    // If val is a left-hand side value, return the value of its address
    auto addr = BindAddr::makeBindAddr(val, fp);
    auto valOpt = conf->getStore()->lookup(addr);
    assert(valOpt.hasValue());
    auto value = valOpt.getValue();
    return value;
    
    // If val is a primitive operation, do the operation
    // If val is a sizeof operation
  }
  
  std::shared_ptr<Location> addrsOf(Value* lhs,
                                    std::shared_ptr<FrameAddr> fp,
                                    std::shared_ptr<ConcreteConf> conf,
                                    Module& M) {
    Type* lhsType = lhs->getType();
    std::string var = lhs->getName();
    errs() << "lhsType: " << lhsType->getTypeID() << ", ";
    lhsType->print(errs());
    errs() << "\n";
    errs() << "var name: " << var << "\n";
  
    if (lhsType->isIntegerTy()) {
      auto addr = std::make_shared<BindAddr>(lhs, fp);
      errs() << "make a new bind addr: ";
      addr->print();
      errs() << "\n";
      return addr;
    }
    else if (lhsType->isPointerTy()) {
      if (M.getGlobalVariable(var, true)) {
        return std::make_shared<BindAddr>(lhs, ConcreteStackAddr::initFp());
      }
      
      std::shared_ptr<ConcreteStore> store = conf->getStore();
      std::shared_ptr<Location> bind = std::make_shared<BindAddr>(lhs, fp);
      
      auto result = store->lookup(bind);
      if (result.hasValue()) {
        auto val = result.getValue();
        assert(isa<LocationValue>(*val) && "The result from store should be a Location/Function");
        auto newVal = std::static_pointer_cast<LocationValue>(val);
        return newVal->getLocation();
      }
      else {
        assert(false && "Unbound variable");
        std::shared_ptr<Location> mt;
        return mt;
      }
    }
    else if (lhsType->isAggregateType()) {
      assert(false && "TODO: lhs is an aggregate type");
    }
    else {
      assert(false && "TODO");
    }
    
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

