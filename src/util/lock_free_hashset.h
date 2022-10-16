#pragma once

#include <list>
#include <unordered_map>

#include "spin_lock.h"

namespace cppgo {

// a thread-safe (on insert and erase) and no-rehash-affected hash set
template <typename T>
requires std::convertible_to<T, size_t>
class LockFreeHashset {
 public:
  using Iter = std::list<T>::iterator;

 public:
  template <typename... Args>
  std::pair<Iter, bool> emplace(Args&&... args) {
    std::unique_lock guard(_mtx);
    Iter it = _data.emplace(_data.end(), std::forward<Args>(args)...);
    _index[size_t(*it)] = it;
    return {it, true};
  }

  // erase if exists
  void erase(size_t id) {
    std::unique_lock guard(_mtx);
    if (_index.count(id)) {
      _data.erase(_index[id]);
      _index.erase(id);
    }
  }

  Iter find(size_t id) {
    std::unique_lock guard(_mtx);
    return _index.at(id);
  }

  T& at(size_t id) { return *find(id); }

  Iter begin() { return _data.begin(); }
  Iter end() { return _data.end(); }

 private:
  SpinLock _mtx;
  std::unordered_map<size_t, Iter> _index;
  std::list<T> _data;
};

}  // namespace cppgo
