//
// Created by WeiGuannan on 08/02/2017.
//
#include "AbstractAAM.h"

namespace AbstractAAM {
  using namespace AAM;
  
  Function* ZeroCFAStackAddr::initFunc = nullptr;
  Module* AbsState::module = nullptr;
  unsigned long long AbsState::id = 0;
  
  std::shared_ptr<AbsStore> getInitStore(Module& M) {
    std::shared_ptr<AbsStore> store = std::make_shared<AbsStore>();
    auto initFp = ZeroCFAStackAddr::initFp();
    auto& funcs = M.getFunctionList();
    for (auto& f : funcs) {
      //errs() << f.getName() << "\n";
      std::shared_ptr<BindAddr> b = std::make_shared<BindAddr>(f.getName(), initFp);
      std::shared_ptr<FuncValue> v = std::make_shared<FuncValue>(&f);
      std::shared_ptr<AbsD> d = AbsD::makeD(v);
      store->inplaceUpdate(b, d);
    }
  
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
  
    assert(store->size() == (funcs.size() + globs.size()));
  
    return store;
  }
  
  std::shared_ptr<AbsConf> getInitConf(Module& M) {
    std::shared_ptr<AbsStore> store = getInitStore(M);
    std::shared_ptr<AbsSucc> succ = std::make_shared<AbsSucc>();
    std::shared_ptr<AbsPred> pred = std::make_shared<AbsPred>();
    std::shared_ptr<AbsMeasure> measure = std::make_shared<AbsMeasure>();
    std::shared_ptr<AbsConf> conf = AbsConf::makeAbsConf(store, succ, pred, measure);
    return conf;
  }
}
