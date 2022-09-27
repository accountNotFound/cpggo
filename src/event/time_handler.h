#pragma once

#include <chrono>
#include <queue>
#include <unordered_map>

#include "base.h"
#include "common/allocator.h"
#include "common/spinlock.h"
#include "context/context.h"
#include "coroutine/coroutine.h"
#include "synchronize/channel.h"

namespace cppgo::time {

struct TimeFd : public Fd {
 public:
  struct Greater {
    size_t operator()(const TimeFd& a, const TimeFd& b) const {
      return a.expired_time > b.expired_time;
    }
  };

 public:
  TimeFd(unsigned long long millisecond);

 public:
  static SpinLock cls_;
  static size_t cls_id_;

  std::chrono::milliseconds expired_time;
};

// TimeFd create_timefd(unsigned long long millisecond) {
//   return TimeFd(millisecond);
// }

class TimeHandler : public EventHandlerBase {
 public:
  TimeHandler(Context* ctx) : EventHandlerBase(ctx) {}

  // ONESHOT event is always added
  void add(const Fd& fd, Event listen_on);
  AsyncFunction<void> signal(const Fd& fd);
  void evloop();

 private:
  SpinLock self_;
  std::priority_queue<TimeFd, std::vector<TimeFd>, TimeFd::Greater> time_queue_;
  Allocator<Channel<bool>> chan_mgr_;
  std::unordered_map<Fd, Channel<bool>*, Fd::Hash, Fd::Equal> chan_map_;
};

}  // namespace cppgo::time
