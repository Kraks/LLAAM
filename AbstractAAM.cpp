//
// Created by WeiGuannan on 08/02/2017.
//
#include "AbstractAAM.h"

namespace AbstractAAM {
  using namespace AAM;
  
  Function* ZeroCFAStackAddr::initFunc = nullptr;
  Module* AbsState::module = nullptr;
  unsigned long long AbsState::id = 0;
  
  void run(AbsState::StatePtrType s) {
    errs() << AbsState::getModule()->getSourceFileName() << "\n";
    
    AbsState::StateSetPtrType todo = StateSet::getMtSet();
    AbsState::StateSetPtrType done = StateSet::getMtSet();
    todo->inplaceInsert(s);
    
    while (todo->nonEmpty()) {
      auto currentState = todo->inplacePop();
      currentState->getConf()->getStore()->print();
      
      errs() << "\n";
      currentState->print();
      errs() << "\n";
      
      auto newStates = currentState->next();
      
      for (auto& s : newStates->getValueSet()) {
        if (!done->contains(s)) {
          todo->inplaceInsert(s);
        }
      }
      
      done->inplaceInsert(currentState);
    }
    
  }
  
  std::shared_ptr<AbsD> evalAtom(Value* val,
                                 std::shared_ptr<FrameAddr> fp,
                                 std::shared_ptr<AbsConf> conf,
                                 Module& M) {
    // If val is a constant integer, return a D contains one element
    if (ConstantInt* ci = dyn_cast<ConstantInt>(val)) {
      auto i = IntValue::makeInt(ci->getValue());
      return AbsD::makeD(i);
    }
    
    // If val is a left-hand side value, return the value of its address
    auto addr = BindAddr::makeBindAddr(val, fp);
    auto valOpt = conf->getStore()->lookup(addr);
    assert(valOpt.hasValue());
    auto value = valOpt.getValue();
    return value;
  }
  
  std::shared_ptr<AbsD> primOp(unsigned op,
                               std::shared_ptr<AbsD> lhs,
                               std::shared_ptr<AbsD> rhs) {
    assert(lhs->template verify<AnyIntValue>());
    assert(lhs->template verify<AnyIntValue>());
    
    auto results = AbsD::makeMtD();
    auto& lhsValues = lhs->getValueSet();
    auto& rhsValues = rhs->getValueSet();
    auto anyInt = AnyIntValue::getInstance();
    for (auto lit = lhsValues.begin(); lit != lhsValues.end(); lit++) {
      for (auto rit = rhsValues.begin(); rit != rhsValues.end(); rit++) {
        if (isa<IntValue>(**lit) && isa<IntValue>(**rit)) {
          auto lv = dyn_cast<IntValue>(&**lit)->getValue();
          auto rv = dyn_cast<IntValue>(&**rit)->getValue();
          APInt apInt;
          if (Instruction::Add == op) {
            apInt = lv + rv;
          }
          if (Instruction::Sub == op) {
            apInt = lv - rv;
          }
          if (Instruction::Mul == op) {
            apInt = lv * rv;
          }
          results->inplaceAdd(IntValue::makeInt(apInt));
        }
        else {
          results->inplaceAdd(anyInt);
        }
      }
    }
    return results;
  }
  
  std::shared_ptr<AbsLoc> addrsOf(Value* lhs,
                                  std::shared_ptr<FrameAddr> fp,
                                  std::shared_ptr<AbsConf> conf,
                                  Module& M) {
    Type* lhsType = lhs->getType();
    std::string var = lhs->getName();
    auto d = AbsLoc::makeMtD();
    
    if (lhsType->isIntegerTy()) {
      auto addr = BindAddr::makeBindAddr(lhs, fp);
      d->inplaceAdd(addr);
    }
    else if (lhsType->isPointerTy()) {
      if (M.getGlobalVariable(var, true)) {
        auto addr = BindAddr::makeBindAddr(lhs, ZeroCFAStackAddr::initFp());
        d->inplaceAdd(addr);
      }
      else {
        auto store = conf->getStore();
        auto fromAddr = BindAddr::makeBindAddr(lhs, fp);
        auto result = store->lookup(fromAddr);
        if (result.hasValue()) {
          auto vals = result.getValue();
          assert(vals->template verify<LocationValue>());
          for (auto& v : vals->getValueSet()) {
            auto loc = dyn_cast<LocationValue>(&*v)->getLocation();
            d->inplaceAdd(loc);
          }
        }
        else {
          assert(false && "Unbound variable");
          //TODO:
        }
      }
    }
    else if (lhsType->isAggregateType()) {
      assert(false && "TODO: lhs is an aggregate type");
    }
    else {
      assert(false && "TODO");
    }
    return d;
  }
  
  std::shared_ptr<AbsMeasure> getInitMeasure(Module& M) {
    std::shared_ptr<AbsMeasure> measure = std::make_shared<AbsMeasure>();
    auto initFp = ZeroCFAStackAddr::initFp(&M);
    Function* main = M.getFunction("main");
    auto& mainArgs = main->getArgumentList();
    
    //Put argc into measure
    auto arg_it = mainArgs.begin();
    auto argcBindAddr = BindAddr::makeBindAddr(&*arg_it, initFp);
    auto argcMeasure = AbstractNat::getOneInstance();
    measure->inplaceUpdate(argcBindAddr, argcMeasure);
    //TODO: argv
    
    return measure;
  }
  
