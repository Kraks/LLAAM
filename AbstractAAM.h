//
// Created by WeiGuannan on 08/02/2017.
//
#include "AAM.h"

#ifndef LLVM_ABSTRACTAAM_H
#define LLVM_ABSTRACTAAM_H


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
  
  template<class T, class Less>
  class D {
  public:
    typedef std::shared_ptr<D<T,Less>> DPtr;
    typedef std::shared_ptr<T> ValPtr;
    
    D() {}
    D(std::set<ValPtr, Less> s) : set(s) {}
  
    size_t size() { return set.size(); }
    
    DPtr copy() {
      auto newSet = set;
      auto newD = std::make_shared<D<T,Less>>(newSet);
      return newD;
    }
    
    static DPtr makeMtD() {
      auto d = std::make_shared<D<T,Less>>();
      return d;
    }
    
    void inplaceAdd(ValPtr val) {
      set.insert(val);
    }
    
    void inplaceJoin(DPtr that) {
      inplaceJoin(*that);
    }
    
    void inplaceJoin(D<T,Less>& that) {
      inplaceJoin(that.set);
    }
    
    void inplaceJoin(std::set<ValPtr, Less>& another) {
      for (auto& v : another) {
        set.insert(v);
      }
    }
    
    size_t hashValue() {
      size_t seed = 0;
      for (auto& e : set) {
        seed = hash_combine(seed, e->hashValue());
      }
      return seed;
    }
    
    inline bool operator==(D& that) {
      auto pred = [] (decltype(*set.begin()) a, decltype(*set.begin()) b) {
        return *a == *b;
      };
      return this->size() == that.size() &&
             std::equal(this->set.begin(), this->set.end(), that.set.begin(), that.set.end(), pred);
    }
    
    inline bool operator!=(D& that) {
      return !(*this == that);
    }
    
    virtual void print() const {
      errs() << "{";
      for (auto it = set.begin(); it != set.end(); it++) {
        (*it)->print();
        if (!isLast(it, set)) { errs() << ","; }
      }
      errs() << "}";
    }
    
  private:
    std::set<ValPtr, Less> set;
  };
  
  
  class AbstractNat {
  public:
    enum AbstractNatEnum { Zero, One, Inf };
  
    AbstractNat(AbstractNatEnum e) : e(e) {};
    
    static std::shared_ptr<AbstractNat> getZeroInstance() {
      static auto z = std::make_shared<AbstractNat>(Zero);
      return z;
    }
    
    static std::shared_ptr<AbstractNat> getOneInstance() {
      static auto o = std::make_shared<AbstractNat>(One);
      return o;
    }
    
    static std::shared_ptr<AbstractNat> getInfInstance() {
      static auto i = std::make_shared<AbstractNat>(Inf);
      return i;
    }
    
    size_t hashValue() {
      size_t seed = 0;
      seed = hash_value(e);
      return seed;
    }
    
    virtual void print() const {
      if (e == Zero) {
        errs() << "0";
      }
      else if (e == One) {
        errs() << "1";
      }
      else {
        errs() << "inf";
      }
    }
    
  private:
    AbstractNatEnum e;
  };
  
  typedef D<AbstractValue, AbstractValueLess> AbsD;
  
  typedef D<Location, LocationLess> AbsLoc;
  
  typedef Store<Location, AbsD, LocationLess> AbsStore;
  
  typedef Store<Location, AbsLoc, LocationLess> AbsSucc;
  
  typedef Store<Location, AbsLoc, LocationLess> AbsPred;
  
  typedef Store<Location, AbstractNat, LocationLess> AbsMeasure;
  
  class AbsConf : public Conf<AbsStore, AbsSucc, AbsPred, AbsMeasure> {
  public:
    typedef std::shared_ptr<AbsConf> AbsConfPtr;
    
    AbsConf(StorePtrType store, SuccPtrType succ, PredPtrType pred, MeasurePtrType m) :
      Conf(store, succ, pred, m) {}
    
    static AbsConfPtr makeAbsConf(StorePtrType store, SuccPtrType succ, PredPtrType pred, MeasurePtrType m) {
      auto c = std::make_shared<AbsConf>(store, succ, pred, m);
      return c;
    }
  };
  
  template<typename T> struct StateSetHasher;
  template<typename T> struct StateSetEqual;
  
  template<typename T>
  class StateSet {
  public:
    typedef std::shared_ptr<T> EleType;
    
    StateSet() {}
    
    static std::shared_ptr<StateSet<T>> getMtSet() {
      auto s = std::make_shared<StateSet<T>>();
      return s;
    }
    
    void inplaceInsert(EleType state) {
      set.insert(state);
    }
    
    void inplaceRemove(EleType state) {
      set.erase(state);
    }
    
    EleType inplacePop() {
      auto it = set.begin();
      auto head = *it;
      inplaceRemove(*it);
      return head;
    }
    
    bool contains(EleType state) {
      auto it = set.find(state);
      return it != set.end();
    }
    
    void dump() {
      for (const auto& elem: set) {
        errs() << elem->hashValue() << "\n";
      }
    }
    
    size_t size() { return set.size(); }
  
  private:
    std::unordered_set<EleType, StateSetHasher<T>, StateSetEqual<T>> set;
  };
  
  template<typename T>
  struct StateSetHasher {
    std::size_t operator()(const std::shared_ptr<T>& s) const {
      return s->hashValue();
    }
  };
  
  template<typename T>
  struct StateSetEqual {
    bool operator()(const std::shared_ptr<T>& a, const std::shared_ptr<T>& b) const {
      return *a == *b;
    }
  };
  
  class AbsState;
  
  typedef StateSet<AbsState> AbsStateSet;
  
  class AbsState : public State<Stmt, FrameAddr, AbsConf, StackAddr> {
  private:
    static Module* module;
    static unsigned long long id;
    unsigned long long myId;
  
  public:
    typedef std::shared_ptr<AbsStateSet> AbsStateSetPtrType;
    
    static void setModule(Module* M) {
      module = M;
    }
    static Module* getModule() {
      assert(module != nullptr);
      return module;
    }
    
    AbsStateSetPtrType next() {
      auto s = AbsStateSet::getMtSet();
      return s;
    }
    
    typedef std::shared_ptr<AbsState> StatePtrType;
    
    AbsState(CPtrType c, EPtrType e, SPtrType s, KPtrType k) :
      State(c, e, s, k) {
      myId = id++;
    };
    
    StatePtrType copy() {
      auto s = AbsState::makeState(this->getControl(), this->getFp(), this->getConf(), this->getSp());
      return s;
    }
    
    static StatePtrType makeState(CPtrType c, EPtrType e, SPtrType s, KPtrType k) {
      auto state = std::make_shared<AbsState>(c, e, s, k);
      return state;
    }
  };
}

#endif //LLVM_ABSTRACTAAM_H
