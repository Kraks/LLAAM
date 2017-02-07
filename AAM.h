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

  struct Location {
    unsigned long long myId;
    Location() {
      myId = id++;
    };
    friend bool operator < (const struct Location &a, const struct Location &b) {
      return a.myId < b.myId;
    }
  private:
    static unsigned long long id;
  };

  unsigned long long Location::id = 0;

  struct HeapAddr : public Location {};
  struct StackPtr : public Location {};
  typedef StackPtr FramePtr;
  struct BindAddr : public Location {};

  struct LocalBindAddr : public BindAddr {
    var name;
    FramePtr fp;
    LocalBindAddr(var name, FramePtr fp) : name(name), fp(fp) {};
  };

  struct AbstractValue {
    unsigned long long myId;
  };
  struct Cont : public AbstractValue {};
  struct LocationValue : public AbstractValue {
    Location loc;
    LocationValue(Location loc) : loc(loc) {};
  };
  struct FuncValue : public AbstractValue {
    Function& fun;
    FuncValue(Function& fun) : fun(fun) {};
  };
  struct PrimValue : public AbstractValue {
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

  struct Store {};
  struct Succ {};
  struct Pred {};
  struct Measure {};
  struct Conf {};

  struct State {};
}

namespace ConcreteAAM {
  using namespace AAM;

  struct ConcreteHeapAddr : public HeapAddr {
    unsigned long long myId;
    ConcreteHeapAddr() {
      myId = id++;
    }
  private:
    static unsigned long long id;
  };

  struct ConcreteStackAddr : public Location {
    unsigned long long myId;
    ConcreteStackAddr() {
      myId = id++;
    }
  private:
    static unsigned long long id;
  };

  typedef ConcreteStackAddr ConcreteFramePtr;

  struct ConcreteStore : public Store {
    std::map<Location, AbstractValue> m;
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
  unsigned long long ConcreteStackAddr::id = 0;
}

namespace AbstractAAM {
  using namespace AAM;

  struct ZeroCFAHeapAddr : public HeapAddr {
    //TODO
  };

  struct ZeroCFAStackPtr : public StackPtr {
    //TODO
  };

  typedef ZeroCFAStackPtr ZeroCFAFramePtr;

  struct AbstractStore : public Store {
    std::set<AbstractValue> s;

  };
}


#endif //LLVM_AAM_H
