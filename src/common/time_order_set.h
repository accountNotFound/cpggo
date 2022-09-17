#pragma once

#include <list>
#include <unordered_map>

namespace cppgo {

template <typename T>
class TimeOrderSet {
 public:
  using Iterator = std::list<T>::iterator;

 public:
  void insert(const T& val) {
    list_.push_back(val);
    v2it_[val] = --list_.end();
  }
  void erase(const T& val) {
    list_.erase(v2it_.at(val));
    v2it_.erase(val);
  }
  Iterator begin() { return list_.begin(); }
  Iterator end() { return list_.end(); }
  size_t count(const T& val) { return v2it_.count(val); }
  bool empty() { return list_.empty(); }

 private:
  std::unordered_map<T, Iterator> v2it_;
  std::list<T> list_;
};

}  // namespace cppgo
