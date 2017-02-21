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
  
  class Measure {};
  
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
    
  private:
    std::set<ValPtr, Less> set;
  };
   
  typedef D<AbstractValue, AbstractValueLess> AbsD;
}

#endif //LLVM_ABSTRACTAAM_H
