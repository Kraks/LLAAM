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

/* TODO
 * equality
 * continuation
 */

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
      KHeapAddrEnd,
      
      KStackPtr,
        KConcreteStackPtr,
        KZeroCFAStackPtr,
      KStackPtrEnd,
      
      KBindAddr,
        KLocalBindAddr,
      KBindAddrEnd
    };
  
    static std::string KingToString(LocationKind v) {
      switch (v) {
        case KHeapAddr: return "HeapAddr";
        case KConcreteHeapAddr: return "ConcreteHeapAddr";
        case KZeroCFAHeapAddr: return "0CFAHeapAddr";
        case KStackPtr: return "StackPtr";
        case KConcreteStackPtr: return "ConcreteStackPtr";
        case KZeroCFAStackPtr: return "0CFAStackPtr";
        case KBindAddr: return "BindAddr";
        case KLocalBindAddr: return "LocalBindAddr";
      }
    }
    
    Location() {
      myId = id++;
    };
    
    LocationKind getKind() const {
      return kind;
    }
    
    /* Used for when as Key type of map */
    friend bool operator<(const Location& a, const Location& b) {
      return a.myId < b.myId;
    }
      
    friend bool operator==(const Location& a, const Location& b) {
      return a.equalTo(b);
    }

  protected:
    Location(LocationKind kind) : kind(kind) {}
    virtual bool equalTo(const Location& that) const {
      errs() << "LOC EQ\n";
      return false;
    }
    
  private:
    LocationKind kind;
    unsigned long long myId;
    static unsigned long long id;
  };

  class HeapAddr : public Location {
  public:
    //inline bool operator==(HeapAddr& that) { return false; }
    virtual bool equalTo(const Location& that) const {
      return false;
    }
  
    static bool classof(const Location* loc) {
      LocationKind k = loc->getKind();
      return k >= KHeapAddr && k <=KHeapAddrEnd;
    }
  protected:
    HeapAddr(LocationKind kind) : Location(kind) {}
  };

  class StackPtr : public Location {
  public:
    /*
    friend bool operator==(const StackPtr& a, const StackPtr& b) {
      return a.equalTo(b);
    }
    
    virtual bool equalTo(const Location& that) const {
      if (!isa<StackPtr>(&that)) return false;
      
      errs() << "to stack ptr equal to\n";
      auto* newThat = dyn_cast<StackPtr>(&that);
      return newThat->equalTo(*this);
    }
    */
    
    static bool classof(const Location* loc) {
      LocationKind k = loc->getKind();
      return k >= KStackPtr && k <= KStackPtrEnd;
    }

  protected:
    StackPtr(LocationKind kind) : Location(kind) {}
  };

  typedef StackPtr FramePtr;

  class BindAddr : public Location {
  public:
    virtual bool equalTo(const Location& that) const {
      return false;
    }
    
    static bool classof(const Location* loc) {
      LocationKind k = loc->getKind();
      return k >= KBindAddr && k <= KBindAddrEnd;
    }

  protected:
    BindAddr(LocationKind kind) : Location(kind) {}
  };

  class LocalBindAddr : public BindAddr {
  public:
    LocalBindAddr(var name, FramePtr fp) : BindAddr(KLocalBindAddr), name(name), fp(fp) {};
    
    virtual bool equalTo(const Location& that) const {
      if (!isa<LocalBindAddr>(&that))
        return false;
      auto* newThat = dyn_cast<LocalBindAddr>(&that);
      errs() << "b1: " << (newThat->name == this->name);
      errs() << "b2: " << (newThat->fp == this->fp);
      return newThat->name == this->name &&
             newThat->fp == this->fp;
    }
    
    static bool classof(const Location* loc) {
      return loc->getKind() == KLocalBindAddr;
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
    
    static std::string KindToString(ValKind v) {
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
    //TODO
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
  
  /******** Static initialization ********/
  
  unsigned long long Location::id = 0;
}

namespace ConcreteAAM {
  using namespace AAM;

  class ConcreteHeapAddr : public HeapAddr {
  public:
    ConcreteHeapAddr() : HeapAddr(KConcreteHeapAddr) {
      myId = id++;
    }
    
    static bool classof(const Location* loc) {
      return loc->getKind() == KConcreteHeapAddr;
    }
    
    virtual bool equalTo(const Location& that) const {
      if (!isa<ConcreteHeapAddr>(&that))
        return false;
      auto* newThat = dyn_cast<ConcreteHeapAddr>(&that);
      return newThat->myId == this->myId;
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
  
    static bool classof(const Location* loc) {
      return loc->getKind() == KConcreteStackPtr;
    }
    
    virtual bool equalTo(const Location& that) const {
      if (!isa<ConcreteStackPtr>(&that))
        return false;
      auto* newThat = dyn_cast<ConcreteStackPtr>(&that);
      errs() << "that id: " << newThat->myId << " ; this id: " << this->myId << "\n";
      return newThat->myId == this->myId;
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
    
    /* Immutable update */
    ConcreteStore update(Location& loc, std::shared_ptr<AbstractValue> val) {
      auto newMap = m;
      newMap[loc] = val;
      ConcreteStore newStore(newMap);
      return newStore;
    }
    
    /*
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
