//
// Created by WeiGuannan on 19/01/2017.
//

#include "llvm/Pass.h"
#include "llvm/IR/type.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Casting.h"

#include <set>
#include <map>
#include <vector>
#include <memory>
#include <ostream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

#include "Utils.h"

#ifndef LLVM_AAM_H
#define LLVM_AAM_H

using namespace llvm;

/* TODO
 */

namespace AAM {
  /**
   * `strvar` and `var` will be used in `BindAddr` and `Cont`.
   * the reason there are two variable name representations is in LLVM,
   * some instructions (or variables) are unnamed. We can also use the
   * address of instruction to distinguish them. The name like `%n`
   * is generated when we dump the IR from memory to file.
   * For convenience of testing, we can use string representation,
   * while when analyzing real IR program, we use Value* instead.
   */
  typedef std::string strvar;
  typedef Value* var;
  

  /**
   * Location = HeapAddr
   *          | StackAddr
   *          | FrameAddr
   *          | BAddr
   */
  class Location {
  public:
    enum LocationKind {
      KHeapAddr,
        KConcreteHeapAddr,
        KZeroCFAHeapAddr,
      KHeapAddrEnd,
      
      KStackAddr,
        KConcreteStackAddr,
        KZeroCFAStackAddr,
      KStackAddrEnd,
      
      KBAddr,
        KBindAddr,
      KBAddrEnd
    };
  
    static std::string KindToString(LocationKind v) {
      switch (v) {
        case KHeapAddr: return "HeapAddr";
        case KConcreteHeapAddr: return "ConcreteHeapAddr";
        case KZeroCFAHeapAddr: return "0CFAHeapAddr";
        case KStackAddr: return "StackAddr";
        case KConcreteStackAddr: return "ConcreteStackAddr";
        case KZeroCFAStackAddr: return "0CFAStackAddr";
        case KBAddr: return "BAddr";
        case KBindAddr: return "BindAddr";
        default: return "Unknown";
      }
    }
    
    LocationKind getKind() const {
      return kind;
    }
      
    friend bool operator==(const Location& a, const Location& b) {
      return a.equalTo(b);
    }
    
    friend bool operator!=(const Location& a, const Location& b) {
      return !(a == b);
    }
    
    virtual void print() const {
      assert(false && "should not call Location::print");
    }
    
    virtual size_t hashValue() const {
      assert(false && "should not call Location::hashValue");
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
      // TODO: using memory location?
      // TODO: using equality if hash values collided?
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
      assert(false && "should not call HeapAddr::hashValue");
      return hash_value("HeapAddr");
    }
    
    virtual void print() const override {
      assert(false && "should not call HeapAddr::print");
    }
    
  protected:
    HeapAddr(LocationKind kind) : Location(kind) {}
    
    virtual bool equalTo(const Location& that) const override {
      //errs() << "should not call HeapAddr::equalTo\n";
      assert(false && "should not call HeapAddr::equalTo");
      return false;
    }
  };

  class StackAddr : public Location {
  public:
    static bool classof(const Location* loc) {
      LocationKind k = loc->getKind();
      return k >= KStackAddr && k <= KStackAddrEnd;
    }
    
    virtual bool isInitFp() = 0;
    
    virtual size_t hashValue() const override {
      assert(false && "should not call StackAddr::hashValue");
      return hash_value("StackAddr");
    }
    
    virtual void print() const override {
      assert(false && "should not call StackAddr::print");
    }
    
  protected:
    virtual bool equalTo(const Location& that) const override {
      assert(false && "should not call StackAddr::equalTo");
      return false;
    }
  
    StackAddr(LocationKind kind) : Location(kind) {}
  };

  typedef StackAddr FrameAddr;

  class BAddr : public Location {
  public:
    static bool classof(const Location* loc) {
      LocationKind k = loc->getKind();
      return k >= KBAddr && k <= KBAddrEnd;
    }
  
    virtual size_t hashValue() const override {
      assert(false && "should not call BAddr::hashValue");
      return hash_value("BAddr");
    }
    
    virtual void print() const override {
      assert(false && "should not call BAddr::print");
    }

  protected:
    BAddr(LocationKind kind) : Location(kind) {}
    
    virtual bool equalTo(const Location& that) const override {
      assert(false && "should not call BAddr::equalTo");
      //errs() << "should not call BAddr::equalTo\n";
      return false;
    }
  };

  class BindAddr : public BAddr {
  private:
    strvar strname;
    var name;
    std::shared_ptr<FrameAddr> fp;
    
