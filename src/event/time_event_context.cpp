#include "time_event_context.h"

#include <thread>

#define USE_DEBUG
#include "common/log.h"

namespace cppgo::time {

SpinLock TimeFd::cls_;
size_t TimeFd::cls_id_;

std::chrono::milliseconds current_millisec() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch());
}

TimeFd::TimeFd(unsigned long long millisecond) {
  std::unique_lock guard(cls_);
  uid = cls_id_++;
  expired_time = current_millisec() + std::chrono::milliseconds(millisecond);
}

void TimeEventContext::add(const Fd& fd, Event listen_on) {
  std::unique_lock guard(self_);
  auto signal = chan_mgr_.create(this, 1);
  chan_map_[fd] = signal;
  time_queue_.push(dynamic_cast<const TimeFd&>(fd));
}

AsyncFunction<void> TimeEventContext::signal(const Fd& fd) {
  Channel<bool>* chan = nullptr;
  {
    std::unique_lock guard(self_);
    chan = chan_map_.at(fd);
  }
  co_await chan->recv();
  {
    // ONESHOT
    std::unique_lock guard(self_);
    chan_mgr_.destroy(chan_map_.at(fd));
    chan_map_.erase(fd);
  }
}

void TimeEventContext::evloop() {
  while (!done()) {
    self_.lock();
    if (!time_queue_.empty() &&
        current_millisec() >= time_queue_.top().expired_time) {
      auto fd = time_queue_.top();
      time_queue_.pop();
      self_.unlock();

      // assert true
      chan_map_.at(fd)->send_noblock(true);
    } else {
      self_.unlock();
      // a simple wait strategy
      // DEBUG("event waiting", 0);
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }
}

}  // namespace cppgo::time
