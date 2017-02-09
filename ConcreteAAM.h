#include "AAM.h"

#ifndef LLVM_ConcreteAAM_H
#define LLVM_ConcreteAAM_H

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
  
  struct ConcreteStore : public Store {
    typedef std::map<std::shared_ptr<Location>,
    std::shared_ptr<AbstractValue>,
    LocationLess> StoreMap;
    StoreMap m;
  public:
    ConcreteStore() {};
    ConcreteStore(StoreMap m) : m(m) {};
    virtual size_t size() const {
      return m.size();
    }
    
    Optional<AbstractValue*> lookup(std::shared_ptr<Location> loc) {
      auto it = m.find(loc);
      if (it != m.end()) return it->second.get();
      return None;
    }
    
    /* Immutable update */
    ConcreteStore update(std::shared_ptr<Location> loc, std::shared_ptr<AbstractValue> val) {
      auto newMap = m;
      newMap[loc] = val;
      ConcreteStore newStore(newMap);
      return newStore;
    }
    
    inline bool operator==(ConcreteStore& that) {
      auto pred = [] (decltype(*m.begin()) a, decltype(*m.begin()) b) {
        return *a.first == *b.first && *a.second == *b.second;
      };
      return this->m.size() == that.m.size() &&
             std::equal(this->m.begin(), this->m.end(), that.m.begin(), pred);
    }
  };
  
  class ConcreteSucc : public Succ {
    typedef std::map<std::shared_ptr<Location>,
                     std::shared_ptr<Location>,
                     LocationLess> SuccMap;
    SuccMap m;
  public:
    ConcreteSucc(SuccMap m) : m(m) {}
    
  };
  
  /******** Static initialization ********/
  
  unsigned long long ConcreteHeapAddr::id = 0;
  unsigned long long ConcreteStackPtr::id = 0;
}

#endif //LLVM_ConcreteAAM_H