  public:
    typedef std::shared_ptr<BindAddr> BindAddrPtrType;
    BindAddr(strvar strname, std::shared_ptr<FrameAddr> fp) : BAddr(KBindAddr), strname(strname), name(nullptr), fp(fp) {};
    BindAddr(var name, std::shared_ptr<FrameAddr> fp) : BAddr(KBindAddr), strname(""), name(name), fp(fp) {};
  
    static bool classof(const Location* loc) {
      return loc->getKind() == KBindAddr;
    }
    
    static BindAddrPtrType makeBindAddr(var name, std::shared_ptr<FrameAddr> fp) {
      auto ba = std::make_shared<BindAddr>(name, fp);
      return ba;
    }
    
    virtual void print() const override {
      errs() << "BindAddr[";
      if (strname != "") { errs() << strname; }
      else if (name->hasName()) { errs() << name->getName(); }
      else { name->print(errs(), false); }
      errs() << ",";
      fp->print();
      errs() << "]";
    }
  
    virtual size_t hashValue() const override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(strname));
      seed = hash_combine(seed, hash_value(name));
      seed = hash_combine(seed, fp->hashValue());
      return seed;
    }
    
  protected:
    virtual bool equalTo(const Location& that) const override {
      if (!isa<BindAddr>(&that))
        return false;
      auto* newThat = dyn_cast<BindAddr>(&that);
      return newThat->strname == this->strname &&
             newThat->name == this->name &&
             *newThat->fp == *this->fp;
    }
  };
  
  /**
   * AbstractValue = Cont
   *               | LocationValue
   *               | FuncValue
   *               | PrimValue
   *
   * PrimValue = PrimValue
   *           | IntValue
   *           | BotValue
   */
  class AbstractValue {
  public:
    enum ValKind {
      KContV,
      KLocationV,
      KFuncV,
      KPrimV,
        KIntV,
        KBotV,
      KPrimVEnd
    };
    
    static std::string KindToString(ValKind v) {
      switch (v) {
        case KContV: return "ContV";
        case KLocationV: return "LocationV";
        case KFuncV: return "FuncV";
        case KPrimV: return "PrimV";
        case KIntV: return "IntV";
        case KBotV: return "BotV";
        default: return "Unknown";
      }
    }

    ValKind getKind() const {
      return kind;
    }
    
    friend bool operator==(const AbstractValue& a, const AbstractValue& b) {
      return a.equalTo(b);
    }
    
    friend bool operator!=(const AbstractValue& a, const AbstractValue& b) {
      return !(a == b);
    }
    
    virtual size_t hashValue() {
      assert(false && "should not call AbstractValue::hashValue");
      return hash_value("AbstractValue");
    }
    
    virtual void print() const {
      assert(false && "should not call AbstractValue::print");
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
  
  struct AbstractValueLess {
    bool operator()(const std::shared_ptr<AbstractValue>& a, const std::shared_ptr<AbstractValue>& b) const {
      //errs() << "a hash: " << a->hashValue() << " b hash: " << b->hashValue() << "\n";
      // TODO: using memory location?
      // TODO: using equality if hash values collided?
      return a->hashValue() < b->hashValue();
    }
  };

  class Cont : public AbstractValue {
  private:
    strvar _lhs;
    var lhs;
    Instruction* inst;
    std::shared_ptr<FrameAddr> frameAddr;
    std::shared_ptr<StackAddr> stackAddr;
    
  public:
    typedef std::shared_ptr<Cont> ContPtrType;
    Cont(strvar _lhs, Instruction* inst, std::shared_ptr<FrameAddr> frameAddr, std::shared_ptr<StackAddr> stackAddr)
      : AbstractValue(KContV), _lhs(_lhs), lhs(nullptr), inst(inst), frameAddr(frameAddr), stackAddr(stackAddr) {}
    
    Cont(var lhs, Instruction* inst, std::shared_ptr<FrameAddr> frameAddr, std::shared_ptr<StackAddr> stackAddr)
      : AbstractValue(KContV), _lhs(""), lhs(lhs), inst(inst), frameAddr(frameAddr), stackAddr(stackAddr) {}
    
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KContV;
    }
    
    static ContPtrType makeCont(var lhs, Instruction* inst,
                                std::shared_ptr<FrameAddr> frameAddr,
                                std::shared_ptr<StackAddr> stackAddr) {
      auto cont = std::make_shared<Cont>(lhs, inst, frameAddr, stackAddr);
      return cont;
    }
    
    virtual size_t hashValue() override {
      size_t seed = 0;
      if (!_lhs.empty()) {
        seed = hash_combine(seed, hash_value(_lhs));
      }
      if (!lhs) {
        seed = hash_combine(seed, hash_value(lhs));
      }
      seed = hash_combine(seed, hash_value(inst));
      seed = hash_combine(seed, frameAddr->hashValue());
      seed = hash_combine(seed, stackAddr->hashValue());
      return seed;
    }
    
    virtual void print() const override {
      errs() << "Cont[";
      if (_lhs != "") { errs() << _lhs; }
      else if (lhs->hasName()) { errs() << lhs->getName(); }
      else { lhs->print(errs(), false); }
      errs() << ",";
      inst->print(errs(), false);
      errs() << ",";
      frameAddr->print();
      errs() << ",";
      stackAddr->print();
      errs() << "]";
    }
    
    strvar getStrLhs() { return _lhs; }
    var getLhs() { return lhs; }
    Instruction* getInst() { return inst; }
    std::shared_ptr<FrameAddr> getFrameAddr() { return frameAddr; }
    std::shared_ptr<StackAddr> getStackAddr() { return stackAddr; }

  protected:
    virtual bool equalTo(const AbstractValue& that) const override {
      if (!isa<Cont>(&that))
        return false;
      auto* newThat = dyn_cast<Cont>(&that);
      return this->_lhs == newThat->_lhs &&
             this->lhs == newThat->lhs &&
             this->inst == newThat->inst && // TODO: this->inst==that->inst or *this->inst==*that->inst ?
             *this->frameAddr == *newThat->frameAddr &&
             *this->stackAddr == *newThat->stackAddr;
    }
  };
  
  class LocationValue : public AbstractValue {
    std::shared_ptr<Location> loc;
    size_t step;
  public:
    typedef std::shared_ptr<LocationValue> LocValPtrType;
    
    LocationValue(std::shared_ptr<Location> loc) :
      AbstractValue(KLocationV), loc(loc), step(0) {}
    
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KLocationV;
    }
  
    std::shared_ptr<Location>  getLocation() {
      return loc;
    }
    
    size_t getStep() { return step; }
    
    static LocValPtrType makeLocationValue(std::shared_ptr<Location> loc) {
      std::shared_ptr<LocationValue> locVal = std::make_shared<LocationValue>(loc);
      return locVal;
    }
    
    virtual size_t hashValue() override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value("LocationValue"));
      seed = hash_combine(seed, loc->hashValue());
      return seed;
    }
    
    virtual void print() const override {
      errs() << "LocationValue[";
      loc->print();
      errs() << "]";
      //errs() << "," << step << "]";
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
    typedef std::shared_ptr<FuncValue> FuncValuePtrType;
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
    
    static FuncValuePtrType makeFuncValue(Function* f) {
      auto fv = std::make_shared<FuncValue>(f);
      return fv;
    }
  
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KFuncV;
    }
  
    virtual size_t hashValue() override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(fun));
      return seed;
    }
    
    virtual void print() const override {
      errs() << "FuncValue[";
      errs() << fun->getName();
      errs() << "]";
    }

  protected:
    virtual bool equalTo(const AbstractValue& that) const override {
      if (!isa<FuncValue>(&that))
        return false;
      auto* newThat = dyn_cast<FuncValue>(&that);
      return this->fun == newThat->fun;
    }
  };
  
  // Represent all primitive values
  class PrimValue : public AbstractValue {
  public:
    typedef std::shared_ptr<PrimValue> PrimValuePtrType;
    //TODO: make sure the constructor only called once.
    PrimValue() : AbstractValue(KPrimV) {}
    
    static PrimValuePtrType getInstance() {
      static PrimValuePtrType instance = std::make_shared<PrimValue>();
      return instance;
    }
    static bool classof(const AbstractValue* v) {
      return v->getKind() >= KPrimV &&
             v->getKind() <= KPrimVEnd;
    }
    
    virtual size_t hashValue() override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value("PrimValue"));
      return seed;
    }
    
    virtual void print() const override {
      errs() << "PrimValue";
    }
  
  protected:
    PrimValue(ValKind k) : AbstractValue(k) {}
    
    virtual bool equalTo(const AbstractValue& that) const override {
      return isa<PrimValue>(&that);
    }
  };
  
  /**
   * Assume that the IntValue is the only type in the system.
   */
  class IntValue : public PrimValue {
  private:
    APInt val;
    
  public:
    typedef std::shared_ptr<IntValue> IntValuePtrType;
    IntValue(APInt val) : PrimValue(KIntV), val(val) {}
    
    APInt& getValue() {
      return val;
    }
    
    static bool classof(const AbstractValue* v) {
      return v->getKind() == KIntV;
    }
    
    static IntValuePtrType makeInt(APInt x) {
      auto i = std::make_shared<IntValue>(x);
      return i;
    }
    
    static IntValuePtrType makeInt(int x) {
      return makeInt(APInt(64, x, true));
    }
    
    virtual size_t hashValue() override {
      return hash_value(val);
    }
    
    virtual void print() const override {
      errs() << "IntValue[";
      if (val.getBitWidth() == 1) {
        errs() << val.getBoolValue();
      }
      else {
        errs() << val.getSExtValue();
      }
      errs() << "]";
    }
    
  protected:
    virtual bool equalTo(const AbstractValue& that) const override {
      if (!isa<IntValue>(&that))
        return false;
      auto* newThat = dyn_cast<IntValue>(&that);
      return val.eq(newThat->val);
    }
  };
  
  class BotValue : public PrimValue {
  public:
    typedef std::shared_ptr<BotValue> BotValuePtrType;
    //TODO: make sure the constructor only called once.
    BotValue() : PrimValue(KBotV) {}
  
    static BotValuePtrType getInstance() {
      static BotValuePtrType instance = std::make_shared<BotValue>();
      return instance;
    }
    static bool classof(const AbstractValue* v) {
      return v->getKind() >= KBotV;
    }
  
    virtual size_t hashValue() override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value("BotValue"));
      return seed;
    }
    
    virtual void print() const override {
      errs() << "BotValue";
    }

  protected:
    virtual bool equalTo(const AbstractValue& that) const override {
      return isa<BotValue>(&that);
    }
    
  };
  
  ///////////////////////////////////////////
  
  template<class V>
  struct ReplaceUpdater {
    std::shared_ptr<V> operator()(const std::shared_ptr<V>& oldOne,
                                  const std::shared_ptr<V>& newOne) const {
      return newOne;
    }
  };
  
  template<class K, class V, class Less, class Updater>
  class Store {
  public:
    typedef std::shared_ptr<K> Key;
    typedef std::shared_ptr<V> Val;
    typedef std::map<Key, Val, Less> StoreMap;
    
    Store() {};
    Store(StoreMap m, std::string name = "") : m(m), name(name) {};
    
    size_t size() const {
      return m.size();
    }
    
    std::shared_ptr<Store<K,V,Less,Updater>> copy() {
      auto newMap = m;
      auto newStore = std::make_shared<Store<K,V,Less,Updater>>(newMap);
      return newStore;
    };
    
    Optional<Store<K,V,Less,Updater>::Val> lookup(Store<K,V,Less,Updater>::Key key) {
      auto it = m.find(key);
      if (it != m.end()) return it->second;
      return None;
    }
    
    // Immutable update
    std::shared_ptr<Store<K,V,Less,Updater>> update(Store<K,V,Less,Updater>::Key key, Store<K,V,Less,Updater>::Val val) {
      static Updater updater;
      auto newMap = m;
      newMap[key] = updater(m[key], val);
      std::shared_ptr<Store<K,V,Less,Updater>> newStore = std::make_shared<Store<K,V,Less,Updater>>(newMap);
      return newStore;
    };
    // Immutable remove
    std::shared_ptr<Store<K,V,Less,Updater>> remove(Store<K,V,Less,Updater>::Key key) {
      auto newMap = m;
      newMap.erase(key);
      std::shared_ptr<Store<K,V,Less,Updater>> newStore = std::make_shared<Store<K,V,Less,Updater>>(newMap);
      return newStore;
    };
    
    void inplaceUpdate(Store<K,V,Less,Updater>::Key key, Store<K,V,Less,Updater>::Val val) {
      static Updater updater;
      m[key] = updater(m[key], val);
    }
   
    void inplaceRemove(Store<K,V,Less,Updater>::Key key) {
      m.erase(key);
    };
    
    inline bool operator==(Store<K,V,Less,Updater>& that) {
      auto pred = [] (decltype(*m.begin()) a, decltype(*m.begin()) b) {
        return *a.first == *b.first && *a.second == *b.second;
      };
      return this->m.size() == that.m.size() &&
             std::equal(this->m.begin(), this->m.end(), that.m.begin(), pred);
    }
    
    inline bool operator!=(Store<K,V,Less,Updater>& that) {
      return !(*this == that);
    }
    
    virtual size_t hashValue() {
      size_t seed = 0;
      for (const auto& pair : m) {
        seed = hash_combine(seed, pair.first->hashValue());
        seed = hash_combine(seed, pair.second->hashValue());
      }
      return seed;
    }
    
    void print() {
      errs() << "Store summary[" << size() << "]:\n";
      for (const auto& pair : m) {
        errs() << "\t";
        pair.first->print();
        errs() << " --> ";
        pair.second->print();
        errs() << "\n";
      }
    }
  
  private:
    StoreMap m;
    std::string name;
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
            ((this->measure.hasValue() && that.measure.hasValue() &&
             *this->measure.getValue() == *that.measure.getValue()) ||
              (!this->measure.hasValue() && !that.measure.hasValue()));
    }
    
    inline bool operator!=(Conf<StoreType, SuccType, PredType, MeasureType>& that) {
      return !(*this == that);
    }
    
    virtual size_t hashValue() {
      size_t seed = 0;
      seed = hash_combine(seed, store->hashValue());
      seed = hash_combine(seed, succ->hashValue());
      seed = hash_combine(seed, pred->hashValue());
      if (measure.hasValue()) {
        seed = hash_combine(seed, measure.getValue()->hashValue());
      }
      return seed;
    }
    
  private:
    StorePtrType store;
    SuccPtrType  succ;
    PredPtrType  pred;
    Optional<MeasurePtrType> measure;
  };
  
  class Stmt {
  private:
    Instruction* prev;
    Instruction* inst;
    
  public:
    Stmt(Instruction* inst): prev(nullptr), inst(inst) {}
    Stmt(Instruction* inst, Instruction* prev) : prev(prev), inst(inst) {}
    
    Instruction* getInst() { return inst; }
    
    Instruction* getPrev() { return prev; }
    
    inline bool operator==(Stmt& that) {
      return this->inst == that.inst &&
             this->prev == that.prev;
    }
    
    inline bool operator!=(Stmt& that) {
      return !(*this == that);
    }
    
    static std::shared_ptr<Stmt> makeStmt(Instruction* inst) {
      std::shared_ptr<Stmt> s = std::make_shared<Stmt>(inst);
      return s;
    }
    
    static std::shared_ptr<Stmt> makeStmt(Instruction* inst, Instruction* prev) {
      std::shared_ptr<Stmt> s = std::make_shared<Stmt>(inst, prev);
      return s;
    }
    
    virtual size_t hashValue() {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(inst));
      seed = hash_combine(seed, hash_value(prev));
      return seed;
    }
  };
  
  template<class C, class E, class S, class K>
  class State {
  public:
    typedef std::shared_ptr<C> CPtrType;
    typedef std::shared_ptr<E> EPtrType;
    typedef std::shared_ptr<S> SPtrType;
    typedef std::shared_ptr<K> KPtrType;
    typedef std::shared_ptr<State<C,E,S,K>> StatePtrType;
    
    State(CPtrType c, EPtrType e, SPtrType s, KPtrType k)
      : cPtr(c), ePtr(e), sPtr(s), kPtr(k) {}
    
    CPtrType getControl() { return cPtr; }
    EPtrType getFp() { return ePtr; }
    SPtrType getConf() { return sPtr; }
    KPtrType getSp() { return kPtr; }
    
    // TODO: shared_ptr as covariant return type
    //virtual StatePtrType next() = 0;
    
    inline bool operator==(State<C,E,S,K>& that) {
      return *this->cPtr == *that.cPtr &&
             *this->ePtr == *that.ePtr &&
             *this->sPtr == *that.sPtr &&
             *this->kPtr == *that.kPtr;
    }
    
    inline bool operator!=(State<C,E,S,K>& that) {
      return !(*this == that);
    }
  
    virtual size_t hashValue() const {
      size_t seed = 0;
      seed = hash_combine(seed, cPtr->hashValue());
      seed = hash_combine(seed, ePtr->hashValue());
      seed = hash_combine(seed, sPtr->hashValue());
      seed = hash_combine(seed, kPtr->hashValue());
      return seed;
    }
    
  private:
    CPtrType cPtr;
    EPtrType ePtr;
    SPtrType sPtr;
    KPtrType kPtr;
  };
}

#endif //LLVM_AAM_H
