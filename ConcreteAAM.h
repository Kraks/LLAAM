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
  
  typedef uint8_t RemoveOption;
  const RemoveOption RM_ALL   = 0xFF;
  const RemoveOption RM_STORE = 0x01;
  const RemoveOption RM_SUCC  = 0x02;
  const RemoveOption RM_PRED  = 0x04;
  
  class ConcreteHeapAddr : public HeapAddr {
  public:
    typedef std::shared_ptr<ConcreteHeapAddr> ConcreteHeapAddrPtrType;
    
    ConcreteHeapAddr() : HeapAddr(KConcreteHeapAddr) {
      myId = id++;
    }
    
    static ConcreteHeapAddrPtrType makeConcreteHeapAddr() {
      ConcreteHeapAddrPtrType h = std::make_shared<ConcreteHeapAddr>();
      return h;
    }
    
    static bool classof(const Location* loc) {
      return loc->getKind() == KConcreteHeapAddr;
    }
    
    static std::shared_ptr<std::vector<ConcreteHeapAddrPtrType>> allocate(size_t n) {
      std::shared_ptr<std::vector<ConcreteHeapAddrPtrType>> v = std::make_shared<std::vector<ConcreteHeapAddrPtrType>>();
      for (int i = 0; i < n; i++) {
        ConcreteHeapAddrPtrType s = makeConcreteHeapAddr();
        v->push_back(s);
      }
      assert(v->size() == n);
      return v;
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
      static ConcreteStackAddrPtrType initFp = makeConcreteStackAddr();
      return initFp;
    }
    
    static ConcreteStackAddrPtrType makeConcreteStackAddr() {
      ConcreteStackAddrPtrType s = std::make_shared<ConcreteStackAddr>();
      return s;
    }
    
    static std::shared_ptr<std::vector<ConcreteStackAddrPtrType>> allocate(size_t n) {
      std::shared_ptr<std::vector<ConcreteStackAddrPtrType>> v = std::make_shared<std::vector<ConcreteStackAddrPtrType>>();
      for (int i = 0; i < n; i++) {
        ConcreteStackAddrPtrType s = makeConcreteStackAddr();
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
    
    ConfPtrType copy() {
      auto conf = makeConf(this->getStore()->copy(), this->getSucc()->copy(), this->getPred()->copy());
      return conf;
    }
    
    // Immutable remove
    ConfPtrType remove(ConcreteStore::Key key) {
      auto newConf = makeConf(this->getStore()->remove(key),
                              this->getSucc()->remove(key),
                              this->getPred()->remove(key));
      /*
      assert(newConf->getStore()->size() == (this->getStore()->size()-1));
      assert(newConf->getSucc()->size() == (this->getSucc()->size()-1));
      assert(newConf->getPred()->size() == (this->getPred()->size()-1));
      */
      return newConf;
    }
    
    void inplaceRemove(ConcreteStore::Key key, RemoveOption opt = ConcreteAAM::RM_ALL) {
      if (opt & ConcreteAAM::RM_STORE) {
        errs() << "Remove ";
        key->print();
        errs() <<" from store\n";
        this->getStore()->inplaceRemove(key);
      }
      if (opt & ConcreteAAM::RM_SUCC) {
        errs() << "Remove ";
        key->print();
        errs() << " from succ\n";
        this->getSucc()->inplaceRemove(key);
      }
      if (opt & ConcreteAAM::RM_PRED) {
        errs() << "Remove ";
        key->print();
        errs() << " from pred\n";
        this->getPred()->inplaceRemove(key);
      }
    }
    
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
      auto s = ConcreteState::makeState(this->getControl(), this->getFp(), this->getConf(), this->getSp());
      return s;
    }
    
    StatePtrType next() {
      // Core instruction:
      //    StoreInst, LoadInst, AllocaInst, ReturnInst
      //    CallInst @malloc, CallInst @free
      // TODO: implement the real next()
      Instruction* inst = getControl()->getInst();
      Instruction* nextInst = getSyntacticNextInst(inst);
      
      auto nextStmt = Stmt::makeStmt(nextInst);
      errs() << "\nCurrent state[" << myId << "] ";
      inst->dump();
      
      if (isa<ReturnInst>(inst)) {
        ReturnInst* returnInst = dyn_cast<ReturnInst>(inst);
        errs() << "op nums: " << returnInst->getNumOperands() << "\n";
        
        auto fp = this->getFp();
        if (*fp == *ConcreteStackAddr::initFp()) {
          return this->copy();
        }
  
        auto contValOpt = this->getConf()->getStore()->lookup(this->getFp());
        assert(contValOpt.hasValue());
        auto contVal = contValOpt.getValue();
        assert(isa<Cont>(&*contVal));
        auto cont = dyn_cast<Cont>(&*contVal);
        auto newStore = this->getConf()->getStore()->copy();
        
        if (returnInst->getNumOperands() > 0) {
          auto ret = returnInst->getOperand(0);
          auto rval = evalAtom(ret, this->getFp(), this->getConf(), *ConcreteState::getModule());
          auto lhs = cont->getLhs();
          auto destAddr = BindAddr::makeBindAddr(lhs, cont->getFrameAddr());
          newStore->inplaceUpdate(destAddr, rval);
        }
        
        auto newConf = ConcreteConf::makeConf(newStore, getConf()->getSucc(), getConf()->getPred());
        //GC
        newConf->inplaceRemove(fp);
        newConf->inplaceRemove(cont->getStackAddr(), ConcreteAAM::RM_PRED);
        
        auto calleeStmt = Stmt::makeStmt(cont->getInst());
        auto newState = ConcreteState::makeState(calleeStmt, cont->getFrameAddr(), newConf, cont->getStackAddr());
        return newState;
      }
      else if (isa<InvokeInst>(inst)) {
        
      }
      else if (isa<CallInst>(inst)) {
        CallInst* callInst = dyn_cast<CallInst>(inst);
        Function* function = callInst->getCalledFunction();
        std::string fname = function->getName();
        auto actualArgs = callInst->arg_operands();
        auto& formalArgs = function->getArgumentList();
        
        errs() << "function name: " << fname << "\n";
        errs() << "op num: " << callInst->getNumOperands() << "\n";
        
        if (fname == "malloc") {
          
        }
        else if (fname == "free") {
          
        }
        else {
          // TODO: call a function pointer
          auto entry = getEntry(*function);
          auto entryStmt = Stmt::makeStmt(entry);
          std::vector<std::shared_ptr<AbstractValue>> ds;
          for (auto& arg : actualArgs) {
            auto valArg = arg.get();
            auto arg_v = evalAtom(valArg, getFp(), getConf(), *ConcreteState::getModule());
            ds.push_back(arg_v);
          }
          assert(ds.size() == formalArgs.size());
  
          auto newFP = ConcreteFrameAddr::makeConcreteStackAddr(); //TODO: rename it
          auto newSP = newFP;
          
          auto newStore = this->getConf()->getStore()->copy();
          //TODO: if just a call function, pass nullptr instead of inst
          auto cont = Cont::makeCont(inst, nextInst, this->getFp(), this->getSp());
          newStore->inplaceUpdate(newFP, cont);
          
          auto ds_it = ds.begin();
          auto fa_it = formalArgs.begin();
          for (; ds_it != ds.end() &&
                 fa_it != formalArgs.end();
                 ds_it++, fa_it++) {
            auto addr = BindAddr::makeBindAddr(&*fa_it, newFP);
            newStore->inplaceUpdate(addr, *ds_it);
          }
  
          auto newConf = ConcreteConf::makeConf(newStore, this->getConf()->getSucc(), this->getConf()->getSucc());
          auto newState = ConcreteState::makeState(entryStmt, newFP, newConf, newSP);
          return newState;
        }
      }
      else if (isa<LoadInst>(inst)) {
        LoadInst* loadInst = dyn_cast<LoadInst>(inst);
        Value* op0 = loadInst->getOperand(0);
        auto destAddr = BindAddr::makeBindAddr(loadInst, this->getFp());
        
        /*
        errs() << "load num oprands: " << loadInst->getNumOperands() << "\n";
        errs() << "op(0): ";
        op0->print(errs());
        errs() << "\n";
        */
        
        auto targetAddr = addrsOf(op0, this->getFp(), this->getConf(), *ConcreteState::getModule());
        auto valOpt = this->getConf()->getStore()->lookup(targetAddr);
        assert(valOpt.hasValue());
        auto val = valOpt.getValue();
        
        if (loadInst->getType()->isIntegerTy()) {
          assert(isa<IntValue>(*val));
        } else if (loadInst->getType()->isPointerTy()) {
          assert(isa<LocationValue>(*val));
        }
        
        auto newStore = this->getConf()->getStore()->copy();
        newStore->inplaceUpdate(destAddr, val);
        auto newConf = ConcreteConf::makeConf(newStore, this->getConf()->getSucc(), this->getConf()->getPred());
        auto newState = ConcreteState::makeState(nextStmt, this->getFp(), newConf, this->getSp());
        return newState;
      }
      else if (isa<StoreInst>(inst)) {
        StoreInst* storeInst = dyn_cast<StoreInst>(inst);
        Value* op0 = storeInst->getOperand(0);
        Type* op0_ty = op0->getType();
        Value* op1 = storeInst->getOperand(1);
        Type* op1_ty = op1->getType();
        
        /*
        errs() << "store num oprands: " << storeInst->getNumOperands() << "\n";
        errs() << "op(0): ";
        storeInst->getOperand(0)->print(errs());
        errs() << ". type: " << storeInst->getOperand(0)->getType()->getTypeID();
        errs() << "\nop0 constantint?: " << (isa<ConstantInt>(op0)) << "\n";
        errs() << "op(1): ";
        storeInst->getOperand(1)->print(errs());
        errs() << ". type: " << storeInst->getOperand(1)->getType()->getTypeID();
        errs() << "\n";
        */
        
        auto destAddr = addrsOf(op1, this->getFp(), this->getConf(), *ConcreteState::getModule());
        errs() << "dest: ";
        destAddr->print();
        errs() << "\n";
  
        auto newStore = this->getConf()->getStore()->copy();
        
        if (op0_ty->isIntegerTy()) {
          auto val = evalAtom(op0, getFp(), getConf(), *ConcreteState::getModule());
          newStore->inplaceUpdate(destAddr, val);
        }
        else if (op0_ty->isPointerTy()) {
          auto fromAddr = addrsOf(op0, getFp(), getConf(), *ConcreteState::getModule());
          auto val = LocationValue::makeLocationValue(fromAddr);
          newStore->inplaceUpdate(destAddr, val);
        }
        
        auto newConf = ConcreteConf::makeConf(newStore, this->getConf()->getSucc(), this->getConf()->getPred());
        auto newState = ConcreteState::makeState(nextStmt, this->getFp(), newConf, this->getSp());
        return newState;
      }
      else if (isa<AllocaInst>(inst)) {
        AllocaInst* allocaInst = dyn_cast<AllocaInst>(inst);
        Type* allocaType = allocaInst->getAllocatedType();
        // The typeByteSize is the number of bytes, for example, int_32 is 4 bytes.
        // Note: For now, assuming that the only type is int_32, and we can set the
        // minimum allocation size in store to 4 byte.
        uint64_t typeByteSize = ConcreteState::getModule()->getDataLayout().getTypeAllocSize(allocaType);
        assert(typeByteSize > 0);
  
        // TODO: If there are other primitive types (for example, byte, short or long),
        // TODO: then we need to set the minimum allocation size to 1 byte
        // TODO: and also handle the cast operations.
        //uint64_t nAlloc = typeByteSize / MIN_ALLOC;
        uint64_t nAlloc = 1;
        
        bool isArrayAlloc = allocaInst->isArrayAllocation();
        Value* arraySize = allocaInst->getArraySize(); //TODO: allocate array size
        if (isArrayAlloc && isa<ConstantInt>(arraySize)) { errs() << "array size is constant\n"; }
        
        /*
        errs() << "allocaInst num oprands: " << allocaInst->getNumOperands() << "\n";
        errs() << "allocaInst array alloc? " << allocaInst->isArrayAllocation() << "\n";
        errs() << "allocaInst alloca size: "  << allocaInst->getArraySize() << "\n";
        */
        
        auto store = this->getConf()->getStore();
        auto succ = this->getConf()->getSucc();
        auto pred = this->getConf()->getPred();
        
        // aTop is the stack pointer in current state.
        auto aTop = this->getSp();
        auto bot = BotValue::getInstance();
        auto addrs = ConcreteStackAddr::allocate(nAlloc);
        
        auto newStore = store->copy();
        auto locVal = LocationValue::makeLocationValue(addrs->front());
        for (auto& addr : *addrs) {
          newStore->inplaceUpdate(addr, bot);
        }
        assert(newStore->size() == (store->size() + addrs->size()));
        auto destAddr = BindAddr::makeBindAddr(inst, this->getFp());
        newStore->inplaceUpdate(destAddr, locVal);
        
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
        auto newState = ConcreteState::makeState(nextStmt, this->getFp(), newConf, newStackPtr);
        return newState;
      }
      else if (isa<BranchInst>(inst)) {
        BranchInst* branchInst = dyn_cast<BranchInst>(inst);
      }
      else if (isa<GetElementPtrInst>(inst)) {
        
      }
      else if (Instruction::Add == inst->getOpcode() ||
               Instruction::Sub == inst->getOpcode()) {
        Value* lhs = inst->getOperand(0);
        Value* rhs = inst->getOperand(1);
        APInt lhs_v;
        APInt rhs_v;
        
        assert(lhs->getType()->isIntegerTy());
        assert(rhs->getType()->isIntegerTy());
        
        if (ConstantInt* lhs_ci = dyn_cast<ConstantInt>(lhs)) {
          lhs_v = lhs_ci->getValue();
        }
        else {
          auto addr = addrsOf(lhs, this->getFp(), this->getConf(), *ConcreteState::getModule());
          auto valOpt = this->getConf()->getStore()->lookup(addr);
          assert(valOpt.hasValue());
          auto val = valOpt.getValue();
          assert(isa<IntValue>(&*val));
          auto intVal = dyn_cast<IntValue>(&*val);
          lhs_v = intVal->getValue();
        }
        
        if (ConstantInt* rhs_ci = dyn_cast<ConstantInt>(rhs)) {
          rhs_v = rhs_ci->getValue();
        }
        else {
          auto addr = addrsOf(rhs, this->getFp(), this->getConf(), *ConcreteState::getModule());
          auto valOpt = this->getConf()->getStore()->lookup(addr);
          assert(valOpt.hasValue());
          auto val = valOpt.getValue();
          assert(isa<IntValue>(&*val));
          auto intVal = dyn_cast<IntValue>(&*val);
          rhs_v = intVal->getValue();
        }
        
        APInt result;
        if (Instruction::Add == inst->getOpcode()) {
          result = lhs_v + rhs_v;
        }
        if (Instruction::Sub == inst->getOpcode()) {
          result = lhs_v - rhs_v;
        }
        auto resultVal = IntValue::makeInt(result);
        auto destAddr = addrsOf(inst, this->getFp(), this->getConf(), *ConcreteState::getModule());
        auto newStore = this->getConf()->getStore()->copy();
        newStore->inplaceUpdate(destAddr, resultVal);
        auto newConf = ConcreteConf::makeConf(newStore, this->getConf()->getSucc(), this->getConf()->getPred());
        auto newState = ConcreteState::makeState(nextStmt, this->getFp(), newConf, this->getSp());
        return newState;
      }
      else {
        
      }
      
      ///////////////////////////
      assert(nextInst != nullptr && "next instruction is nullptr");
      auto stmt = Stmt::makeStmt(nextInst);
      auto state = makeState(stmt, getFp(), getConf(), getSp());
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
