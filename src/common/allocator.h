#pragma once

#include <unordered_set>

#include "error.h"

namespace cppgo {

template <typename T>
class Allocator {
 public:
  using Iterator = std::unordered_set<T*>::iterator;

 public:
  template <typename... Args>
  T* create(Args&&... args) {
    T* ptr = new T(std::forward<Args>(args)...);
    objects_.insert(ptr);
    return ptr;
  }

  T* add(T* ptr) {
    objects_.insert(ptr);
    return ptr;
  }

  void destroy(T* ptr) {
    if (!objects_.count(ptr)) {
      RAISE("object is not found");
    }
    objects_.erase(ptr);
    delete ptr;
  }

  Iterator begin() { return objects_.begin(); }
  Iterator end() { return objects_.end(); }

 private:
  std::unordered_set<T*> objects_;
};

}  // namespace cppgo
