//
// Created by WeiGuannan on 08/02/2017.
//
#include "AAM.h"

#ifndef LLVM_ABSTRACTAAM_H
#define LLVM_ABSTRACTAAM_H

// TODO: addrsOf, evalAtom

namespace AbstractAAM {
  using namespace AAM;
  
  class ZeroCFAStackAddr : public StackAddr {
  public:
    typedef std::shared_ptr<ZeroCFAStackAddr> ZeroCFAStackAddrPtrType;
    
    ZeroCFAStackAddr() : StackAddr(KZeroCFAStackAddr) {}
    
    static bool classof(const Location* loc) {
      return loc->getKind() == KZeroCFAStackAddr;
    }
    
    virtual bool equalTo(const Location& that) const override {
      if (!isa<ZeroCFAStackAddr>(&that))
        return false;
      auto* newThat = dyn_cast<ZeroCFAStackAddr>(&that);
      //TODO
      return false;
    }
  
    virtual size_t hashValue() const override {
      size_t seed = 0;
      //TODO
      seed = hash_combine(seed, hash_value("ZeroCFAStackAddr"));
      return seed;
    }
  
    virtual void print() const override {
      errs() << "0CFAStackAddr[" << "TODO" << "]";
    }

  private:
    //TODO
  };
  
  typedef ZeroCFAStackAddr ZeroCFAFrameAddr;
  
  class ZeroCFAHeapAddr : public HeapAddr {
  public:
    typedef std::shared_ptr<ZeroCFAHeapAddr> ZeroCFAHeapAddrPtrType;
    
    ZeroCFAHeapAddr() : HeapAddr(KZeroCFAHeapAddr) {}
    
    static bool classof(const Location* loc) {
      return loc->getKind() == KZeroCFAHeapAddr;
    }
  
    virtual bool equalTo(const Location& that) const override {
      if (!isa<ZeroCFAHeapAddr>(&that))
        return false;
      auto* newThat = dyn_cast<ZeroCFAHeapAddr>(&that);
      //TODO
      return false;
    }
    
    virtual size_t hashValue() const override {
      size_t seed = 0;
      //TODO
      seed = hash_combine(seed, hash_value("ZeroCFAHeapAddr"));
      return seed;
    }
  
    virtual void print() const override {
      errs() << "0CFAHeapAddr[" << "TODO" << "]";
    }

  private:
    //TODO
  };
  
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
  
  template<typename T> struct PSetHasher;
  template<typename T> struct PSetEqual;
  
  template<typename T>
  class PSet {
  public:
    typedef std::shared_ptr<T> EleType;
    
    PSet() {}
    
    static std::shared_ptr<PSet<T>> getMtSet() {
      auto s = std::make_shared<PSet<T>>();
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
    std::unordered_set<EleType, PSetHasher<T>, PSetEqual<T>> set;
  };
  
  template<typename T>
  struct PSetHasher {
    std::size_t operator()(const std::shared_ptr<T>& s) const {
      return s->hashValue();
    }
  };
  
  template<typename T>
  struct PSetEqual {
    bool operator()(const std::shared_ptr<T>& a, const std::shared_ptr<T>& b) const {
      return *a == *b;
    }
  };
  
  class AbsState;
  
  typedef std::shared_ptr<AbsState> StatePtrType;
  typedef PSet<AbsState> StateSet;
  
  class AbsState : public State<Stmt, FrameAddr, AbsConf, StackAddr> {
  private:
    static Module* module;
    static unsigned long long id;
    unsigned long long myId;
  
  public:
    typedef std::shared_ptr<StateSet> StateSetPtrType;
    
    static void setModule(Module* M) {
      module = M;
    }
    static Module* getModule() {
      assert(module != nullptr);
      return module;
    }
    
    StateSetPtrType next() {
      auto states = StateSet::getMtSet();
      LLVMContext& C = getModule()->getContext();
      Instruction* inst = getControl()->getInst();
      Instruction* nextInst = getSyntacticNextInst(inst);
      auto nextStmt = Stmt::makeStmt(nextInst);
  
      size_t opNum = inst->getNumOperands();
      errs() << "op num: " << opNum << "\n";
      
      if (isa<ReturnInst>(inst)) {
        
      }
      else if (isa<CallInst>(inst)) {
        
      }
      else if (isa<LoadInst>(inst)) {
      
      }
      else if (isa<StoreInst>(inst)) {
      
      }
      else if (isa<AllocaInst>(inst)) {
      
      }
      else if (isa<GetElementPtrInst>(inst)) {
        
      }
      else if (Instruction::Add == inst->getOpcode() ||
               Instruction::Sub == inst->getOpcode() ||
               Instruction::Mul == inst->getOpcode()) {
        
      }
      else if (isa<ICmpInst>(inst)) {
      
      }
      else if (isa<BranchInst>(inst)) {
        
      }
      else if (isa<BitCastInst>(inst)) {
        
      }
      else if (isa<SExtInst>(inst)) {
        
      }
      else if (isa<ZExtInst>(inst)) {
        
      }
      else if (isa<TruncInst>(inst)) {
        
      }
      else {
        assert(false && "Unsupported instruction");
      }
      
      return states;
    }
    
    void print() {
      errs() << "state[";
      getControl()->getInst()->print(errs());
      errs() << "][" << myId << "]";
    }
    
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
