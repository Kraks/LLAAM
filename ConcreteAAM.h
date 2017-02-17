//
// Created by WeiGuannan on 08/02/2017.
//

#include "AAM.h"

#ifndef LLVM_CONCRETEAAM_H
#define LLVM_CONCRETEAAM_H

// TODO cast

namespace ConcreteAAM {
  using namespace AAM;
  
  #define MIN_ALLOC 4;
  
  class ConcreteHeapAddr : public HeapAddr {
  public:
    ConcreteHeapAddr() : HeapAddr(KConcreteHeapAddr) {
      myId = id++;
    }
    
    static bool classof(const Location* loc) {
      return loc->getKind() == KConcreteHeapAddr;
    }
    
    virtual bool equalTo(const Location& that) const override {
      if (!isa<ConcreteHeapAddr>(&that))
        return false;
      auto* newThat = dyn_cast<ConcreteHeapAddr>(&that);
      //errs() << "that id: " << newThat->myId << " ; this id: " << this->myId << "\n";
      return newThat->myId == this->myId;
    }
    
    virtual size_t hashValue() const override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(myId));
      seed = hash_combine(seed, hash_value("ConcreteHeapAddr"));
      return seed;
    }
    
    virtual void print() const override {
      errs() << "ConcHeapAddr[" << myId << "]";
    }
  
  private:
    unsigned long long myId;
    static unsigned long long id;
  };
  
  class ConcreteStackAddr : public StackAddr {
  public:
    typedef std::shared_ptr<ConcreteStackAddr> ConcreteStackAddrPtrType;
    ConcreteStackAddr() : StackAddr(KConcreteStackAddr) {
      //errs() << "ConcreteStackAddr constructor\n";
      myId = id++;
    }
    
    static bool classof(const Location* loc) {
      return loc->getKind() == KConcreteStackAddr;
    }
    
    static ConcreteStackAddrPtrType initFp() {
      static ConcreteStackAddrPtrType initFp = makeConcreteStackAdd();
      return initFp;
    }
    
    static ConcreteStackAddrPtrType makeConcreteStackAdd() {
      ConcreteStackAddrPtrType s = std::make_shared<ConcreteStackAddr>();
      return s;
    }
    
    static std::shared_ptr<std::vector<ConcreteStackAddrPtrType>> allocate(size_t n) {
      std::shared_ptr<std::vector<ConcreteStackAddrPtrType>> v = std::make_shared<std::vector<ConcreteStackAddrPtrType>>();
      for (int i = 0; i < n; i++) {
        ConcreteStackAddrPtrType s = makeConcreteStackAdd();
        v->push_back(s);
      }
      assert(v->size() == n);
      return v;
    }
    
    virtual bool equalTo(const Location& that) const override {
      if (!isa<ConcreteStackAddr>(&that))
        return false;
      auto* newThat = dyn_cast<ConcreteStackAddr>(&that);
      //errs() << "StackPtr equalTo: " << newThat->myId << " " << this->myId << "\n";
      return newThat->myId == this->myId;
    }
    
    virtual size_t hashValue() const override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(myId));
      seed =hash_combine(seed, hash_value("ConcreteStackAddr"));
      return seed;
    }
    
    virtual void print() const override {
      errs() << "ConcStackAddr[" << myId << "]";
    }
  
  private:
    unsigned long long myId;
    static unsigned long long id;
  };
  
  typedef ConcreteStackAddr ConcreteFrameAddr;
  
  typedef Store<Location, AbstractValue, LocationLess> ConcreteStore;
  
  typedef Store<Location, Location, LocationLess> ConcreteSucc;
  
  typedef Store<Location, Location, LocationLess> ConcretePred;
  
  class DummyMeasure {
  public:
    size_t hashValue() { return 0; }
    inline bool operator==(DummyMeasure& that) { return true; }
    inline bool operator!=(DummyMeasure& that) { return true; }
  };
  
  class ConcreteConf : public Conf<ConcreteStore, ConcreteSucc, ConcretePred, DummyMeasure> {
  public:
    typedef std::shared_ptr<ConcreteConf> ConfPtrType;
      
    ConcreteConf(StorePtrType store, SuccPtrType succ, PredPtrType pred) :
      Conf(store, succ, pred) {}
    
    static ConfPtrType makeConf(StorePtrType store, SuccPtrType succ, PredPtrType pred) {
      ConfPtrType conf = std::make_shared<ConcreteConf>(store, succ, pred);
      return conf;
    }
  };
  
  
  std::shared_ptr<ConcreteStore> getInitStore(Module& M);
  std::shared_ptr<ConcreteConf>  getInitConf(Module& M);
  
  std::shared_ptr<AbstractValue> evalAtom(ConstantInt* i,
                                          std::shared_ptr<FrameAddr> fp,
                                          std::shared_ptr<ConcreteConf> conf,
                                          Module& M);
  std::shared_ptr<AbstractValue> evalAtom(Value* val,
                                          std::shared_ptr<FrameAddr> fp,
                                          std::shared_ptr<ConcreteConf> conf,
                                          Module& M);
  
  std::shared_ptr<Location> addrsOf(std::string var,
                                    std::shared_ptr<FrameAddr> fp,
                                    std::shared_ptr<ConcreteConf> conf,
                                    Module& M);
  std::shared_ptr<Location> addrsOf(Value* lhs,
                                    std::shared_ptr<FrameAddr> fp,
                                    std::shared_ptr<ConcreteConf> conf,
                                    Module& M);
  
  class ConcreteState : public State<Stmt, FrameAddr, ConcreteConf, StackAddr> {
  private:
    static Module* module;
    static unsigned long long id;
    unsigned long long myId;
    
  public:
    static void setModule(Module* M) {
      module = M;
    }
    static Module* getModule() {
      assert(module != nullptr);
      return module;
    }
    
    typedef std::shared_ptr<ConcreteState> StatePtrType;
    
    ConcreteState(CPtrType c, EPtrType e, SPtrType s, KPtrType k) :
      State(c, e, s, k) {
      myId = id++;
    };
    
    StatePtrType copy() {
      auto s = ConcreteState::makeState(this->getControl(), this->getEnv(), this->getStore(), this->getCont());
      return s;
    }
    
    StatePtrType next() {
      // TODO: implement the real next()
      Instruction* inst = getControl()->getInst();
      Instruction* nextInst = getSyntacticNextInst(inst);
      
      auto nextStmt = Stmt::makeStmt(nextInst);
      errs() << "Current state[" << myId << "] ";
      inst->dump();
      
      if (isa<ReturnInst>(inst)) {
        ReturnInst* returnInst = dyn_cast<ReturnInst>(inst);
        
        auto fp = this->getEnv();
        if (*fp == *ConcreteStackAddr::initFp()) {
          return this->copy();
        }
        else {
          //TODO
        }
      }
      else if (isa<CallInst>(inst)) {
        //TODO: invoke inst
        CallInst* callInst = dyn_cast<CallInst>(inst);
        Function* function = callInst->getFunction();
        std::string fname = function->getName();
        errs() << "function name: " << fname << "\n";
        //TODO: malloc/free
        if (fname == "malloc") {
          
        }
        else if (fname == "free") {
          
        }
        else {
          
        }
      }
      else if (isa<LoadInst>(inst)) {
        LoadInst* loadInst = dyn_cast<LoadInst>(inst);
        auto addr = addrsOf(loadInst, this->getEnv(), this->getStore(), *ConcreteState::getModule());
        errs() << "load num oprands: " << loadInst->getNumOperands() << "\n";
      }
      else if (isa<StoreInst>(inst)) {
        StoreInst* storeInst = dyn_cast<StoreInst>(inst);
        Value* op0 = storeInst->getOperand(0);
        Value* op1 = storeInst->getOperand(1);
        
        errs() << "store num oprands: " << storeInst->getNumOperands() << "\n";
        errs() << "op(0): ";
        storeInst->getOperand(0)->print(errs());
        errs() << ". type: " << storeInst->getOperand(0)->getType()->getTypeID();
        errs() << "\nop0 constantint?: " << (isa<ConstantInt>(op0)) << "\n";
        errs() << "\nop(1): ";
        storeInst->getOperand(1)->print(errs());
        errs() << ". type: " << storeInst->getOperand(1)->getType()->getTypeID();
        errs() << "\n";
        
        if (ConstantInt* op0_ci = dyn_cast<ConstantInt>(op0)) {
          auto val = evalAtom(op0_ci, getEnv(), getStore(), *ConcreteState::getModule());
          auto addr = addrsOf(op1, this->getEnv(), this->getStore(), *ConcreteState::getModule());
          auto newStore = this->getStore()->getStore()->copy();
          newStore->inplaceUpdate(addr, val);
          auto newConf = ConcreteConf::makeConf(newStore, this->getStore()->getSucc(), this->getStore()->getPred());
          auto newState = ConcreteState::makeState(nextStmt, this->getEnv(), newConf, this->getCont());
          return newState;
        }
        else {
          errs() << "store a vairable to another\n";
        }
      }
      else if (isa<AllocaInst>(inst)) {
        AllocaInst* allocaInst = dyn_cast<AllocaInst>(inst);
        Type* allocaType = allocaInst->getAllocatedType();
        // The typeByteSize is the number of bytes, for example, int_32 is 4 bytes.
        // For now, assuming that the only type is int_32, and we can set the minimum allocation size in store to 4 byte
        uint64_t typeByteSize = ConcreteState::getModule()->getDataLayout().getTypeAllocSize(allocaType);
        assert(typeByteSize > 0);
  
        // TODO: If there are other primitive types (for example, byte, short or long),
        // TODO: then we need to set the minimum allocation size to 1 byte
        // TODO: and also handle the cast operation.
        uint64_t nAlloc = typeByteSize / MIN_ALLOC;
        
        bool isArrayAlloc = allocaInst->isArrayAllocation();
        Value* arraySize = allocaInst->getArraySize(); //TODO: allocate array size
        if (isArrayAlloc && isa<ConstantInt>(arraySize)) { errs() << "array size is constant\n"; }
        
        /*
        errs() << "allocaInst num oprands: " << allocaInst->getNumOperands() << "\n";
        errs() << "allocaInst array alloc? " << allocaInst->isArrayAllocation() << "\n";
        errs() << "allocaInst alloca size: "  << allocaInst->getArraySize() << "\n";
        */
        
        auto store = this->getStore()->getStore();
        auto succ = this->getStore()->getSucc();
        auto pred = this->getStore()->getPred();
        
        // aTop is the stack pointer in current state.
        auto aTop = this->getCont();
        auto bot = BotValue::getInstance();
        auto addrs = ConcreteStackAddr::allocate(nAlloc);
        
        auto newStore = store->copy();
        auto locVal = LocationValue::makeLocationValue(addrs->front());
        for (auto& addr : *addrs) {
          newStore->inplaceUpdate(addr, bot);
        }
        assert(newStore->size() == (store->size() + addrs->size()));
        newStore->inplaceUpdate(addrsOf(inst, this->getEnv(), this->getStore(), *ConcreteState::getModule()), locVal);
        
        auto newSucc = succ->copy();
        for (unsigned long i = 0; i < addrs->size(); i++) {
          if (i == addrs->size()-1) {
            newSucc->inplaceUpdate(addrs->at(i), aTop);
          }
          else {
            newSucc->inplaceUpdate(addrs->at(i), addrs->at(i+1));
          }
        }
        assert(newSucc->size() == (succ->size() + addrs->size()));
        
        auto newPred = pred->copy();
        std::shared_ptr<Location> last = *addrs->begin();
        newPred->inplaceUpdate(aTop, addrs->back());
        for (unsigned long i = addrs->size()-1; i > 0; i--) {
          newPred->inplaceUpdate(addrs->at(i), addrs->at(i-1));
        }
        assert(newPred->size() == (pred->size() + addrs->size()));
        
        auto newConf = ConcreteConf::makeConf(newStore, newSucc, newPred);
        auto newStackPtr = addrs->front();
        auto newState = ConcreteState::makeState(nextStmt, this->getEnv(), newConf, newStackPtr);
        return newState;
      }
      else if (isa<BranchInst>(inst)) {
        BranchInst* branchInst = dyn_cast<BranchInst>(inst);
      }
      else if (isa<SwitchInst>(inst)) {
        
      }
      else {
        
      }
      
      ///////////////////////////
      assert(nextInst != nullptr && "next instruction is nullptr");
      auto stmt = Stmt::makeStmt(nextInst);
      auto state = makeState(stmt, getEnv(), getStore(), getCont());
      return state;
    }
    
    static StatePtrType inject(Module& M, std::string mainFuncName) {
      std::shared_ptr<ConcreteConf> initConf = getInitConf(M);
      Function* main = M.getFunction(mainFuncName);
      Instruction* entry = getEntry(*main);
      std::shared_ptr<Stmt> initStmt = std::make_shared<Stmt>(entry);
      std::shared_ptr<ConcreteState> initState = makeState(initStmt, ConcreteStackAddr::initFp(), initConf, ConcreteStackAddr::initFp());
      return initState;
    }
    
    static StatePtrType makeState(CPtrType c, EPtrType e, SPtrType s, KPtrType k) {
      std::shared_ptr<ConcreteState> state = std::make_shared<ConcreteState>(c, e, s, k);
      return state;
    }
  };
  
  void run(ConcreteState::StatePtrType s);
}

#endif //LLVM_CONCRETEAAM_H
