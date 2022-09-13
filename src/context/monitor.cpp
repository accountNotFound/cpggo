#include "monitor.h"

#include "context.h"
#include "task.h"

// #define USE_DEBUG
#include "common/log.h"

namespace cppgo {

SpinLock Monitor::cls_;
Monitor::Id Monitor::cls_id_ = 0;

Monitor::Monitor(Context* ctx) : ctx_(ctx) {
  std::unique_lock guard(cls_);
  id_ = cls_id_++;
}

AsyncFunction<void> Monitor::enter() {
  auto current = ctx_->this_running_task();
  while (true) {
    self_.lock();
    if (!busy_flag_) {
      current->callback_ = [this, current]() -> bool {
        DEBUG("task {%u} lock success", current->id());
        this->busy_flag_ = true;
        this->self_.unlock();
        return true;
      };
      co_return;
    } else {
      current->callback_ = [this, current]() -> bool {
        DEBUG("task {%u} lock failed, [running] -> [blocked]", current->id());
        {
          std::unique_lock guard(this->ctx_->self_);
          this->blocked_set_.insert(current);
          current->change_status_(Task::Status::RNUNING, Task::Status::BLOCKED);
        }
        this->self_.unlock();
        return false;
      };
      co_await std::suspend_always{};
    }
  }
}

AsyncFunction<void> Monitor::wait() {
  co_await block();
  co_await enter();
}

AsyncFunction<void> Monitor::block() {
  auto current = ctx_->this_running_task();
  self_.lock();
  current->callback_ = [this, current]() -> bool {
    if (busy_flag_) {
      busy_flag_ = false;
      this->notify_one_with_lock_();
    }
    DEBUG("task {%u} wait, [running] -> [blocked]", current->id());
    {
      std::unique_lock guard(this->ctx_->self_);
      this->blocked_set_.insert(current);
      current->change_status_(Task::Status::RNUNING, Task::Status::BLOCKED);
    }
    this->self_.unlock();
    return false;
  };
  co_await std::suspend_always{};
}

void Monitor::notify_one() {
  std::unique_lock guard(self_);
  notify_one_with_lock_();
}

void Monitor::exit() {
  auto current = ctx_->this_running_task();
  {
    std::unique_lock guard(self_);
    busy_flag_ = false;
    notify_one_with_lock_();
    DEBUG("task {%u} unlock", current->id());
  }
}

void Monitor::notify_one_with_lock_() {
  auto current = ctx_->this_running_task();
  {
    std::unique_lock guard(ctx_->self_);
    if (blocked_set_.empty() && busy_flag_) {
      RAISE("invlalid notification");
    }
    DEBUG("len(monitor_blocked_set)=%d", blocked_set_.size());
    if (!blocked_set_.empty()) {
      auto next = *blocked_set_.begin();
      DEBUG("task {%u} is notified, [blocked] -> [runnable]", next->id());
      blocked_set_.erase(next);
      next->change_status_(Task::Status::BLOCKED, Task::Status::RUNNABLE);
    }
  }
}

}  // namespace cppgo