  std::shared_ptr<AbsStore> getInitStore(Module& M) {
    std::shared_ptr<AbsStore> store = std::make_shared<AbsStore>();
    auto initFp = ZeroCFAStackAddr::initFp(&M);
    Function* main = M.getFunction("main");
    
    auto& mainArgs = main->getArgumentList();
    if (mainArgs.size() > 0) {
      assert(mainArgs.size() == 2);
      //Put argc and argv into store
      auto arg_it = mainArgs.begin();
      auto argcBindAddr = BindAddr::makeBindAddr(&*arg_it, initFp);
      auto argcValue = AbsD::makeD(AnyIntValue::getInstance());
      store->inplaceStrongUpdate(argcBindAddr, argcValue);
  
      arg_it++;
      //TODO: argv
      /*
      auto argvBindAddr = BindAddr::makeBindAddr(&*arg_it, initFp);
      auto argvValue = LocationValue::makeLocationValue(ZeroCFAHeapAddr::make(&*arg_it, 0));
      store->inplaceUpdate(argvBindAddr, AbsD::makeD(argvValue));
      */
    }
    
    auto& funcs = M.getFunctionList();
    for (auto& f : funcs) {
      //errs() << f.getName() << "\n";
      std::shared_ptr<BindAddr> b = std::make_shared<BindAddr>(f.getName(), initFp);
      std::shared_ptr<FuncValue> v = std::make_shared<FuncValue>(&f);
      std::shared_ptr<AbsD> d = AbsD::makeD(v);
      store->inplaceUpdate(b, d);
    }
    
    //TODO: if has argc && argv, update the assertion
    assert(store->size() == funcs.size());
    
    auto& globs = M.getGlobalList();
    for (auto& g : globs) {
      // NOTE: By using `getUniqueInteger()` we assume Int is the only primitive type
      //errs() << g.getName() << " : ";
      //errs() << g.getInitializer()->getUniqueInteger() << "\n";
      if (g.hasInitializer()) {
        Value* gv = dyn_cast<Value>(&g);
        std::shared_ptr<BindAddr> b = std::make_shared<BindAddr>(gv, initFp);
        std::shared_ptr<IntValue> v = std::make_shared<IntValue>(g.getInitializer()->getUniqueInteger());
        std::shared_ptr<AbsD> d = AbsD::makeD(v);
        store->inplaceUpdate(b, d);
      }
      else {
        assert(false && "TODO: g is a global declaration");
      }
    }
  
    //TODO: if has argc && argv, update the assertion
    assert(store->size() == (funcs.size() + globs.size()));
    return store;
  }
  
  std::shared_ptr<AbsConf> getInitConf(Module& M) {
    std::shared_ptr<AbsStore> store = getInitStore(M);
    std::shared_ptr<AbsSucc> succ = std::make_shared<AbsSucc>();
    std::shared_ptr<AbsPred> pred = std::make_shared<AbsPred>();
    std::shared_ptr<AbsMeasure> measure = getInitMeasure(M);
    std::shared_ptr<AbsConf> conf = AbsConf::makeAbsConf(store, succ, pred, measure);
    return conf;
  }
  
  /**
   * Immediately touchable locations for an abstract value.
   */
  std::shared_ptr<AbsLoc> touchable(std::shared_ptr<AbstractValue> v,
                                    std::shared_ptr<AbsConf> conf,
                                    Module& M) {
    auto result = AbsLoc::makeMtD();
    
    if (isa<AnyIntValue>(&*v)) { }
    else if (isa<BotValue>(&*v)) { }
    else if(isa<FuncValue>(&*v)) {
      auto fv = dyn_cast<FuncValue>(&*v);
      auto func = fv->getFunction();
      //Global va
    }
    else if (isa<LocationValue>(&*v)) {
      auto lv = dyn_cast<LocationValue>(&*v);
      auto loc = lv->getLocation();
      result->inplaceAdd(loc);
      
      auto predOpt = conf->getPred()->lookup(loc);
      if (predOpt.hasValue()) {
        for (auto& p : predOpt.getValue()->getValueSet()) {
          result->inplaceAdd(p);
        }
      }
      
      auto succOpt = conf->getSucc()->lookup(loc);
      if (succOpt.hasValue()) {
        for (auto& s : succOpt.getValue()->getValueSet()) {
          result->inplaceAdd(s);
        }
      }
    }
    else if (isa<Cont>(&*v)) {
      
    }
    else {
      
    }
  }
  
  std::shared_ptr<AbsLoc> touchable(std::shared_ptr<AbsD> d,
                                    std::shared_ptr<AbsConf> conf,
                                    Module& M) {
    auto& vals = d->getValueSet();
    auto result = AbsLoc::makeMtD();
    for (auto& v : vals) {
      auto ls = touchable(v, conf, M);
      result->inplaceJoin(ls);
    }
    return result;
  }
  
  /**
   * Immediate touchable value for a state(inst, fp, conf, sp),
   * includes live values at the entry of inst,
   *          frame pointer,
   *          arguments,
   *          declared variables may used in future,
   *          global variables
   */
  
}
