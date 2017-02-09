//
// Created by WeiGuannan on 19/01/2017.
//

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Casting.h"

#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <memory>

#include "Utils.h"

#ifndef LLVM_AAM_H
#define LLVM_AAM_H

using namespace llvm;

/* TODO
 * test locationvalue
 * copy constructor for abs location
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
  
    static std::string KindToString(LocationKind v) {
      switch (v) {
        case KHeapAddr: return "HeapAddr";
        case KConcreteHeapAddr: return "ConcreteHeapAddr";
        case KZeroCFAHeapAddr: return "0CFAHeapAddr";
        case KStackPtr: return "StackPtr";
        case KConcreteStackPtr: return "ConcreteStackPtr";
        case KZeroCFAStackPtr: return "0CFAStackPtr";
        case KBindAddr: return "BindAddr";
        case KLocalBindAddr: return "LocalBindAddr";
        default: return "Unknown";
      }
    }
    
    LocationKind getKind() const {
      return kind;
    }
      
    friend bool operator==(const Location& a, const Location& b) {
      return a.equalTo(b);
    }
  
    virtual size_t hashValue() const {
      return hash_value("Location");
    }
    
  protected:
    Location(LocationKind kind) : kind(kind) {}
    
    virtual bool equalTo(const Location& that) const {
      assert(false && "should not call Location::equalTo");
      return false;
    }
    
  private:
    LocationKind kind;
    Location() {};
  };
  
  struct LocationHasher {
    std::size_t operator()(const std::shared_ptr<Location>& loc) const {
      Location* lp = loc.get();
      return lp->hashValue();
    }
  };
  
  struct LocationLess {
    bool operator()(const std::shared_ptr<Location>& a, const std::shared_ptr<Location>& b) const {
      //errs() << "a hash: " << a->hashValue() << " b hash: " << b->hashValue() << "\n";
      return a->hashValue() < b->hashValue();
    }
  };
  
  struct LocationEqual {
    bool operator()(const std::shared_ptr<Location>& a, const std::shared_ptr<Location>& b) const {
      return *a.get() == *b.get();
    }
  };

  class HeapAddr : public Location {
  public:
    static bool classof(const Location* loc) {
      LocationKind k = loc->getKind();
      return k >= KHeapAddr && k <=KHeapAddrEnd;
    }
  
    virtual size_t hashValue() const override {
      return hash_value("HeapAddr");
    }
    
  protected:
    HeapAddr(LocationKind kind) : Location(kind) {}
    
    virtual bool equalTo(const Location& that) const override {
      //errs() << "should not call HeapAddr::equalTo\n";
      assert(false && "should not call HeapAddr::equalTo");
      return false;
    }
  };

  class StackPtr : public Location {
  public:
    static bool classof(const Location* loc) {
      LocationKind k = loc->getKind();
      return k >= KStackPtr && k <= KStackPtrEnd;
    }
    
    virtual size_t hashValue() const override {
      return hash_value("StackPtr");
    }
    
  protected:
    virtual bool equalTo(const Location& that) const override {
      //errs() << "should not call StackPtr::equalTo\n";
      assert(false && "should not call StackPtr::equalTo");
      return false;
    }
  
    StackPtr(LocationKind kind) : Location(kind) {}
  };

  typedef StackPtr FramePtr;

  class BindAddr : public Location {
  public:
    static bool classof(const Location* loc) {
      LocationKind k = loc->getKind();
      return k >= KBindAddr && k <= KBindAddrEnd;
    }
  
    virtual size_t hashValue() const override {
      return hash_value("BindAddr");
    }

  protected:
    BindAddr(LocationKind kind) : Location(kind) {}
    
    virtual bool equalTo(const Location& that) const override {
      errs() << "should not call BindAddr::equalTo\n";
      //assert(false && "should not call BindAddr::equalTo");
      return false;
    }
  };

  class LocalBindAddr : public BindAddr {
  private:
    var name;
    std::shared_ptr<FramePtr> fp;
    
  public:
    LocalBindAddr(var name, std::shared_ptr<FramePtr> fp) : BindAddr(KLocalBindAddr), name(name), fp(fp) {};
  
    static bool classof(const Location* loc) {
      return loc->getKind() == KLocalBindAddr;
    }
  
    virtual size_t hashValue() const override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(name));
      seed = hash_combine(seed, fp->hashValue());
      return seed;
    }
    
  protected:
    virtual bool equalTo(const Location& that) const override {
      if (!isa<LocalBindAddr>(&that))
        return false;
      auto* newThat = dyn_cast<LocalBindAddr>(&that);
      return newThat->name == this->name &&
             *newThat->fp == *this->fp;
    }
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
        default: return "Unknown";
      }
    }
    
    ValKind getKind() const {
      return kind;
    }
    
    friend bool operator==(const AbstractValue& a, const AbstractValue& b) {
      return a.equalTo(b);
    }
    
  private:
    ValKind kind;
    AbstractValue() {}
    
  protected:
    AbstractValue(ValKind kind): kind(kind) {}
    virtual bool equalTo(const AbstractValue& that) const {
      assert(false && "should not call AbstractValue::equalTo");
      return false;
    }
  };

  class Cont : public AbstractValue {
  private:
    var lhs;
    Instruction* inst;
    std::shared_ptr<FramePtr> framePtr;
    std::shared_ptr<StackPtr> stackPtr;
    
  public:
    Cont(var lhs, Instruction* inst, std::shared_ptr<FramePtr> framePtr, std::shared_ptr<StackPtr> stackPtr)
      : AbstractValue(KContV), lhs(lhs), inst(inst), framePtr(framePtr), stackPtr(stackPtr) {}
    
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KContV;
    }
    
    var getLhs() { return lhs; }
    Instruction* getInst() { return inst; }
    std::shared_ptr<FramePtr> getFramePtr() { return framePtr; }
    std::shared_ptr<StackPtr> getStackPtr() { return stackPtr; }

  protected:
    virtual bool equalTo(const AbstractValue& that) const override {
      if (!isa<Cont>(&that))
        return false;
      auto* newThat = dyn_cast<Cont>(&that);
      return this->lhs == newThat->lhs &&
             this->inst == newThat->inst && // TODO: this->inst==that->inst or *this->inst==*that->inst ?
             *this->framePtr == *newThat->framePtr &&
             *this->stackPtr == *newThat->stackPtr;
    }
  };

  class LocationValue : public AbstractValue {
    std::shared_ptr<Location> loc;
  public:
    LocationValue(std::shared_ptr<Location> loc) : AbstractValue(KLocationV), loc(loc) {}
    
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KLocationV;
    }
  
    std::shared_ptr<Location>  getLocation() {
      return loc;
    }

  protected:
    virtual bool equalTo(const AbstractValue& that) const override {
      if (!isa<LocationValue>(&that))
        return false;
      auto* newThat = dyn_cast<LocationValue>(&that);
      return *this->loc == *newThat->loc;
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
    
    Function* getFunction() const {
      return fun;
    }
  
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KFuncV;
    }

  protected:
    virtual bool equalTo(const AbstractValue& that) const override {
      if (!isa<FuncValue>(&that))
        return false;
      auto* newThat = dyn_cast<FuncValue>(&that);
      return this->fun == newThat->fun;
    }
  };

  class PrimValue : public AbstractValue {
  public:
    PrimValue() : AbstractValue(KPrimV) {}
    
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KPrimV;
    }

  protected:
    virtual bool equalTo(const AbstractValue& that) const override {
      return isa<PrimValue>(&that);
    }
  };

  enum AbstractNat { Zero, One, Inf };
  
  template<class K, class V, class Less>
  class Store {
  public:
    typedef std::shared_ptr<K> Key;
    typedef std::shared_ptr<V> Val;
    typedef std::map<Key, Val, Less> StoreMap;
    
    Store() {};
    Store(StoreMap m) : m(m) {};
    
    size_t size() const {
      return m.size();
    }
    
    Optional<Store<K,V,Less>::Val> lookup(Store<K,V,Less>::Key key) {
      auto it = m.find(key);
      if (it != m.end()) return it->second;
      return None;
    }
    
    // Immutable update
    Store<K,V,Less> update(Store<K,V,Less>::Key key, Store<K,V,Less>::Val val) {
      auto newMap = m;
      newMap[key] = val;
      Store<K,V,Less> newStore(newMap);
      return newStore;
    }
    
    // Immutable remove
    Store<K,V,Less> remove(Store<K,V,Less>::Key key) {
      auto newMap = m;
      newMap.erase(key);
      Store<K,V,Less> newStore(newMap);
      return newStore;
    };
    
    inline bool operator==(Store<K,V,Less>& that) {
      auto pred = [] (decltype(*m.begin()) a, decltype(*m.begin()) b) {
        return *a.first == *b.first && *a.second == *b.second;
      };
      return this->m.size() == that.m.size() &&
             std::equal(this->m.begin(), this->m.end(), that.m.begin(), pred);
    }
  
  private:
    StoreMap m;
  };
  
  template<class StoreType, class SuccType, class PredType, class MeasureType>
  class Conf {
  public:
    typedef std::shared_ptr<StoreType> StorePtrType;
    typedef std::shared_ptr<SuccType>  SuccPtrType;
    typedef std::shared_ptr<PredType>  PredPtrType;
    typedef std::shared_ptr<MeasureType> MeasurePtrType;
    
    Conf(StorePtrType store, SuccPtrType succ, PredPtrType pred)
      : store(store), succ(succ), pred(pred), measure(None) {}
    
    Conf(StorePtrType store, SuccPtrType succ, PredPtrType pred, MeasurePtrType measure)
      : store(store), succ(succ), pred(pred), measure(measure) {}
    
    StorePtrType getStore() { return store; }
    SuccPtrType getSucc() { return succ; }
    PredPtrType getPred() { return pred; }
    MeasurePtrType getMeasure() {
      assert(measure.hasValue());
      return measure.getValue();
    }
    
    inline bool operator==(Conf<StoreType, SuccType, PredType, MeasureType>& that) {
      return *this->store == *that.store &&
             *this->succ == *that.succ &&
             *this->pred == *that.pred &&
             *this->measure.getValue() == *that.measure.getValue();
    }
    
  private:
    StorePtrType store;
    SuccPtrType  succ;
    PredPtrType  pred;
    Optional<MeasurePtrType> measure;
  };
  
  class Measure {};
  
  struct State {};
  
}

#endif //LLVM_AAM_H
