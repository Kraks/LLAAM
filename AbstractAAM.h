//
// Created by WeiGuannan on 08/02/2017.
//
#include "AAM.h"

#ifndef LLVM_ABSTRACTAAM_H
#define LLVM_ABSTRACTAAM_H

// TODO: addrsOf

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
    
    static ZeroCFAStackAddrPtrType initFp(Module* M) {
      if (initFunc == nullptr) {
        initFunc = M->getFunction("main");
      }
      return initFp();
    }
    
    static ZeroCFAStackAddrPtrType initFp() {
      return make(initFunc);
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
    
    ZeroCFAHeapAddr(Value* inst, size_t offset) : val(inst), offset(offset), HeapAddr(KZeroCFAHeapAddr) {
      assert(inst != nullptr);
      assert(isa<CallInst>(inst));
    }
    
    static ZeroCFAHeapAddrPtrType make(Value* inst, size_t offset) {
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
      return this->val == newThat->val &&
             this->offset == newThat->offset;
    }
    
    virtual size_t hashValue() const override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(val));
      seed = hash_combine(seed, hash_value(offset));
      seed = hash_combine(seed, hash_value("ZeroCFAHeapAddr"));
      return seed;
    }
  
    virtual void print() const override {
      errs() << "0CFAHeapAddr[";
      val->print(errs());
      errs() << "," << offset << "]";
    }

  private:
    Value* val;
    size_t offset;
  };
  
  class AbstractNat {
  public:
    enum AbstractNatEnum { Zero = 0, One = 1, Inf = 2 };
    typedef std::shared_ptr<AbstractNat> AbstractNatPtrType;
    
    AbstractNat(AbstractNatEnum e) : e(e) {};
    
    AbstractNatEnum getVal() { return e; }
    
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
    
    inline bool operator<=(AbstractNat& that) {
      auto thisE = this->e;
      auto thatE = that.e;
      return thisE <= thatE;
    }
    
    AbstractNatPtrType plus(AbstractNatPtrType x) {
      AbstractNatEnum res = AbstractNat::plus(this->e, x->e);
      return std::make_shared<AbstractNat>(res);
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
    
    std::shared_ptr<AbstractNat> getMeasure() {
      if (set.size() == 0)
        return AbstractNat::getZeroInstance();
      if (set.size() == 1)
        return AbstractNat::getOneInstance();
      return AbstractNat::getInfInstance();
    }
    
    template<class A>
    bool verify() {
      for (auto it = set.begin(); it != set.end(); it++) {
        if (!isa<A>(**it)) return false;
      }
      return true;
    }
    
    template<class A>
    bool has() {
      for (auto it = set.begin(); it != set.end(); it++) {
        if (isa<A>(**it)) return true;
      }
      return false;
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
    
    // TODO: TEST
    void inplaceRemove(ValPtr val) {
      set.erase(val);
    }
    
    void inplaceClear() {
      set.clear();
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
  
  typedef D<AbstractValue, AbstractValueLess> AbsD;
  
  typedef D<Location, LocationLess> AbsLoc;
  
  template<class V>
  struct JoinUpdater {
    std::shared_ptr<V> operator()(const std::shared_ptr<V>& oldOne,
                                  const std::shared_ptr<V>& newOne) const {
      auto newD = newOne->copy();
      newD->inplaceJoin(oldOne);
      return newD;
    }
  };
  
  template<class V>
  struct PlusUpdater {
    std::shared_ptr<V> operator()(const std::shared_ptr<V>& oldOne,
                                  const std::shared_ptr<V>& newOne) const {
      return oldOne->plus(newOne);
    }
  };
  
  typedef Store<Location, AbsD, LocationLess, JoinUpdater<AbsD>> AbsStore;
  
  typedef Store<Location, AbsLoc, LocationLess, JoinUpdater<AbsLoc>> AbsSucc;
  
  typedef Store<Location, AbsLoc, LocationLess, JoinUpdater<AbsLoc>> AbsPred;
  
  typedef Store<Location, AbstractNat, LocationLess, PlusUpdater<AbstractNat>> AbsMeasure;
  
  class AbsConf : public Conf<AbsStore, AbsSucc, AbsPred, AbsMeasure> {
  public:
    typedef std::shared_ptr<AbsConf> AbsConfPtr;
    
    AbsConf(StorePtrType store, SuccPtrType succ, PredPtrType pred, MeasurePtrType m) :
      Conf(store, succ, pred, m) {}
    
    static AbsConfPtr makeAbsConf(StorePtrType store, SuccPtrType succ, PredPtrType pred, MeasurePtrType m) {
      auto c = std::make_shared<AbsConf>(store, succ, pred, m);
      return c;
    }
    
    AbsConfPtr copy() {
      auto conf = makeAbsConf(getStore()->copy(),
                              getSucc()->copy(),
                              getPred()->copy(),
                              getMeasure()->copy());
      return conf;
    }
    
    void inplaceRemove(AbsStore::Key key, RemoveOption opt = RM_ALL) {
      if (opt & RM_STORE) {
        errs() << "Remove ";
        key->print();
        errs() << " from store\n";
        this->getStore()->inplaceRemove(key);
      }
      if (opt & RM_SUCC) {
        errs() << "Remove ";
        key->print();
        errs() << " from succ\n";
        this->getSucc()->inplaceRemove(key);
      }
      if (opt & RM_PRED) {
        errs() << "Remove ";
        key->print();
        errs() << " from pred\n";
        this->getPred()->inplaceRemove(key);
      }
      if (opt & RM_MEASURE) {
        errs() << "Remove ";
        key->print();
        errs() << " from measure\n";
        this->getMeasure()->inplaceRemove(key);
      }
    }
    
    void inplaceRemoveWhen(AbsStore::Key key, std::function<bool()> pred, RemoveOption opt = RM_ALL) {
      if (pred()) {
        inplaceRemove(key, opt);
      }
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
    
    std::unordered_set<EleType, PSetHasher<T>, PSetEqual<T>>& getValueSet() {
      return set;
    };
    
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
    
    bool nonEmpty() {
      return size() != 0;
    }
  
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
  std::shared_ptr<AbsMeasure> getInitMeasure(Module& M);
  std::shared_ptr<AbsConf> getInitConf(Module& M);
  
  std::shared_ptr<AbsD> evalAtom(Value* val,
                                 std::shared_ptr<FrameAddr> fp,
                                 std::shared_ptr<AbsConf> conf,
                                 Module& M);
  
  std::shared_ptr<AbsLoc> addrsOf(Value* lhs,
                                  std::shared_ptr<FrameAddr> fp,
                                  std::shared_ptr<AbsConf> conf,
                                  Module& M);
  
  std::shared_ptr<AbsD> primOp(unsigned op,
                               std::shared_ptr<AbsD> lhs,
                               std::shared_ptr<AbsD> rhs);
  
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
  
      auto one = AbstractNat::getOneInstance();
      
      size_t opNum = inst->getNumOperands();
      errs() << "op num: " << opNum << "\n";
      
      if (isa<ReturnInst>(inst)) {
        /**
         * For a return instruction,
         */
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
          //states->inplaceInsert(this->copy());
        }
        else {
          // Return to other functions
          auto contsOpt = this->getConf()->getStore()->lookup(this->getFp());
          assert(contsOpt.hasValue());
          auto conts = contsOpt.getValue();
          assert(conts->template verify<Cont>());
          auto& contVals = conts->getValueSet();
          
          auto newStore = this->getConf()->getStore()->copy();
          auto newMeasure = this->getConf()->getMeasure()->copy();
          
          if (opNum > 0) {
            auto ret = returnInst->getOperand(0);
            auto retVals = evalAtom(ret, getFp(), getConf(), *AbsState::getModule());
            for (auto it = contVals.begin(); it != contVals.end(); it++) {
              auto cont = dyn_cast<Cont>(&**it);
              auto lhs = cont->getLhs();
              auto destAddr = BindAddr::makeBindAddr(lhs, cont->getFrameAddr());
              newStore->inplaceStrongUpdateWhen(destAddr, retVals, [&]() {
                auto m = getConf()->getMeasure()->lookup(destAddr);
                if (!m.hasValue() || *m.getValue() <= *one) {
                  newMeasure->inplaceStrongUpdate(destAddr, one);
                  return true;
                }
                newMeasure->inplaceUpdate(destAddr, one);
                return false;
              });
            }
          }
          
          auto newConf = AbsConf::makeAbsConf(newStore,
                                              getConf()->getSucc(),
                                              getConf()->getPred(),
                                              newMeasure);
          
          newConf->inplaceRemoveWhen(getFp(), [&]() {
            auto m = getConf()->getMeasure()->lookup(getFp());
            return (!m.hasValue() || *m.getValue() <= *one);
          });
          newConf->inplaceRemoveWhen(getSp(), [&]() {
            auto m = getConf()->getMeasure()->lookup(getSp());
            return (!m.hasValue() || *m.getValue() <= *one);
          });
          
          for (auto it = contVals.begin(); it != contVals.end(); it++) {
            auto cont = dyn_cast<Cont>(&**it);
            auto inst = cont->getInst();
            auto calleeStmt = Stmt::makeStmt(inst);
            auto newState = AbsState::makeState(calleeStmt,
                                                cont->getFrameAddr(),
                                                newConf,
                                                cont->getStackAddr());
            states->inplaceInsert(newState);
          }
        }
      }
      else if (isa<CallInst>(inst)) {
        CallInst* callInst = dyn_cast<CallInst>(inst);
        Value* f = callInst->getCalledFunction();
        Function* function = nullptr;
        
        if (f) {
          // Call a function with literal name
          function = dyn_cast<Function>(f);
        }
        else {
          // Call a function with some variable
          // Note: Called function is the last operand
          size_t fpos = callInst->getNumOperands() - 1;
          Value* fop = callInst->getOperand(fpos);
          auto faddr = BindAddr::makeBindAddr(fop, getFp());
          auto valOpt = getConf()->getStore()->lookup(faddr);
          assert(valOpt.hasValue());
          auto val = valOpt.getValue();
          auto& valSet = val->getValueSet();
          // Note: Since the setting is first-order function, so we
          // assert the size of value is 1.
          assert(valSet.size() == 1 && "multiple functions");
          auto fval = dyn_cast<FuncValue>(&**valSet.begin());
          function = fval->getFunction();
        }
        
        std::string fname = function->getName();
        auto actualArgs = callInst->arg_operands();
        auto& formalArgs = function->getArgumentList();
        
        if (fname == "malloc") {
          auto mallocSize = callInst->getOperand(0);
          auto bot = BotValue::getInstance();
          auto botD = AbsD::makeD(bot);
          bool unknownSize = false;
          int64_t nMalloc = -1;
          
          if (ConstantInt* mallocSizeCI = dyn_cast<ConstantInt>(mallocSize)) {
            //TODO: Abstract to be a function
            // Malloc a size of constant int
            nMalloc = mallocSizeCI->getSExtValue();
            unknownSize = false;
            errs() << "malloc size: " << nMalloc << "\n";
          }
          else {
            auto sizeValD = evalAtom(mallocSize, getFp(), getConf(), *AbsState::getModule());
            auto& sizeValSet = sizeValD->getValueSet();
            errs() << "size number: " << sizeValSet.size() << "\n";
            assert(sizeValSet.size() == 1 && "TODO: handle size multiple value");
            
            for (auto it = sizeValSet.begin(); it != sizeValSet.end(); it++) {
              if (isa<IntValue>(**it)) {
                unknownSize = false;
                nMalloc = dyn_cast<IntValue>(&**it)->getValue().getSExtValue();
              }
              else if (isa<AnyIntValue>(**it)) {
                unknownSize = true;
                nMalloc = 2; // 1, *
              }
              else {
                assert(false && "Not a integer");
              }
            }
          }
  
          if (unknownSize) { assert(nMalloc == 2); }
          else { assert(nMalloc > 0); }
          
          auto addrs = ZeroCFAHeapAddr::allocate(callInst, nMalloc + 1); // with an additional T
          auto store = getConf()->getStore();
          auto succ = getConf()->getSucc();
          auto pred = getConf()->getPred();
  
          auto newStore = store->copy();
          auto newSucc = getConf()->getSucc()->copy();
          auto newPred = getConf()->getPred()->copy();
          auto newMeasure = getConf()->getMeasure()->copy();
  
          for (auto& addr : *addrs) {
            newStore->inplaceUpdate(addr, botD);
          }
          assert(newStore->size() == (store->size() + addrs->size()));
  
          auto destAddr = BindAddr::makeBindAddr(inst, getFp());
          auto locVal = LocationValue::makeLocationValue(addrs->front());
          auto locValD = AbsD::makeD(locVal);
          newStore->inplaceStrongUpdateWhen(destAddr, locValD, [&]() {
            auto mOpt = newMeasure->lookup(destAddr);
            if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
              // If the measure is Zero or One, we perform a strong update to one
              newMeasure->inplaceStrongUpdate(destAddr, one);
              return true;
            }
            // If the measure if Inf, we need to join them
            newMeasure->inplaceUpdate(destAddr, one);
            return false;
          });
          
          for (unsigned long i = 0; i < addrs->size()-1; i++) {
            if (unknownSize) {
              assert(i <= 1);
              auto d = AbsLoc::makeMtD();
              d->inplaceAdd(addrs->at(1));
              d->inplaceAdd(addrs->at(2));
              // d = { a_*, a_T }
              newSucc->inplaceUpdate(addrs->at(i), d);
            }
            else {
              newSucc->inplaceUpdate(addrs->at(i), AbsLoc::makeD(addrs->at(i+1)));
            }
          }
  
          for (unsigned long i = addrs->size()-1; i > 0; i--) {
            if (unknownSize) {
              assert(i > 1);
              auto d = AbsLoc::makeMtD();
              d->inplaceAdd(addrs->at(0));
              d->inplaceAdd(addrs->at(1));
              // d = { a_1, a_* }
              newPred->inplaceUpdate(addrs->at(i), d);
            }
            newPred->inplaceUpdate(addrs->at(i), AbsLoc::makeD(addrs->at(i-1)));
          }
          
          for (unsigned long i = 0; i < addrs->size(); i++) {
            if (unknownSize && i == 1) {
              // a_* can be pointed to any number of values
              newMeasure->inplaceUpdate(addrs->at(i), AbstractNat::getInfInstance());
            }
            else {
              newMeasure->inplaceUpdate(addrs->at(i), one);
            }
          }
  
          auto newConf = AbsConf::makeAbsConf(newStore, newSucc, newPred, newMeasure);
          auto newState = AbsState::makeState(nextStmt, getFp(), newConf, getSp());
          states->inplaceInsert(newState);
        }
        else if (fname == "free") {
          auto v = callInst->getOperand(0);
          auto vAddr = BindAddr::makeBindAddr(v, getFp());
          auto locValOpt = getConf()->getStore()->lookup(vAddr);
          
          auto newConf = getConf()->copy();
          if (locValOpt.hasValue()) {
            auto locVals = locValOpt.getValue();
            auto& locValSet = locVals->getValueSet();
            for (auto& v : locValSet) {
              assert(isa<LocationValue>(&*v));
              //auto loc =std::static_pointer_cast<LocationValue>(v);
              auto loc = dyn_cast<LocationValue>(&*v)->getLocation();
              newConf->inplaceRemoveWhen(loc, [&]() {
                auto m = getConf()->getMeasure()->lookup(loc);
                if (!m.hasValue()) return true;
                if (*m.getValue() <= *one) return true;
                return false;
              });
            }
          }
          else {
            assert(false && "Invalid pointer");
          }
          
          auto newState = AbsState::makeState(nextStmt, getFp(), newConf, getSp());
          states->inplaceInsert(newState);
        }
        else {
          auto entry = getEntry(*function);
          auto entryStmt = Stmt::makeStmt(entry);
          std::vector<std::shared_ptr<AbsD>> ds;
          
          for (auto& arg : actualArgs) {
            auto valArg = arg.get();
            auto arg_v = evalAtom(valArg, getFp(), getConf(), *AbsState::getModule());
            ds.push_back(arg_v);
          }
          assert(ds.size() == formalArgs.size());
          
          auto newFP = ZeroCFAFrameAddr::make(function);
          auto newSP = newFP;
          
          auto newStore = getConf()->getStore()->copy();
          auto newMeasure = getConf()->getMeasure()->copy();
          //TODO: if no lhs waiting for a assignment, pass nullptr instead of inst
          auto cont = Cont::makeCont(inst, nextInst, getFp(), getSp());
          auto contD = AbsD::makeD(cont);
          newStore->inplaceUpdate(newFP, contD);
          newMeasure->inplaceUpdate(newFP, AbstractNat::getOneInstance());
          
          auto ds_it = ds.begin();
          auto fa_it = formalArgs.begin();
          for (; ds_it != ds.end() &&
                 fa_it != formalArgs.end();
                 ds_it++, fa_it++) {
            auto addr = BindAddr::makeBindAddr(&*fa_it, newFP);
            //TODO: upadte meausre of local variables
            newStore->inplaceStrongUpdateWhen(addr, *ds_it, [&]() {
              auto mOpt = newMeasure->lookup(addr);
              if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
                // addr -> Zero
                newMeasure->inplaceStrongUpdate(addr, one);
                return true;
              }
              newMeasure->inplaceUpdate(addr, one);
              return false;
            });
          }
  
          auto newConf = AbsConf::makeAbsConf(newStore,
                                              getConf()->getSucc(),
                                              getConf()->getPred(),
                                              newMeasure);
          auto newState = AbsState::makeState(entryStmt, newFP, newConf, newSP);
          states->inplaceInsert(newState);
        }
      }
      else if (isa<LoadInst>(inst)) {
        LoadInst* loadInst = dyn_cast<LoadInst>(inst);
        auto destAddr = BindAddr::makeBindAddr(loadInst, getFp());
        auto destVal = AbsD::makeMtD();
        
        Value* op0 = loadInst->getOperand(0);
        std::shared_ptr<AbsLoc> fromAddrs = addrsOf(op0, getFp(), getConf(), *AbsState::getModule());
        for (auto& fromAddr : fromAddrs->getValueSet()) {
          auto valsOpt = getConf()->getStore()->lookup(fromAddr);
          assert(valsOpt.hasValue());
          auto vals = valsOpt.getValue();
          if (loadInst->getType()->isIntegerTy()) {
            assert(vals->template verify<IntValue>());
          }
          else if (loadInst->getType()->isPointerTy()) {
            assert(vals->template verify<LocationValue>());
          }
          destVal->inplaceJoin(vals);
        }
        
        auto newStore = getConf()->getStore()->copy();
        auto newMeasure = getConf()->getMeasure()->copy();
        newStore->inplaceStrongUpdateWhen(destAddr, destVal, [&]() {
          auto mOpt = getConf()->getMeasure()->lookup(destAddr);
          if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
            newMeasure->inplaceStrongUpdate(destAddr, destVal->getMeasure());
            return true;
          }
          newMeasure->inplaceUpdate(destAddr, destVal->getMeasure());
          return false;
        });
  
        auto newConf = AbsConf::makeAbsConf(newStore,
                                            getConf()->getSucc(),
                                            getConf()->getPred(),
                                            newMeasure);
        auto newState = AbsState::makeState(nextStmt, getFp(), newConf, getSp());
        states->inplaceInsert(newState);
      }
      else if (isa<StoreInst>(inst)) {
        //TODO: test type info and avaliable size
        StoreInst* storeInst = dyn_cast<StoreInst>(inst);
        Value* from = storeInst->getOperand(0);
        Type* fromType = from->getType();
        Value* dest = storeInst->getOperand(1);
        Type* destType = dest->getType();
        
        auto destAddrs = addrsOf(dest, getFp(), getConf(), *AbsState::getModule());
        
        auto newStore = getConf()->getStore()->copy();
        auto newMeasure = getConf()->getMeasure()->copy();
        
        if (fromType->isIntegerTy()) {
          auto vals = evalAtom(from, getFp(), getConf(), *AbsState::getModule());
          for (auto& destAddr : destAddrs->getValueSet()) {
            newStore->inplaceStrongUpdateWhen(destAddr, vals, [&]() {
              auto mOpt = getConf()->getMeasure()->lookup(destAddr);
              if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
                newMeasure->inplaceStrongUpdate(destAddr, vals->getMeasure());
                return true;
              }
              newMeasure->inplaceUpdate(destAddr, vals->getMeasure());
              return false;
            });
          }
        }
        else if (fromType->isPointerTy()) {
          if (isa<Function>(from)) {
            // Function pointer
            Function* f = getModule()->getFunction(from->getName());
            assert(f && "can not get the function");
            auto fVal = FuncValue::makeFuncValue(f);
            auto fD = AbsD::makeD(fVal);
            //TODO: This assume that functions are first order, so just do a strong update.
            for (auto& destAddr : destAddrs->getValueSet()) {
              newStore->inplaceStrongUpdate(destAddr, fD);
              newMeasure->inplaceStrongUpdate(destAddr, one);
            }
          }
          else {
            auto fromAddr = BindAddr::makeBindAddr(from, getFp());
            auto fromValsOpt = getConf()->getStore()->lookup(fromAddr);
            assert(fromValsOpt.hasValue());
            auto fromVals = fromValsOpt.getValue();
            //TODO: Refactor this
            for (auto& destAddr : destAddrs->getValueSet()) {
              newStore->inplaceStrongUpdateWhen(destAddr, fromVals, [&]() {
                auto mOpt = getConf()->getMeasure()->lookup(destAddr);
                if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
                  newMeasure->inplaceStrongUpdate(destAddr, fromVals->getMeasure());
                  return true;
                }
                newMeasure->inplaceUpdate(destAddr, fromVals->getMeasure());
                return false;
              });
            }
          }
        }
        else {
          assert(false && "TODO: Other types not supported");
        }
        auto newConf = AbsConf::makeAbsConf(newStore,
                                            getConf()->getSucc(),
                                            getConf()->getPred(),
                                            newMeasure);
        auto newState = AbsState::makeState(nextStmt, getFp(), newConf, getSp());
        states->inplaceInsert(newState);
      }
      else if (isa<AllocaInst>(inst)) {
        AllocaInst* allocaInst = dyn_cast<AllocaInst>(inst);
        Type* allocaType = allocaInst->getAllocatedType();
        
        uint64_t totalByteSize = AbsState::getModule()->getDataLayout().getTypeAllocSize(allocaType);
        assert(totalByteSize > 0);
        errs() << "total byte size: " << totalByteSize << "\n";
        uint64_t nAlloc = totalByteSize;
        uint64_t step = totalByteSize;
        
        if (auto* allocaArrayType = dyn_cast<ArrayType>(allocaType)) {
          Type* eleType = allocaArrayType->getElementType();
          errs() << "eleType: ";
          eleType->print(errs());
          uint64_t eleNum = allocaArrayType->getNumElements();
          errs() << " num: " << eleNum << "\n";
  
          step = AbsState::getModule()->getDataLayout().getTypeAllocSize(eleType);
          assert(step * eleNum == nAlloc);
        }
        else if (auto* allocaStructType = dyn_cast<StructType>(allocaType)) {
          size_t nEle = allocaStructType->getStructNumElements();
          errs() << "num ele: " << nEle << "\n";
        }
        
        auto store = getConf()->getStore();
        auto succ = getConf()->getSucc();
        auto pred = getConf()->getPred();
        auto measure = getConf()->getMeasure();
        
        auto newStore = store->copy();
        auto newSucc = succ->copy();
        auto newPred = pred->copy();
        auto newMeasure = measure->copy();
        
        auto aTop = this->getSp();
        auto aTopD = AbsLoc::makeD(aTop);
        auto bot = AbsD::makeD(BotValue::getInstance());
        auto addrs = ZeroCFAStackAddr::allocate(allocaInst, nAlloc);
        
        auto locVal = LocationValue::makeLocationValue(addrs->front());
        auto locValD = AbsD::makeD(locVal);
        
        auto destAddr = BindAddr::makeBindAddr(allocaInst, getFp());
        newStore->inplaceStrongUpdateWhen(destAddr, locValD, [&]() {
          auto mOpt = measure->lookup(destAddr);
          if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
            newMeasure->inplaceStrongUpdate(destAddr, locValD->getMeasure());
            return true;
          }
          newMeasure->inplaceUpdate(destAddr, locValD->getMeasure());
          return false;
        });
  
        for (auto& addr : *addrs) {
          newStore->inplaceUpdate(addr, bot);
        }
        
        for (unsigned long i = 0; i < addrs->size(); i++) {
          if (i == addrs->size()-1) {
            newSucc->inplaceUpdate(addrs->at(i), aTopD);
          }
          else {
            newSucc->inplaceUpdate(addrs->at(i), AbsLoc::makeD(addrs->at(i+1)));
          }
        }
        //TODO: assertion
        
        for (unsigned long i = addrs->size()-1; i > 0; i--) {
          newPred->inplaceUpdate(addrs->at(i), AbsLoc::makeD(addrs->at(i-1)));
        }
        newPred->inplaceUpdate(aTop, AbsLoc::makeD(addrs->back()));
        //TODO: assertion
        
        for (unsigned long i = 0; i < addrs->size(); i++) {
          newMeasure->inplaceUpdate(addrs->at(i), one);
        }
        
        auto newConf = AbsConf::makeAbsConf(newStore, newSucc, newPred, newMeasure);
        auto newSp = addrs->front();
        auto newState = AbsState::makeState(nextStmt, getFp(), newConf, newSp);
        states->inplaceInsert(newState);
      }
      else if (isa<GetElementPtrInst>(inst)) {
        GetElementPtrInst* ptrInst = dyn_cast<GetElementPtrInst>(inst);
        
        auto srcObj = ptrInst->getOperand(0);
        auto srcType = srcObj->getType();
        auto ty = srcType;
        int64_t n = 0;
        
        for (int i = 1; i < opNum; i++) {
          Value* offset = ptrInst->getOperand(i);
          int64_t offset_v = 0;
          uint64_t ty_size = 0;
          if (ConstantInt* offset_ci = dyn_cast<ConstantInt>(offset)) {
            offset_v = offset_ci->getValue().getSExtValue();
            if (ty->isPointerTy()) {
              ty = ty->getPointerElementType();
            }
            ty_size = AbsState::getModule()->getDataLayout().getTypeAllocSize(ty);
            n += (ty_size * offset_v);
          }
          else {
            //TODO: variable as offset?
          }
          
          // Move to next level type
          if (ty->isArrayTy()) {
            ty = ty->getArrayElementType();
          }
          else if (ty->isStructTy()) {
            assert(offset_v >= 0);
            ty = ty->getStructElementType(offset_v);
          }
          else {
            // Primitive type
          }
        }
        errs() << "n: " << n << "\n";
        
        auto srcAddr = BindAddr::makeBindAddr(srcObj, getFp());
        auto srcValsOpt = getConf()->getStore()->lookup(srcAddr);
        assert(srcValsOpt.hasValue());
        auto srcVals = srcValsOpt.getValue();
        assert(srcVals->template verify<LocationValue>());
        
        auto& srcValsSet = srcVals->getValueSet();
        errs() << "src obj num: " << srcValsSet.size() << "\n";
        
        auto succ = getConf()->getSucc();
        auto pred = getConf()->getPred();
        auto destLocs = AbsD::makeMtD();
        
        //TODO: test
        for (auto& src : srcValsSet) {
          assert(isa<LocationValue>(&*src));
          auto lv = dyn_cast<LocationValue>(&*src);
          auto addr = lv->getLocation();
          auto addrs = AbsLoc::makeD(addr);
          
          if (n >= 0) {
            for (int i = 0; i < n; i++) {
              auto tempAddrs = AbsLoc::makeMtD();
              for (auto& addr : addrs->getValueSet()) {
                auto newAddrs = succ->lookup(addr).getValue();
                tempAddrs->inplaceJoin(newAddrs);
              }
              addrs->inplaceClear();
              addrs->inplaceJoin(tempAddrs);
            }
          }
          else {
            for (int i = 0; i > n; i--) {
              auto tempAddrs = AbsLoc::makeMtD();
              for (auto& addr : addrs->getValueSet()) {
                auto newAddrs = pred->lookup(addr).getValue();
                tempAddrs->inplaceJoin(newAddrs);
              }
              addrs->inplaceClear();
              addrs->inplaceJoin(tempAddrs);
            }
          }
  
          for (auto& addr : addrs->getValueSet()) {
            destLocs->inplaceAdd(LocationValue::makeLocationValue(addr));
          }
        }
        
        auto newStore = getConf()->getStore()->copy();
        auto newMeasure = getConf()->getMeasure()->copy();
        
        auto destAddr = BindAddr::makeBindAddr(ptrInst, getFp());
        newStore->inplaceStrongUpdateWhen(destAddr, destLocs, [&]() {
          auto mOpt = getConf()->getMeasure()->lookup(destAddr);
          if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
            newMeasure->inplaceStrongUpdate(destAddr, destLocs->getMeasure());
            return true;
          }
          newMeasure->inplaceUpdate(destAddr, destLocs->getMeasure());
          return false;
        });
  
        auto newConf = AbsConf::makeAbsConf(newStore,
                                            getConf()->getSucc(),
                                            getConf()->getPred(),
                                            newMeasure);
        auto newState = AbsState::makeState(nextStmt, getFp(), newConf, getSp());
        states->inplaceInsert(newState);
      }
      else if (Instruction::Add == inst->getOpcode() ||
               Instruction::Sub == inst->getOpcode() ||
               Instruction::Mul == inst->getOpcode()) {
        Value* lhs = inst->getOperand(0);
        Value* rhs = inst->getOperand(1);
        
        auto lhsVals = AbsD::makeMtD();
        auto rhsVals = AbsD::makeMtD();
        
        if (ConstantInt* lhsConst = dyn_cast<ConstantInt>(lhs)) {
          lhsVals->inplaceAdd(IntValue::makeInt(lhsConst->getValue()));
        }
        else {
          auto lhsAddr = BindAddr::makeBindAddr(lhs, getFp());
          lhsVals = getConf()->getStore()->lookup(lhsAddr).getValue();
        }
        
        if (ConstantInt* rhsConst = dyn_cast<ConstantInt>(rhs)) {
          rhsVals->inplaceAdd(IntValue::makeInt(rhsConst->getValue()));
        }
        else {
          auto rhsAddr = BindAddr::makeBindAddr(rhs, getFp());
          rhsVals = getConf()->getStore()->lookup(rhsAddr).getValue();
        }
        
        auto resVals = primOp(inst->getOpcode(), lhsVals, rhsVals);
        auto destAddr = BindAddr::makeBindAddr(inst, getFp());
        
        auto newStore = getConf()->getStore()->copy();
        auto newMeasure = getConf()->getMeasure()->copy();
        newStore->inplaceStrongUpdateWhen(destAddr, resVals, [&]() {
          auto mOpt = getConf()->getMeasure()->lookup(destAddr);
          if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
            newMeasure->inplaceStrongUpdate(destAddr, resVals->getMeasure());
            return true;
          }
          newMeasure->inplaceUpdate(destAddr, resVals->getMeasure());
          return false;
        });
  
        auto newConf = AbsConf::makeAbsConf(newStore,
                                            getConf()->getSucc(),
                                            getConf()->getPred(),
                                            newMeasure);
        auto newState = AbsState::makeState(nextStmt, getFp(), newConf, getSp());
        states->inplaceInsert(newState);
      }
      else if (isa<ICmpInst>(inst)) {
        ICmpInst* cmpInst = dyn_cast<ICmpInst>(inst);
        auto lhs = evalAtom(cmpInst->getOperand(0), getFp(), getConf(), *getModule());
        assert(lhs->template verify<AnyIntValue>());
        auto rhs = evalAtom(cmpInst->getOperand(1), getFp(), getConf(), *getModule());
        assert(rhs->template verify<AnyIntValue>());
        
        auto t = IntValue::makeInt(ConstantInt::getTrue(C)->getValue());
        auto f = IntValue::makeInt(ConstantInt::getFalse(C)->getValue());
        auto result = AbsD::makeMtD();
        
        CmpInst::Predicate pred = cmpInst->getPredicate();
        for (auto& lv : lhs->getValueSet()) {
          for (auto& rv : rhs->getValueSet()) {
            if (isa<AnyIntValue>(&*lv) ||
                isa<AnyIntValue>(&*rv)) {
              result->inplaceAdd(t);
              result->inplaceAdd(f);
              break;
            }
            auto& lhs_v = dyn_cast<IntValue>(&*lv)->getValue();
            auto& rhs_v = dyn_cast<IntValue>(&*rv)->getValue();
            bool res;
            switch (pred) {
              case CmpInst::ICMP_EQ:
                res = (lhs_v == rhs_v);
                break;
              case CmpInst::ICMP_NE:
                res = (lhs_v != rhs_v);
                break;
              case CmpInst::ICMP_SGE:
                res = lhs_v.sge(rhs_v);
                break;
              case CmpInst::ICMP_SGT:
                res = lhs_v.sgt(rhs_v);
                break;
              case CmpInst::ICMP_SLE:
                res = lhs_v.sle(rhs_v);
                break;
              case CmpInst::ICMP_SLT:
                res = lhs_v.slt(rhs_v);
                break;
              default: assert(false && "Predicate not supported");
            }
            if (res) {
              result->inplaceAdd(t);
            }
            else {
              result->inplaceAdd(f);
            }
          }
        }
        
        auto newStore = getConf()->getStore()->copy();
        auto newMeasure = getConf()->getMeasure()->copy();
        auto destAddr = BindAddr::makeBindAddr(cmpInst, getFp());
        newStore->inplaceStrongUpdateWhen(destAddr, result, [&]() {
          auto mOpt = getConf()->getMeasure()->lookup(destAddr);
          if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
            newMeasure->inplaceStrongUpdate(destAddr, result->getMeasure());
            return true;
          }
          newMeasure->inplaceUpdate(destAddr, result->getMeasure());
          return false;
        });
        
        auto newConf = AbsConf::makeAbsConf(newStore,
                                            getConf()->getSucc(),
                                            getConf()->getPred(),
                                            newMeasure);
        auto newState = AbsState::makeState(nextStmt, getFp(), newConf, getSp());
        states->inplaceInsert(newState);
      }
      else if (isa<BranchInst>(inst)) {
        BranchInst* branchInst = dyn_cast<BranchInst>(inst);
        if (opNum == 1) {
          errs() << "Unconditional branch\n";
          BasicBlock* targetBlock = branchInst->getSuccessor(0);
          Instruction* nextSemanticInst = getEntry(*targetBlock);
          auto nextSemanticStmt = Stmt::makeStmt(nextSemanticInst, branchInst);
          auto newState = AbsState::makeState(nextSemanticStmt, getFp(), getConf(), getSp());
          states->inplaceInsert(newState);
        }
        else if (opNum == 3) {
          errs() << "Conditional branch\n";
          BasicBlock* thnBlock = branchInst->getSuccessor(0);
          Instruction* thnEntryInst = getEntry(*thnBlock);
          auto thnEntryStmt = Stmt::makeStmt(thnEntryInst, branchInst);
          auto thnState = AbsState::makeState(thnEntryStmt, getFp(), getConf(), getSp());
          
          BasicBlock* elsBlock = branchInst->getSuccessor(1);
          Instruction* elsEntryBlock = getEntry(*elsBlock);
          auto elsEntryStmt = Stmt::makeStmt(elsEntryBlock, branchInst);
          auto elsState = AbsState::makeState(elsEntryStmt, getFp(), getConf(), getSp());
  
          Value* cnd = branchInst->getOperand(0);
          auto cndVals = evalAtom(cnd, getFp(), getConf(), *getModule());
          assert(cndVals->template verify<AnyIntValue>());
          
          for (auto& v : cndVals->getValueSet()) {
            if (IntValue* i = dyn_cast<IntValue>(&*v)) {
              if (i->getValue() == ConstantInt::getFalse(C)->getValue()) {
                states->inplaceInsert(elsState);
              }
              else {
                states->inplaceInsert(thnState);
              }
            }
            else if (AnyIntValue* ai = dyn_cast<AnyIntValue>(&*v)) {
              states->inplaceInsert(thnState);
              states->inplaceInsert(elsState);
              break;
            }
            else {
              assert(false && "Not an integer");
            }
          }
        }
        else {
          assert(false && "Number of operands is greater than 3");
        }
      }
      else if (isa<BitCastInst>(inst)) {
        BitCastInst* bitCastInst = dyn_cast<BitCastInst>(inst);
        Type* destType = bitCastInst->getDestTy();
        Type* srcType = bitCastInst->getSrcTy();
        Value* src = bitCastInst->getOperand(0);
        
        auto locVals = evalAtom(src, getFp(), getConf(), *getModule());
        assert(locVals->template verify<LocationValue>());
        
        auto destAddr = BindAddr::makeBindAddr(bitCastInst, getFp());
        auto newStore = getConf()->getStore()->copy();
        auto newMeasure = getConf()->getMeasure()->copy();
        newStore->inplaceStrongUpdateWhen(destAddr, locVals, [&]() {
          auto mOpt = getConf()->getMeasure()->lookup(destAddr);
          if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
            newMeasure->inplaceStrongUpdate(destAddr, locVals->getMeasure());
            return true;
          }
          newMeasure->inplaceUpdate(destAddr, locVals->getMeasure());
          return false;
        });
  
        auto newConf = AbsConf::makeAbsConf(newStore,
                                            getConf()->getSucc(),
                                            getConf()->getPred(),
                                            newMeasure);
        auto newState = AbsState::makeState(nextStmt, getFp(), newConf, getSp());
        states->inplaceInsert(newState);
      }
      else if (isa<SExtInst>(inst)) {
        SExtInst* sExtInst = dyn_cast<SExtInst>(inst);
        Value* op0 = sExtInst->getOperand(0);
        
        Type* destType = sExtInst->getDestTy();
        auto vals = evalAtom(op0, getFp(), getConf(), *getModule());
        assert(vals->template verify<AnyIntValue>());
        
        auto destVals = AbsD::makeMtD();
        for (auto& v : vals->getValueSet()) {
          if (IntValue* iv = dyn_cast<IntValue>(&*v)) {
            APInt apInt = iv->getValue().sext(destType->getIntegerBitWidth());
            auto newV = IntValue::makeInt(apInt);
            destVals->inplaceAdd(newV);
          }
          else if (AnyIntValue* ai = dyn_cast<AnyIntValue>(&*v)) {
            destVals->inplaceAdd(AnyIntValue::getInstance());
          }
          else {
            assert(false && "Not an integer");
          }
        }
        
        auto destAddr = BindAddr::makeBindAddr(sExtInst, getFp());
        auto newStore = getConf()->getStore()->copy();
        auto newMeasure = getConf()->getMeasure()->copy();
        newStore->inplaceStrongUpdateWhen(destAddr, destVals, [&]() {
          auto mOpt = getConf()->getMeasure()->lookup(destAddr);
          if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
            newMeasure->inplaceStrongUpdate(destAddr, destVals->getMeasure());
            return true;
          }
          newMeasure->inplaceUpdate(destAddr, destVals->getMeasure());
          return false;
        });
  
        auto newConf = AbsConf::makeAbsConf(newStore,
                                            getConf()->getSucc(),
                                            getConf()->getPred(),
                                            newMeasure);
        auto newState = AbsState::makeState(nextStmt, getFp(), newConf, getSp());
        states->inplaceInsert(newState);
      }
      else if (isa<ZExtInst>(inst)) {
        SExtInst* sExtInst = dyn_cast<SExtInst>(inst);
        Value* op0 = sExtInst->getOperand(0);
  
        Type* destType = sExtInst->getDestTy();
        auto vals = evalAtom(op0, getFp(), getConf(), *getModule());
        assert(vals->template verify<AnyIntValue>());
  
        auto destVals = AbsD::makeMtD();
        for (auto& v : vals->getValueSet()) {
          if (IntValue* iv = dyn_cast<IntValue>(&*v)) {
            APInt apInt = iv->getValue().zext(destType->getIntegerBitWidth());
            auto newV = IntValue::makeInt(apInt);
            destVals->inplaceAdd(newV);
          }
          else if (AnyIntValue* ai = dyn_cast<AnyIntValue>(&*v)) {
            destVals = AbsD::makeD(AnyIntValue::getInstance());
            break;
            //destVals->inplaceAdd(AnyIntValue::getInstance());
          }
          else {
            assert(false && "Not an integer");
          }
        }
  
        auto destAddr = BindAddr::makeBindAddr(sExtInst, getFp());
        auto newStore = getConf()->getStore()->copy();
        auto newMeasure = getConf()->getMeasure()->copy();
        newStore->inplaceStrongUpdateWhen(destAddr, destVals, [&]() {
          auto mOpt = getConf()->getMeasure()->lookup(destAddr);
          if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
            newMeasure->inplaceStrongUpdate(destAddr, destVals->getMeasure());
            return true;
          }
          newMeasure->inplaceUpdate(destAddr, destVals->getMeasure());
          return false;
        });
  
        auto newConf = AbsConf::makeAbsConf(newStore,
                                            getConf()->getSucc(),
                                            getConf()->getPred(),
                                            newMeasure);
        auto newState = AbsState::makeState(nextStmt, getFp(), newConf, getSp());
        states->inplaceInsert(newState);
      }
      else if (isa<TruncInst>(inst)) {
        TruncInst* truncInst = dyn_cast<TruncInst>(inst);
        Value* op0 = truncInst->getOperand(0);
  
        Type* destType = truncInst->getDestTy();
        errs() << "dest type: ";
        destType->print(errs());
        errs() << "\n";
        
        auto vals = evalAtom(op0, getFp(), getConf(), *getModule());
        errs() << "\n"; vals->print(); errs() << "\n";
        assert(vals->template verify<AnyIntValue>());
        
        auto destVals = AbsD::makeMtD();
        for (auto& v : vals->getValueSet()) {
          if (IntValue* iv = dyn_cast<IntValue>(&*v)) {
            APInt apInt = iv->getValue().trunc(destType->getIntegerBitWidth());
            auto newV = IntValue::makeInt(apInt);
            destVals->inplaceAdd(newV);
          }
          else if (AnyIntValue* ai = dyn_cast<AnyIntValue>(&*v)) {
            destVals = AbsD::makeD(AnyIntValue::getInstance());
            break;
            //destVals->inplaceAdd(AnyIntValue::getInstance());
          }
          else {
            assert(false && "Not an integer");
          }
        }
  
        auto destAddr = BindAddr::makeBindAddr(truncInst, getFp());
        auto newStore = getConf()->getStore()->copy();
        auto newMeasure = getConf()->getMeasure()->copy();
        newStore->inplaceStrongUpdateWhen(destAddr, destVals, [&]() {
          auto mOpt = getConf()->getMeasure()->lookup(destAddr);
          if (!mOpt.hasValue() || *mOpt.getValue() <= *one) {
            newMeasure->inplaceStrongUpdate(destAddr, destVals->getMeasure());
            return true;
          }
          newMeasure->inplaceUpdate(destAddr, destVals->getMeasure());
          return false;
        });
  
        auto newConf = AbsConf::makeAbsConf(newStore,
                                            getConf()->getSucc(),
                                            getConf()->getPred(),
                                            newMeasure);
        auto newState = AbsState::makeState(nextStmt, getFp(), newConf, getSp());
        states->inplaceInsert(newState);
      }
      else if (isa<PHINode>(inst)) {
        //TODO:
      }
      else {
        assert(false && "Unsupported instruction");
      }
      
      errs() << "next states size: " << states->size() << "\n";
      return states;
    }
    
    void print() {
      errs() << "state[" << myId << "][";
      getControl()->getInst()->print(errs());
      errs() << "]";
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
      
      auto initFp = ZeroCFAStackAddr::initFp(&M);
      
      std::shared_ptr<AbsState> initState = makeState(initStmt, initFp, initConf, initFp);
      return initState;
    }
  };
  
  void run(AbsState::StatePtrType s);
}

#endif //LLVM_ABSTRACTAAM_H
