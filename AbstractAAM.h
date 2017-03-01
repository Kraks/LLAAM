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
    static Function* initFunc;
    
    typedef std::shared_ptr<ZeroCFAStackAddr> ZeroCFAStackAddrPtrType;
    
    ZeroCFAStackAddr(Value* val) : val(val), offset(0), StackAddr(KZeroCFAStackAddr) {
      assert(isa<Function>(val));
    }
    
    ZeroCFAStackAddr(Value* val, size_t offset) : val(val), offset(offset), StackAddr(KZeroCFAStackAddr) {
      assert(isa<AllocaInst>(val));
    }
    
    static void setInitFunc(Function* f) {
      assert(f != nullptr);
      initFunc = f;
    }
    
    static ZeroCFAStackAddrPtrType initFp() {
      assert(initFunc != nullptr);
      static auto fp = make(initFunc);
      return fp;
    }
    
    static bool classof(const Location* loc) {
      return loc->getKind() == KZeroCFAStackAddr;
    }
  
    static ZeroCFAStackAddrPtrType make(Function* f) {
      auto s = std::make_shared<ZeroCFAStackAddr>(f);
      return s;
    }
    
    static ZeroCFAStackAddrPtrType make(AllocaInst* inst, size_t offset) {
      auto s = std::make_shared<ZeroCFAStackAddr>(inst, offset);
      return s;
    }
    
    static std::shared_ptr<std::vector<ZeroCFAStackAddrPtrType>> allocate(AllocaInst* inst, size_t n) {
      std::shared_ptr<std::vector<ZeroCFAStackAddrPtrType>> v = std::make_shared<std::vector<ZeroCFAStackAddrPtrType>>();
      for (size_t i = 0; i < n; i++) {
        ZeroCFAStackAddrPtrType a = make(inst, i);
        v->push_back(a);
      }
      return v;
    }
    
    virtual bool isInitFp() override {
      if (!isa<Function>(val))
        return false;
      auto* f = dyn_cast<Function>(val);
      return f == initFunc;
    }
    
    virtual bool equalTo(const Location& that) const override {
      if (!isa<ZeroCFAStackAddr>(&that))
        return false;
      auto* newThat = dyn_cast<ZeroCFAStackAddr>(&that);
      return this->val == newThat->val &&
             this->offset == newThat->offset;
    }
  
    virtual size_t hashValue() const override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(val));
      seed = hash_combine(seed, hash_value(offset));
      seed = hash_combine(seed, hash_value("ZeroCFAStackAddr"));
      return seed;
    }
  
    virtual void print() const override {
      errs() << "0CFAStackAddr[";
      if (isa<Function>(val)) {
        errs() << dyn_cast<Function>(val)->getName();
      }
      else {
        val->print(errs());
      }
      errs() << "," << offset << "]";
    }

  private:
    size_t offset;
    Value* val;
  };
  
  typedef ZeroCFAStackAddr ZeroCFAFrameAddr;
  
  class ZeroCFAHeapAddr : public HeapAddr {
  public:
    typedef std::shared_ptr<ZeroCFAHeapAddr> ZeroCFAHeapAddrPtrType;
    
    ZeroCFAHeapAddr(Instruction* inst, size_t offset) : inst(inst), offset(offset), HeapAddr(KZeroCFAHeapAddr) {
      assert(isa<CallInst>(inst));
    }
    
    static ZeroCFAHeapAddrPtrType make(Instruction* inst, size_t offset) {
      auto a = std::make_shared<ZeroCFAHeapAddr>(inst, offset);
      return a;
    }
    
    static std::shared_ptr<std::vector<ZeroCFAHeapAddrPtrType>> allocate(Instruction* inst, size_t n) {
      std::shared_ptr<std::vector<ZeroCFAHeapAddrPtrType>> v = std::make_shared<std::vector<ZeroCFAHeapAddrPtrType>>();
      for (size_t i = 0; i < n; i++) {
        auto a = make(inst, i);
        v->push_back(a);
      }
      return v;
    }
    
    static bool classof(const Location* loc) {
      return loc->getKind() == KZeroCFAHeapAddr;
    }
  
    virtual bool equalTo(const Location& that) const override {
      if (!isa<ZeroCFAHeapAddr>(&that))
        return false;
      auto* newThat = dyn_cast<ZeroCFAHeapAddr>(&that);
      return this->inst == newThat->inst &&
             this->offset == newThat->offset;
    }
    
    virtual size_t hashValue() const override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(inst));
      seed = hash_combine(seed, hash_value(offset));
      seed = hash_combine(seed, hash_value("ZeroCFAHeapAddr"));
      return seed;
    }
  
    virtual void print() const override {
      errs() << "0CFAHeapAddr[";
      inst->print(errs());
      errs() << "," << offset << "]";
    }

  private:
    Instruction* inst;
    size_t offset;
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
    
    template<class A>
    bool verify() {
      for (auto it = set.begin(); it != set.end(); it++) {
        if (!isa<A>(**it)) return false;
      }
      return true;
    }
    
    static DPtr makeMtD() {
      auto d = std::make_shared<D<T,Less>>();
      return d;
    }
    
    static DPtr makeD(ValPtr v) {
      auto d = makeMtD();
      d->inplaceAdd(v);
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
    
    std::set<ValPtr, Less>& getValueSet() {
      return set;
    };
    
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
    typedef std::shared_ptr<AbstractNat> AbstractNatPtrType;
  
    AbstractNat(AbstractNatEnum e) : e(e) {};
    
    static AbstractNatPtrType getZeroInstance() {
      static auto z = std::make_shared<AbstractNat>(Zero);
      return z;
    }
    
    static AbstractNatPtrType getOneInstance() {
      static auto o = std::make_shared<AbstractNat>(One);
      return o;
    }
    
    static AbstractNatPtrType getInfInstance() {
      static auto i = std::make_shared<AbstractNat>(Inf);
      return i;
    }
  
    inline bool operator==(AbstractNat& that) {
      return this->e == that.e;
    }
    
    static AbstractNatEnum plus(AbstractNatEnum a, AbstractNatEnum b) {
      if (a == Zero) {
        return b;
      }
      if (a == One) {
        if (b == Zero) {
          return One;
        }
        return Inf;
      }
      if (a == Inf) {
        return Inf;
      }
    }
    
    static AbstractNatPtrType plus(AbstractNatPtrType a, AbstractNatPtrType b) {
      AbstractNatEnum res = plus(a->e, b->e);
      return std::make_shared<AbstractNat>(res);
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
  
  //TODO: support update strategy
  typedef Store<Location, AbsD, LocationLess, ReplaceUpdater<AbsD>> AbsStore;
  
  typedef Store<Location, AbsLoc, LocationLess, ReplaceUpdater<AbsLoc>> AbsSucc;
  
  typedef Store<Location, AbsLoc, LocationLess, ReplaceUpdater<AbsLoc>> AbsPred;
  
  class AbsMeasure : public Store<Location, AbstractNat, LocationLess, ReplaceUpdater<AbstractNat>> {
    
  };
  
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
  
  std::shared_ptr<AbsStore> getInitStore(Module& M);
  std::shared_ptr<AbsConf> getInitConf(Module& M);
  
  std::shared_ptr<AbsD> evalAtom(Value* val,
                                 std::shared_ptr<FrameAddr> fp,
                                 std::shared_ptr<AbsConf> conf,
                                 Module& M);
  
  class AbsState;
  
  typedef PSet<AbsState> StateSet;
  
  class AbsState : public State<Stmt, FrameAddr, AbsConf, StackAddr> {
  private:
    static Module* module;
    static unsigned long long id;
    unsigned long long myId;
  
  public:
    typedef std::shared_ptr<AbsState> StatePtrType;
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
        ReturnInst* returnInst = dyn_cast<ReturnInst>(inst);
        auto fp = this->getFp();
        
        if (fp->isInitFp()) {
          // Return in main method
          if (opNum > 0) {
            auto ret = returnInst->getOperand(0);
            auto rval = evalAtom(ret, this->getFp(), this->getConf(), *AbsState::getModule());
            errs() << "Return: ";
            rval->print();
            errs() << "\n";
          }
          else {
            errs() << "Return: Void\n";
          }
          states->inplaceInsert(this->copy());
        }
        else {
          // Return to other functions
          auto contsOpt = this->getConf()->getStore()->lookup(this->getFp());
          assert(contsOpt.hasValue());
          auto conts = contsOpt.getValue();
          assert(conts->template verify<Cont>());
          auto& contVals = conts->getValueSet();
          
          auto newStore = this->getConf()->getStore()->copy();
          
          if (opNum > 0) {
            auto ret = returnInst->getOperand(0);
            auto retVals = evalAtom(ret, getFp(), getConf(), *AbsState::getModule());
            for (auto it = contVals.begin(); it != contVals.end(); it++) {
              auto cont = dyn_cast<Cont>(&**it);
              auto lhs = cont->getLhs();
              auto destAddr = BindAddr::makeBindAddr(lhs, cont->getFrameAddr());
            }
          }
        }
      }
      else if (isa<CallInst>(inst)) {
        CallInst* callInst = dyn_cast<CallInst>(inst);
        Value* f = callInst->getCalledFunction();
        Function* function = nullptr;
        
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
    
    static StatePtrType inject(Module& M, std::string mainFuncName) {
      std::shared_ptr<AbsConf> initConf = getInitConf(M);
      Function* main = M.getFunction(mainFuncName);
      Instruction* entry = getEntry(*main);
      std::shared_ptr<Stmt> initStmt = std::make_shared<Stmt>(entry);
      
      ZeroCFAStackAddr::setInitFunc(main);
      auto initFp = ZeroCFAStackAddr::initFp();
      
      std::shared_ptr<AbsState> initState = makeState(initStmt, initFp, initConf, initFp);
      return initState;
    }
  };
}

#endif //LLVM_ABSTRACTAAM_H
