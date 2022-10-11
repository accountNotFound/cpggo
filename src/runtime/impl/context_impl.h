#pragma once

#include <list>
#include <map>

#include "../context.h"
#include "../executor.h"
#include "../goroutine.h"
#include "util/lock_free_queue.h"
#include "util/spin_lock.h"

namespace cppgo {

template <typename T>
concept HasId = requires(T v) {
  { v.id() } -> std::same_as<size_t>;
};

class Context::Impl {
 public:
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

 public:
  Impl(Context& _this_wrapper, size_t executor_num);

 public:
  Goroutine& go(AsyncFunctionBase&& fn);
  Goroutine& current_goroutine() { return *this_thread_goroutine; }
  void start();
  void wait_until(const std::function<bool()>& pred);
  void stop();

 public:
  static thread_local Goroutine* this_thread_goroutine;

 public:
  SafeHashSet<Executor> executors;
  SafeHashSet<Goroutine> goroutines;
  LockFreeQueue<Goroutine*> runnable_queue;

 private:
  Context* _this_wrapper;
  size_t _executor_num;
};

}  // namespace cppgo
