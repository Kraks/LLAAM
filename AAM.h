//
// Created by WeiGuannan on 19/01/2017.
//

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

#include <set>
#include <map>
#include <vector>
#include <algorithm>

#ifndef LLVM_AAM_H
#define LLVM_AAM_H

using namespace llvm;

namespace AAM {
  typedef std::string var;

  class Location {
  public:
    unsigned long long myId;
    Location() {
      myId = id++;
    };
    friend bool operator < (const Location &a, const Location &b) {
      return a.myId < b.myId;
    }
  private:
    static unsigned long long id;
  };

  unsigned long long Location::id = 0;

  class HeapAddr : public Location {};
  class StackPtr : public Location {};
  typedef StackPtr FramePtr;
  class BindAddr : public Location {};

  class LocalBindAddr : public BindAddr {
    var name;
    FramePtr fp;
  public:
    LocalBindAddr(var name, FramePtr fp) : name(name), fp(fp) {};
  };

  class AbstractValue {
    unsigned long long myId;
  public:
    inline bool operator==(const AbstractValue& that) {
      return false;
    }
  };
  class Cont : public AbstractValue {};
  class LocationValue : public AbstractValue {
    Location loc;
  public:
    LocationValue(Location loc) : loc(loc) {};
  };
  class FuncValue : public AbstractValue {
    Function& fun;
  public:
    FuncValue(Function& fun) : fun(fun) {};
    inline bool operator==(const FuncValue& that) {
      return &this->fun == &that.fun;
    }
  };
  class PrimValue : public AbstractValue {
  public:
    static PrimValue& getInstance() {
      static PrimValue instance;
      return instance;
    }
    PrimValue(PrimValue const&) = delete;
    void operator=(PrimValue const&) = delete;

  private:
    PrimValue() {}
  };

  enum AbstractNat { Zero, One, Inf };

  class Store {};
  class Succ {};
  class Pred {};
  class Measure {};
  class Conf {};

  struct State {};
}

namespace ConcreteAAM {
  using namespace AAM;

  class ConcreteHeapAddr : public HeapAddr {
    unsigned long long myId;
  public:
    ConcreteHeapAddr() {
      myId = id++;
    }
  private:
    static unsigned long long id;
  };

  class ConcreteStackPtr : public Location {
    unsigned long long myId;
  public:
    ConcreteStackPtr() {
      myId = id++;
    }
  private:
    static unsigned long long id;
  };

  typedef ConcreteStackPtr ConcreteFramePtr;

  struct ConcreteStore : public Store {
    std::map<Location, AbstractValue> m;
  public:
    ConcreteStore() {};
    ConcreteStore(std::map<Location, AbstractValue> m) : m(m) {};

    AbstractValue* lookup(Location& loc) {
      auto it = m.find(loc);
      if (it != m.end()) return &it->second;
      return nullptr;
    }

    ConcreteStore update(Location& loc, AbstractValue& val) {
      auto newMap = m;
      auto it = newMap.find(loc);
      if (it != newMap.end()) {
        it->second = val;
      }
      else {
        newMap.insert({loc, val});
      }
      ConcreteStore newStore(newMap);
      return newStore;
    }
  };

  //////////////////////////////////////////

  unsigned long long ConcreteHeapAddr::id = 0;
  unsigned long long ConcreteStackPtr::id = 0;
}

namespace AbstractAAM {
  using namespace AAM;

  class ZeroCFAHeapAddr : public HeapAddr {
    //TODO
  };

  class ZeroCFAStackPtr : public StackPtr {
    //TODO
  };

  typedef ZeroCFAStackPtr ZeroCFAFramePtr;

  class AbstractStore : public Store {
    std::set<AbstractValue> s;

  };
}


#endif //LLVM_AAM_H
