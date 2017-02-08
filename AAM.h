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
#include <memory>

#ifndef LLVM_AAM_H
#define LLVM_AAM_H

using namespace llvm;

namespace AAM {
  typedef std::string var;

  /* Location = HeapAddr | StackPtr | FramePtr | BindAddr
   */
  class Location {
  public:
    unsigned long long myId;
    Location() {
      myId = id++;
    };
    /* Used for when as Key type of map */
    friend bool operator<(const Location &a, const Location &b) {
      return a.myId < b.myId;
    }
    inline bool operator==(Location& that) {
      errs() << "Location operator==\n";
      return false;
    }
  private:
    static unsigned long long id;
  };

  unsigned long long Location::id = 0;

  class HeapAddr : public Location {
  public:
    inline bool operator==(HeapAddr& that) { return false; }
  };

  class StackPtr : public Location {
  public:
    inline bool operator==(StackPtr& that) { return false; }
  };

  typedef StackPtr FramePtr;

  class BindAddr : public Location {
  public:
    inline bool operator==(BindAddr& that) { return false; }
  };

  class LocalBindAddr : public BindAddr {
    var name;
    FramePtr fp;
  public:
    LocalBindAddr(var name, FramePtr fp) : name(name), fp(fp) {};
    inline bool operator==(LocalBindAddr& that) {
      return (this->name == that.name && this->fp == that.fp);
    }
  };

  /* AbstractValue = Cont | LocationValue | FuncValue | PrimValue
   */
  class AbstractValue {
  public:
    enum ValType {ContV, LocationV, FuncV, PrimV};
    static std::string valTypeToString(ValType v) {
      switch (v) {
        case ContV: return "ContV";
        case LocationV: return "LocationV";
        case FuncV: return "FuncV";
        case PrimV: return "PrimV";
      }
    }
    ValType getValType() {
      return valType;
    }
    inline bool operator==(const AbstractValue& that) {
      return false;
    }
  private:
    unsigned long long myId;
    ValType valType;
    AbstractValue() {}
  protected:
    AbstractValue(ValType valType): valType(valType) {}
  };

  class Cont : public AbstractValue {
    Cont() : AbstractValue(ContV) {}
  };

  class LocationValue : public AbstractValue {
    Location loc;
  public:
    LocationValue(Location loc) : AbstractValue(LocationV), loc(loc) {}
  };

  class FuncValue : public AbstractValue {
    Function* fun;
  public:
    FuncValue(Function* fun) : AbstractValue(FuncV), fun(fun) {}
    FuncValue(const FuncValue& funcValue) : AbstractValue(FuncV), fun(funcValue.fun) {}
    
    FuncValue& operator=(FuncValue that) {
      this->fun = that.fun;
      return *this;
    }

    std::string getFunctionName() const {
      return fun->getName();
    }
  
    inline bool operator==(const FuncValue& that) {
      errs() << "FuncValue operator== " << this->getFunctionName();
      errs() << " " << that.getFunctionName() << "\n";
      return this->fun == that.fun;
      //return this->getFunctionName() == that.getFunctionName();
    }
  };

  class PrimValue : public AbstractValue {
  public:
    PrimValue() : AbstractValue(PrimV) {}
    inline bool operator==(const PrimValue& that) {
      return true;
    }
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
    inline bool operator==(ConcreteHeapAddr& that) {
      return (this->myId == that.myId);
    }
  private:
    static unsigned long long id;
  };

  class ConcreteStackPtr : public StackPtr {
    unsigned long long myId;
  public:
    ConcreteStackPtr() {
      myId = id++;
    }
    inline bool operator==(ConcreteStackPtr& that) {
      errs() << "ConcreteStackPtr operator==\n";
      return (this->myId == that.myId);
    }
  private:
    static unsigned long long id;
  };

  typedef ConcreteStackPtr ConcreteFramePtr;

  struct ConcreteStore : public Store {
    typedef std::map<Location, std::shared_ptr<AbstractValue>> StoreMap;
    StoreMap m;
  public:
    ConcreteStore() {};
    ConcreteStore(StoreMap m) : m(m) {};

    AbstractValue* lookup(Location& loc) {
      auto it = m.find(loc);
      if (it != m.end()) return it->second.get();
      return nullptr;
    }
    
    /*
    ConcreteStore update(Location& loc, AbstractValue& val) {
      auto newMap = m;
      auto it = newMap.find(loc);
      if (it != newMap.end()) {
        it->second = &val;
      }
      else {
        newMap.insert({loc, val});
      }
      ConcreteStore newStore(newMap);
      return newStore;
    }
    
    inline bool operator==(ConcreteStore& that) {
      return (this->m == that.m);
    }
     */
  };

  //////////////////////////////////////////

  unsigned long long ConcreteHeapAddr::id = 0;
  unsigned long long ConcreteStackPtr::id = 0;
}

namespace AbstractAAM {
  using namespace AAM;

  /*
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
  */
}


#endif //LLVM_AAM_H
