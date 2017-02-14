//
// Created by WeiGuannan on 08/02/2017.
//
#include "AAM.h"

#ifndef LLVM_CONCRETEAAM_H
#define LLVM_CONCRETEAAM_H

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
    
    virtual bool equalTo(const Location& that) const override {
      if (!isa<ConcreteStackPtr>(&that))
        return false;
      auto* newThat = dyn_cast<ConcreteStackPtr>(&that);
      return newThat->myId == this->myId;
    }
    
    virtual size_t hashValue() const override {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(myId));
      seed =hash_combine(seed, hash_value("ConcreteStackAddr"));
      return seed;
    }
  
  private:
    unsigned long long myId;
    static unsigned long long id;
  };
  
  typedef ConcreteStackPtr ConcreteFramePtr;
  
  typedef Store<Location, AbstractValue, LocationLess> ConcreteStore;
  
  typedef Store<Location, Location, LocationLess> ConcreteSucc;
  
  typedef Store<Location, Location, LocationLess> ConcretePred;
  
  class DummyMeasure {
  public:
    size_t hashValue() { return 0; }
    inline bool operator==(DummyMeasure& that) { return true; }
  };
  typedef Conf<ConcreteStore, ConcreteSucc, ConcretePred, DummyMeasure> ConcreteConf;
  
  const static std::shared_ptr<FramePtr> initFp = std::make_shared<ConcreteFramePtr>();
  
  std::shared_ptr<ConcreteStore> getInitStore(Module& M);
  std::shared_ptr<ConcreteConf>  getInitConf(Module& M);
  
  std::shared_ptr<AbstractValue> evalAtom(ConstantInt* i,
                                          std::shared_ptr<FramePtr> fp,
                                          std::shared_ptr<ConcreteConf> conf,
                                          Module& M);
  std::shared_ptr<AbstractValue> evalAtom(Value* val,
                                          std::shared_ptr<FramePtr> fp,
                                          std::shared_ptr<ConcreteConf> conf,
                                          Module& M);
  
  std::shared_ptr<Location> addrsOf(std::string var,
                                    std::shared_ptr<FramePtr> fp,
                                    std::shared_ptr<ConcreteConf> conf,
                                    Module& M);
  std::shared_ptr<Location> addrsOf(Value* lhs,
                                    std::shared_ptr<FramePtr> fp,
                                    std::shared_ptr<ConcreteConf> conf,
                                    Module& M);
  
  class ConcreteState : public State<Stmt, FramePtr, ConcreteConf, StackPtr> {
  public:
    typedef std::shared_ptr<ConcreteState> StatePtrType;
    
    ConcreteState(CPtrType c, EPtrType e, SPtrType s, KPtrType k) :
      State(c, e, s, k) { };
    
    StatePtrType next() {
      // TODO: implement the real next()
      Instruction* inst = getControl()->getInst();
      Instruction* nextInst = getSyntacticNextInst(inst);
      
      if (isa<CallInst>(inst)) {
        CallInst* callInst = dyn_cast<CallInst>(inst);
        Function* function = callInst->getFunction();
        std::string fname = function->getName();
        errs() << "function name: " << fname << "\n";
        //TODO: malloc/free
      }
      else if (isa<ReturnInst>(inst)) {
        ReturnInst* returnInst = dyn_cast<ReturnInst>(inst);
      }
      else if (isa<LoadInst>(inst)) {
        LoadInst* loadInst = dyn_cast<LoadInst>(inst);
      }
      else if (isa<StoreInst>(inst)) {
        StoreInst* storeInst = dyn_cast<StoreInst>(inst);
      }
      else if (isa<AllocaInst>(inst)) {
        AllocaInst* allocaInst = dyn_cast<AllocaInst>(inst);
      }
      else if (isa<BranchInst>(inst)) {
        BranchInst* branchInst = dyn_cast<BranchInst>(inst);
        
      }
      else {
        
      }
      
      auto stmt = Stmt::makeStmt(nextInst);
      auto state = makeState(stmt, getEnv(), getStore(), getCont());
      return state;
    }
    
    static StatePtrType inject(Module& M, std::string mainFuncName) {
      std::shared_ptr<ConcreteConf> initConf = getInitConf(M);
      Function* main = M.getFunction(mainFuncName);
      Instruction* entry = getEntry(*main);
      std::shared_ptr<Stmt> initStmt = std::make_shared<Stmt>(entry);
      std::shared_ptr<ConcreteState> initState = makeState(initStmt, ConcreteAAM::initFp, initConf, ConcreteAAM::initFp);
      return initState;
    }
    
    static StatePtrType makeState(CPtrType c, EPtrType e, SPtrType s, KPtrType k) {
      std::shared_ptr<ConcreteState> state = std::make_shared<ConcreteState>(c, e, s, k);
      return state;
    }
  };
  
}

#endif //LLVM_CONCRETEAAM_H
