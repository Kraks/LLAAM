//
// Created by WeiGuannan on 08/02/2017.
//
#include "AAM.h"

#ifndef LLVM_CONCRETEAAM_H
#define LLVM_CONCRETEAAM_H

//TODO Using template refactor Store/Succ/Pred

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
  
  template<class K, class V, class Less>
  class GeneralStore {
  public:
    typedef std::shared_ptr<K> Key;
    typedef std::shared_ptr<V> Val;
    typedef std::map<Key, Val, Less> StoreMap;
    
    GeneralStore() {};
    GeneralStore(StoreMap m) : m(m) {};
    virtual size_t size() const {
      return m.size();
    }
    
    Optional<GeneralStore::Val> lookup(GeneralStore::Key key) {
      auto it = m.find(key);
      if (it != m.end()) return it->second;
      return None;
    }
    
    /* Immutable update */
    GeneralStore<K,V,Less> update(GeneralStore::Key key, GeneralStore::Val val) {
      auto newMap = m;
      newMap[key] = val;
      GeneralStore<K,V,Less> newStore(newMap);
      return newStore;
    }
    
    inline bool operator==(GeneralStore& that) {
      auto pred = [] (decltype(*m.begin()) a, decltype(*m.begin()) b) {
        return *a.first == *b.first && *a.second == *b.second;
      };
      return this->m.size() == that.m.size() &&
             std::equal(this->m.begin(), this->m.end(), that.m.begin(), pred);
    }
  
  private:
    StoreMap m;
  };
  
  typedef GeneralStore<Location, AbstractValue, LocationLess> ConcreteStore;
  typedef GeneralStore<Location, Location, LocationLess> ConcreteSucc;
  typedef GeneralStore<Location, Location, LocationLess> ConcretePred;
  
  /*
  struct ConcreteStore : public Store {
  public:
    typedef std::shared_ptr<Location> Key;
    typedef std::shared_ptr<AbstractValue> Val;
    typedef std::map<Key, Val, LocationLess> StoreMap;
    
    ConcreteStore() {};
    ConcreteStore(StoreMap m) : m(m) {};
    virtual size_t size() const {
      return m.size();
    }
    
    Optional<ConcreteStore::Val> lookup(ConcreteStore::Key loc) {
      auto it = m.find(loc);
      if (it != m.end()) return it->second;
      return None;
    }
    
    // Immutable update
    ConcreteStore update(ConcreteStore::Key loc, ConcreteStore::Val val) {
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
    
  private:
    StoreMap m;
  };
  
  class ConcreteSucc : public Succ {
  public:
    typedef std::shared_ptr<Location> Key;
    typedef std::shared_ptr<Location> Val;
    typedef std::map<Key, Val, LocationLess> SuccMap;
    
    ConcreteSucc(SuccMap m) : m(m) {}
    
    Optional<ConcreteSucc::Val> lookup(ConcreteSucc::Key loc) {
      auto it = m.find(loc);
      if (it != m.end()) return it->second;
      return None;
    }
  
    // Immutable update
    ConcreteSucc update(ConcreteSucc::Key loc, ConcreteSucc::Val val) {
      auto newMap = m;
      newMap[loc] = val;
      ConcreteSucc newSucc(newMap);
      return newSucc;
    }
  
    inline bool operator==(ConcreteSucc& that) {
      auto pred = [] (decltype(*m.begin()) a, decltype(*m.begin()) b) {
        return *a.first == *b.first && *a.second == *b.second;
      };
      return this->m.size() == that.m.size() &&
             std::equal(this->m.begin(), this->m.end(), that.m.begin(), pred);
    }
    
  private:
    SuccMap m;
  };
  
  typedef ConcreteSucc ConcretePred;
*/
  
  /******** Static initialization ********/
  
  unsigned long long ConcreteHeapAddr::id = 0;
  unsigned long long ConcreteStackPtr::id = 0;
}

#endif //LLVM_CONCRETEAAM_H
