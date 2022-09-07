#pragma once

#include <atomic>
#include <mutex>
#include <thread>

namespace cppgo {

class SpinLock {
 public:
  SpinLock() : flag_(false) {}
  SpinLock(SpinLock&) = delete;

  void lock() {
    bool expected = false;
    while (!flag_.compare_exchange_weak(expected, true)) {
      expected = false;
    }
  }

  void unlock() { flag_.store(false); }

 private:
  std::atomic<bool> flag_;
};

}  // namespace cppgo
