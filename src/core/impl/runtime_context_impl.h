#pragma once

#include <list>
#include <unordered_map>

#include "runtime.h"
#include "util/lock_free_queue.h"
#include "util/spin_lock.h"

namespace cppgo {

namespace __detail {
template <typename T>
concept HasId = requires(T v) {
  { v.id() } -> std::same_as<size_t>;
};

// a thread-safe (on insert and erase) and no-rehash-affected hash set
template <HasId T>
class SafeHashSet {
 public:
  using Iter = std::list<T>::iterator;

 public:
  template <typename... Args>
  std::pair<Iter, bool> emplace(Args&&... args) {
    std::unique_lock guard(_mtx);
    Iter it = _data.emplace(_data.end(), std::forward<Args>(args)...);
    _index[it->id()] = it;
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

  Iter begin() { return _data.begin(); }
  Iter end() { return _data.end(); }

 private:
  SpinLock _mtx;
  std::unordered_map<size_t, Iter> _index;
  std::list<T> _data;
};
}  // namespace __detail

class Context::Impl {
  friend class Executor::Impl;
  friend class Goroutine::Impl;

 public:
  Impl(Context& _this_wrapper, size_t executor_num);

 public:
  Goroutine& go(AsyncFunctionBase&& fn);
  Goroutine& current_goroutine();
  void start();
  void wait_until(const std::function<bool()>& pred);

 private:
  Context* _this() { return _this_wrapper; }

 private:
  static thread_local Goroutine* _this_thread_goroutine;

 private:
  Context* _this_wrapper;
  size_t _executor_num;
  __detail::SafeHashSet<Executor> _executors;
  __detail::SafeHashSet<Goroutine> _goroutines;
  LockFreeQueue<Goroutine*> _runnable_queue;
  LockFreeQueue<Goroutine*> _done_queue;
};

}  // namespace cppgo
