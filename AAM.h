//
// Created by WeiGuannan on 19/01/2017.
//

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Casting.h"

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

  /* Location = HeapAddr
   *          | StackPtr
   *          | FramePtr
   *          | BindAddr
   */
  class Location {
  public:
    enum LocationKind {
      KHeapAddr,
        KConcreteHeapAddr,
        KZeroCFAHeapAddr,
      KStackPtr,
        KConcreteStackPtr,
        KZeroCFAStackPtr,
      KBindAddr,
        KLocalBindAddr
    };
    
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

  protected:
    Location(LocationKind kind) : kind(kind) {}
    
  private:
    LocationKind kind;
    unsigned long long myId;
    static unsigned long long id;
  };

  unsigned long long Location::id = 0;

  class HeapAddr : public Location {
  public:
    inline bool operator==(HeapAddr& that) { return false; }

  protected:
    HeapAddr(LocationKind kind) : Location(kind) {}
  };

  class StackPtr : public Location {
  public:
    inline bool operator==(StackPtr& that) { return false; }

  protected:
    StackPtr(LocationKind kind) : Location(kind) {}
  };

  typedef StackPtr FramePtr;

  class BindAddr : public Location {
  public:
    inline bool operator==(BindAddr& that) { return false; }

  protected:
    BindAddr(LocationKind kind) : Location(kind) {}
  };

  class LocalBindAddr : public BindAddr {
  public:
    LocalBindAddr(var name, FramePtr fp) : BindAddr(KLocalBindAddr), name(name), fp(fp) {};
    inline bool operator==(LocalBindAddr& that) {
      return (this->name == that.name && this->fp == that.fp);
    }
    
  private:
    var name;
    FramePtr fp;
  };

  /* AbstractValue = Cont
   *               | LocationValue
   *               | FuncValue
   *               | PrimValue
   */
  class AbstractValue {
  public:
    enum ValKind { KContV, KLocationV, KFuncV, KPrimV };
    
    static std::string valTypeToString(ValKind v) {
      switch (v) {
        case KContV: return "ContV";
        case KLocationV: return "LocationV";
        case KFuncV: return "FuncV";
        case KPrimV: return "PrimV";
      }
    }
    
    ValKind getKind() const {
      return kind;
    }
    
    inline bool operator==(const AbstractValue& that) {
      return false;
    }
    
  private:
    ValKind kind;
    unsigned long long myId;
    AbstractValue() {}
    
  protected:
    AbstractValue(ValKind kind): kind(kind) {}
  };

  class Cont : public AbstractValue {
    Cont() : AbstractValue(KContV) {}
    
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KContV;
    }
  };

  class LocationValue : public AbstractValue {
    Location loc;
  public:
    LocationValue(Location loc) : AbstractValue(KLocationV), loc(loc) {}
    
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KLocationV;
    }
  };

  class FuncValue : public AbstractValue {
    Function* fun;
  public:
    FuncValue(Function* fun) : AbstractValue(KFuncV), fun(fun) {}
    FuncValue(const FuncValue& funcValue) : AbstractValue(KFuncV), fun(funcValue.fun) {}
    
    FuncValue& operator=(FuncValue that) {
      this->fun = that.fun;
      return *this;
    }

    std::string getFunctionName() const {
      return fun->getName();
    }
  
    inline bool operator==(const FuncValue& that) {
      //errs() << "FuncValue operator== " << this->getFunctionName();
      //errs() << " " << that.getFunctionName() << "\n";
      return this->fun == that.fun;
    }
  
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KFuncV;
    }
  };

  class PrimValue : public AbstractValue {
  public:
    PrimValue() : AbstractValue(KPrimV) {}
    inline bool operator==(const PrimValue& that) {
      return true;
    }
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KPrimV;
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
  public:
    ConcreteHeapAddr() : HeapAddr(KConcreteHeapAddr) {
      myId = id++;
    }
    
    inline bool operator==(ConcreteHeapAddr& that) {
      return (this->myId == that.myId);
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
    
    inline bool operator==(ConcreteStackPtr& that) {
      return (this->myId == that.myId);
    }
    
  private:
    unsigned long long myId;
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
