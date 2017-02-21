//
// Created by WeiGuannan on 08/02/2017.
//

#include "AAM.h"

#ifndef LLVM_CONCRETEAAM_H
#define LLVM_CONCRETEAAM_H

/**
 * Supported instructions:
 * StoreInst, AllocaInst, LoadInst,
 * ReturnInst, CallInst, CallInst @malloc, CallInst @free,
 * ICmpInst, SExtInst, ZExtInst, BitCastInst,
 * BranchInst, GetElementPtrInst, TruncInst
 */

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
  
  std::shared_ptr<IntValue> primOp(unsigned op, Value* lhs, Value* rhs,
                                   std::shared_ptr<FrameAddr> fp,
                                   std::shared_ptr<ConcreteConf> conf);
  
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
      LLVMContext& C = getModule()->getContext();
      Instruction* inst = getControl()->getInst();
      Instruction* nextInst = getSyntacticNextInst(inst);
      auto nextStmt = Stmt::makeStmt(nextInst);
      errs() << "\nCurrent state[" << myId << "] ";
      inst->dump();
  
      size_t opNum = inst->getNumOperands();
      errs() << "op num: " << opNum << "\n";
      
      if (isa<ReturnInst>(inst)) {
        ReturnInst* returnInst = dyn_cast<ReturnInst>(inst);
        auto fp = this->getFp();
        
        //errs() << "op nums: " << opNum << "\n";
        if (*fp == *ConcreteStackAddr::initFp()) {
          if (opNum > 0) {
            auto ret = returnInst->getOperand(0);
            auto rval = evalAtom(ret, this->getFp(), this->getConf(), *ConcreteState::getModule());
            errs() << "Return: ";
            rval->print();
            errs() << "\n";
          }
          else { errs() << "Return: Void\n"; }
          return this->copy();
        }
  
        auto contValOpt = this->getConf()->getStore()->lookup(this->getFp());
        assert(contValOpt.hasValue());
        auto contVal = contValOpt.getValue();
        assert(isa<Cont>(&*contVal));
        auto cont = dyn_cast<Cont>(&*contVal);
        auto newStore = this->getConf()->getStore()->copy();
        
        if (opNum > 0) {
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
      else if (isa<CallInst>(inst)) {
        CallInst* callInst = dyn_cast<CallInst>(inst);
        Value* f = callInst->getCalledFunction();
        Function* function = nullptr;
        
        if (f) {
          function = dyn_cast<Function>(f);
        }
        else {
          size_t fpos = callInst->getNumOperands() - 1;
          Value* op0 = callInst->getOperand(fpos);
          auto faddr = BindAddr::makeBindAddr(op0, this->getFp());
          auto valOpt = this->getConf()->getStore()->lookup(faddr);
          assert(valOpt.hasValue());
          auto val = valOpt.getValue();
          assert(isa<FuncValue>(&*val));
          auto fval = dyn_cast<FuncValue>(&*val);
          function = fval->getFunction();
        }
        
        std::string fname = function->getName();
        auto actualArgs = callInst->arg_operands();
        auto& formalArgs = function->getArgumentList();
        
        //errs() << "function name: " << fname << "\n";
        
        if (fname == "malloc") {
          auto mallocSize = callInst->getOperand(0);
          int64_t nMalloc = 0;
          if (ConstantInt* mallocSizeCI = dyn_cast<ConstantInt>(mallocSize)) {
            nMalloc = mallocSizeCI->getSExtValue();
          }
          else {
            auto sizeVal = evalAtom(mallocSize, getFp(), getConf(), *ConcreteState::getModule());
            assert(isa<IntValue>(&*sizeVal));
            nMalloc = dyn_cast<IntValue>(&*sizeVal)->getValue().getSExtValue();
          }
          errs() << "malloc size: " << nMalloc << "\n";
  
          //TODO: can malloc a negative number?
          assert(nMalloc > 0);
          auto addrs = ConcreteHeapAddr::allocate(nMalloc + 1);
          
          auto store = this->getConf()->getStore();
          auto succ = this->getConf()->getSucc();
          auto pred = this->getConf()->getPred();
  
          auto bot = BotValue::getInstance();
          auto newStore = store->copy();
          for (auto& addr : *addrs) {
            newStore->inplaceUpdate(addr, bot);
          }
          assert(newStore->size() == (store->size() + addrs->size()));
          
          auto destAddr = BindAddr::makeBindAddr(inst, this->getFp());
          auto locVal = LocationValue::makeLocationValue(addrs->front());
          newStore->inplaceUpdate(destAddr, locVal);
          
          auto newSucc = succ->copy();
          for (unsigned long i = 0; i < addrs->size()-1; i++) {
            newSucc->inplaceUpdate(addrs->at(i), addrs->at(i+1));
          }
          assert(newSucc->size() == (succ->size() + addrs->size() - 1));
          
          auto newPred = pred->copy();
          for (unsigned long i = addrs->size()-1; i > 0; i--) {
            newPred->inplaceUpdate(addrs->at(i), addrs->at(i-1));
          }
          assert(newPred->size() == (pred->size() + addrs->size() - 1));
  
          auto newConf = ConcreteConf::makeConf(newStore, newSucc, newPred);
          auto newState = ConcreteState::makeState(nextStmt, getFp(), newConf, getSp());
          return newState;
        }
        else if (fname == "free") {
          auto v = callInst->getOperand(0);
          auto vAddr = BindAddr::makeBindAddr(v, getFp());
          auto locValOpt = getConf()->getStore()->lookup(vAddr);
          
          auto newConf = getConf()->copy();
          if (locValOpt.hasValue()) {
            //TODO: recursively deallocation?
            auto locVal = locValOpt.getValue();
            auto loc = dyn_cast<LocationValue>(&*locVal);
            newConf->inplaceRemove(loc->getLocation());
          }
          else {
            assert(false && "Invalid pointer");
          }
          
          auto newState = ConcreteState::makeState(nextStmt, getFp(), newConf, getSp());
          return newState;
        }
        else {
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
        errs() << "op(0): ";
        op0->print(errs());
        errs() << "\n";
        */
        
        auto fromAddr = addrsOf(op0, this->getFp(), this->getConf(), *ConcreteState::getModule());
        auto valOpt = this->getConf()->getStore()->lookup(fromAddr);
        assert(valOpt.hasValue());
        auto val = valOpt.getValue();
        
        if (loadInst->getType()->isIntegerTy()) {
          assert(isa<IntValue>(*val));
        }
        else if (loadInst->getType()->isPointerTy()) {
          assert(isa<LocationValue>(*val) || isa<FuncValue>(*val));
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
          errs() << "op0 func? " << (isa<Function>(op0)) << "\n";
          if (isa<Function>(op0)) {
            // Function pointer
            //errs() << "op0_ty func: " << op0_ty->isFunctionTy() << "\n";
            Function* f = getModule()->getFunction(op0->getName());
            assert(f && "can not get function");
            //Function* f = dyn_cast<Function>(op0);
            auto val = FuncValue::makeFuncValue(f);
            newStore->inplaceUpdate(destAddr, val);
          }
          else {
            auto fromAddr = BindAddr::makeBindAddr(op0, this->getFp());
            auto fromValOpt = this->getConf()->getStore()->lookup(fromAddr);
            assert(fromValOpt.hasValue());
            auto fromVal = fromValOpt.getValue();
            newStore->inplaceUpdate(destAddr, fromVal);
          }
        }
        
        auto newConf = ConcreteConf::makeConf(newStore, this->getConf()->getSucc(), this->getConf()->getPred());
        auto newState = ConcreteState::makeState(nextStmt, this->getFp(), newConf, this->getSp());
        return newState;
      }
      else if (isa<AllocaInst>(inst)) {
        AllocaInst* allocaInst = dyn_cast<AllocaInst>(inst);
        Type* allocaType = allocaInst->getAllocatedType();
        
        uint64_t totalByteSize = ConcreteState::getModule()->getDataLayout().getTypeAllocSize(allocaType);
        assert(totalByteSize > 0);
        errs() << "totalByteSize: " << totalByteSize << "\n";
        uint64_t nAlloc = totalByteSize;
        uint64_t step = totalByteSize;
        
        if (auto* allocaArrayType = dyn_cast<ArrayType>(allocaType)) {
          Type* eleType = allocaArrayType->getElementType();
          errs() << "eleType: ";
          eleType->print(errs());
          uint64_t eleNum = allocaArrayType->getNumElements();
          errs() << " num: " << eleNum << "\n";
          
          step = ConcreteState::getModule()->getDataLayout().getTypeAllocSize(eleType);
          assert(step * eleNum == nAlloc);
        }
        else if (auto* allocaStructType = dyn_cast<StructType>(allocaType)) {
          size_t nEle = allocaStructType->getStructNumElements();
          errs() << "num ele: " << nEle << "\n";
        }
        
        /*
        errs() << "allocaInst array alloc? " << allocaInst->isArrayAllocation() << "\n";
        */
        errs() << "allocaInst alloca size: "  << nAlloc << "\n";
        
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
        
        //TODO test it
        auto newPred = pred->copy();
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
      else if (isa<GetElementPtrInst>(inst)) {
        GetElementPtrInst* ptrInst = dyn_cast<GetElementPtrInst>(inst);
        
        auto srcObj = ptrInst->getOperand(0);
        auto srcAddr = BindAddr::makeBindAddr(srcObj, this->getFp());
        auto srcValOpt = this->getConf()->getStore()->lookup(srcAddr);
        assert(srcValOpt.hasValue());
        auto srcVal = srcValOpt.getValue();
        assert(isa<LocationValue>(&*srcVal));
        auto src = dyn_cast<LocationValue>(&*srcVal);
  
        int64_t n = 0;
        auto ty = srcObj->getType();
        
        for (int i = 1; i < opNum; i++) {
          Value* offset = ptrInst->getOperand(i);
          int64_t offset_v = 0;
          uint64_t ty_size = 0;
          if (ConstantInt* offset_ci = dyn_cast<ConstantInt>(offset)) {
            offset_v = offset_ci->getValue().getSExtValue();
            if (ty->isPointerTy()) {
              ty = ty->getPointerElementType();
            }
            ty_size = ConcreteState::getModule()->getDataLayout().getTypeAllocSize(ty);
            errs() << "ty size: " << ty_size << " offset: " << offset_v << "\n";
            n += (ty_size * offset_v);
          }
          else {
            //TODO: Now assume all offsets are constant int, need to handle variable
          }
          
          if (ty->isArrayTy()) {
            ty = ty->getArrayElementType();
          }
          else if (ty->isStructTy()) {
            assert(offset_v > 0);
            ty = ty->getStructElementType(offset_v);
          }
        }
        
        errs() << "n: " << n << "\n";
        auto addr = src->getLocation();
        auto succ = this->getConf()->getSucc();
        auto pred = this->getConf()->getPred();
        if (n >= 0) {
          for (int i = 0; i < n; i++) {
            auto addrOpt = succ->lookup(addr);
            addr = addrOpt.getValue();
          }
        }
        else {
          for (int i = 0; i > n; i--) {
            auto addrOpt = pred->lookup(addr);
            addr = addrOpt.getValue();
          }
        }
        
        auto newStore = this->getConf()->getStore()->copy();
        auto destAddr = BindAddr::makeBindAddr(ptrInst, this->getFp());
        auto locVal = LocationValue::makeLocationValue(addr);
        newStore->inplaceUpdate(destAddr, locVal);
        
        auto newConf = ConcreteConf::makeConf(newStore, getConf()->getSucc(), getConf()->getPred());
        auto newState = ConcreteState::makeState(nextStmt, getFp(), newConf, getSp());
        return newState;
      }
      else if (Instruction::Add == inst->getOpcode() ||
               Instruction::Sub == inst->getOpcode() ||
               Instruction::Mul == inst->getOpcode()) {
        Value* lhs = inst->getOperand(0);
        Value* rhs = inst->getOperand(1);
        auto resultVal = primOp(inst->getOpcode(), lhs, rhs, this->getFp(), this->getConf());
        auto destAddr = addrsOf(inst, this->getFp(), this->getConf(), *ConcreteState::getModule());
        auto newStore = this->getConf()->getStore()->copy();
        newStore->inplaceUpdate(destAddr, resultVal);
        auto newConf = ConcreteConf::makeConf(newStore, this->getConf()->getSucc(), this->getConf()->getPred());
        auto newState = ConcreteState::makeState(nextStmt, this->getFp(), newConf, this->getSp());
        return newState;
      }
      else if (isa<ICmpInst>(inst)) {
        ICmpInst* cmpInst = dyn_cast<ICmpInst>(inst);
        auto lhs = evalAtom(cmpInst->getOperand(0), getFp(), getConf(), *getModule());
        assert(isa<IntValue>(*lhs));
        auto rhs = evalAtom(cmpInst->getOperand(1), getFp(), getConf(), *getModule());
        assert(isa<IntValue>(*rhs));
        
        APInt& lhs_v = dyn_cast<IntValue>(&*lhs)->getValue();
        APInt& rhs_v = dyn_cast<IntValue>(&*rhs)->getValue();
        
        CmpInst::Predicate pred = cmpInst->getPredicate();
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
        
        ConstantInt* result;
        if (res) {
          result = ConstantInt::getTrue(C);
        }
        else {
          result = ConstantInt::getFalse(C);
        }
        
        errs() << "result: ";
        result->print(errs());
        errs() << "\n";
        auto val = IntValue::makeInt(result->getValue());
        auto destAddr = addrsOf(cmpInst, getFp(), getConf(), *getModule());
        auto newStore = getConf()->getStore()->copy();
        newStore->inplaceUpdate(destAddr, val);
        auto newConf = ConcreteConf::makeConf(newStore, getConf()->getSucc(), getConf()->getPred());
        auto newState = ConcreteState::makeState(nextStmt, getFp(), newConf, getSp());
        return newState;
      }
      else if (isa<BranchInst>(inst)) {
        BranchInst* branchInst = dyn_cast<BranchInst>(inst);
        Instruction* nextSemanticInst;
        
        if (opNum == 1) {
          errs() << "Unconditional branch\n";
          BasicBlock* targetBlock = branchInst->getSuccessor(0);
          nextSemanticInst = getEntry(*targetBlock);
        }
        else if (opNum == 3) {
          errs() << "Conditional branch\n";
          Value* cnd = branchInst->getOperand(0);
          BasicBlock* thnBlock = branchInst->getSuccessor(0);
          BasicBlock* elsBlock = branchInst->getSuccessor(1);
          
          auto cndVal = evalAtom(cnd, getFp(), getConf(), *getModule());
          assert(isa<IntValue>(&*cndVal));
          APInt& cndInt = dyn_cast<IntValue>(&*cndVal)->getValue();
          assert(cndInt.getBitWidth() == 1);
          bool b = cndInt.getBoolValue();
          errs() << "b: " << b << "\n";
          
          if (b) {
            nextSemanticInst = getEntry(*thnBlock);
          }
          else {
            nextSemanticInst = getEntry(*elsBlock);
          }
        }
        auto nextSemanticStmt = Stmt::makeStmt(nextSemanticInst);
        auto newState = ConcreteState::makeState(nextSemanticStmt, getFp(), getConf(), getSp());
        return newState;
      }
      else if (isa<BitCastInst>(inst)) {
        //TODO: For now, BitCastInst is doing nothing.
        BitCastInst* bitCastInst = dyn_cast<BitCastInst>(inst);
        Type* destType = bitCastInst->getDestTy();
        Type* srcType = bitCastInst->getSrcTy();
        Value* src = bitCastInst->getOperand(0);
        
        errs() << "src: ";
        src->print(errs());
        errs() << "\n";
        
        auto locVal = evalAtom(src, getFp(), getConf(), *getModule());
        assert(isa<LocationValue>(&*locVal));
        auto destAddr = BindAddr::makeBindAddr(bitCastInst, getFp());
        auto newStore = getConf()->getStore()->copy();
        newStore->inplaceUpdate(destAddr, locVal);
        auto newConf = ConcreteConf::makeConf(newStore, getConf()->getSucc(), getConf()->getSucc());
        auto newState = ConcreteState::makeState(nextStmt, getFp(), newConf, getSp());
        return newState;
      }
      else if (isa<SExtInst>(inst)) {
        SExtInst* sExtInst = dyn_cast<SExtInst>(inst);
        Value* op0 = sExtInst->getOperand(0);
        
        Type* destType = sExtInst->getDestTy();
        auto val = evalAtom(op0, getFp(), getConf(), *getModule());
        assert(isa<IntValue>(&*val));
        APInt v = dyn_cast<IntValue>(&*val)->getValue().sext(destType->getIntegerBitWidth());
        auto newVal = IntValue::makeInt(v);
  
        auto destAddr = addrsOf(sExtInst, getFp(), getConf(), *getModule());
        auto newStore = getConf()->getStore()->copy();
        newStore->inplaceUpdate(destAddr, newVal);
        auto newConf = ConcreteConf::makeConf(newStore, getConf()->getSucc(), getConf()->getPred());
        auto newState = ConcreteState::makeState(nextStmt, getFp(), newConf, getSp());
        return newState;
      }
      else if (isa<ZExtInst>(inst)) {
        ZExtInst* zExtInst = dyn_cast<ZExtInst>(inst);
        Value* op0 = zExtInst->getOperand(0);
        
        Type* destType = zExtInst->getDestTy();
        auto val = evalAtom(op0, getFp(), getConf(), *getModule());
        assert(isa<IntValue>(&*val));
        APInt v = dyn_cast<IntValue>(&*val)->getValue().zext(destType->getIntegerBitWidth());
        auto newVal = IntValue::makeInt(v);
        
        auto destAddr = addrsOf(zExtInst, getFp(), getConf(), *getModule());
        auto newStore = getConf()->getStore()->copy();
        newStore->inplaceUpdate(destAddr, newVal);
        auto newConf = ConcreteConf::makeConf(newStore, getConf()->getSucc(), getConf()->getPred());
        auto newState = ConcreteState::makeState(nextStmt, getFp(), newConf, getSp());
        return newState;
      }
      else if (isa<TruncInst>(inst)) {
        TruncInst* truncInst = dyn_cast<TruncInst>(inst);
        Value* op0 = truncInst->getOperand(0);
        Type* destType = truncInst->getDestTy();
        
        auto val = evalAtom(op0, getFp(), getConf(), *getModule());
        assert(isa<IntValue>(&*val));
        APInt v = dyn_cast<IntValue>(&*val)->getValue().trunc(destType->getIntegerBitWidth());
        auto newVal = IntValue::makeInt(v);
        
        auto destAddr = addrsOf(truncInst, getFp(), getConf(), *getModule());
        auto newStore = getConf()->getStore()->copy();
        newStore->inplaceUpdate(destAddr, newVal);
        auto newConf = ConcreteConf::makeConf(newStore, getConf()->getSucc(), getConf()->getPred());
        auto newState = ConcreteState::makeState(nextStmt, getFp(), newConf, getSp());
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
