//
// Created by WeiGuannan on 08/02/2017.
//
#include "AbstractAAM.h"

namespace AbstractAAM {
  using namespace AAM;
  
  Function* ZeroCFAStackAddr::initFunc = nullptr;
  Module* AbsState::module = nullptr;
  unsigned long long AbsState::id = 0;
  
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
    //Put argc and argv into store
    auto arg_it = mainArgs.begin();
    auto argcBindAddr = BindAddr::makeBindAddr(&*arg_it, initFp);
    auto argcValue = AbsD::makeD(AnyIntValue::getInstance());
    store->inplaceStrongUpdate(argcBindAddr, argcValue);
    
    arg_it++;
    //TODO: argv
    auto argvBindAddr = BindAddr::makeBindAddr(&*arg_it, initFp);
    auto argvValue = LocationValue::makeLocationValue(ZeroCFAHeapAddr::make(&*arg_it, 0));
    store->inplaceUpdate(argvBindAddr, AbsD::makeD(argvValue));
    
    auto& funcs = M.getFunctionList();
    for (auto& f : funcs) {
      //errs() << f.getName() << "\n";
      std::shared_ptr<BindAddr> b = std::make_shared<BindAddr>(f.getName(), initFp);
      std::shared_ptr<FuncValue> v = std::make_shared<FuncValue>(&f);
      std::shared_ptr<AbsD> d = AbsD::makeD(v);
      store->inplaceUpdate(b, d);
    }
  
    assert(store->size() == funcs.size() + 1);
    
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
  
    assert(store->size() == (funcs.size() + globs.size() + 1));
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
}
