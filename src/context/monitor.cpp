#include "context.h"

// #define USE_DEBUG
#include "common/log.h"

namespace cppgo {

AsyncFunction<void> Monitor::enter() {
  while (true) {
    auto current = ctx_->this_running_task();
    {
      std::unique_lock guard(mtx_);
      if (idle_) {
        idle_ = false;
        ASSERT(current->callback_ == nullptr, "callback is not empty");
        DEBUG("task {%u} lock, [running] -> [running]", current->id());
        co_return;
      }
    }
    current->callback_ = [this, &current]() {
      this->blocked_set_.insert(current);
      ctx_->blocked_set_.insert(current);
      current->status_ = Task::Status::BLOCKED;
      DEBUG("task {%u} lock failed, [running] -> [blocked]", current->id());
    };
    co_await std::suspend_always{};
  }
}

AsyncFunction<void> Monitor::exit() {
  auto current = ctx_->this_running_task();
  exit_nowait();
  current->callback_ = [this, current]() {
    ctx_->runnable_set_.insert(current);
    current->status_ = Task::Status::RUNNABLE;
    DEBUG("task {%u} lock failed, [running] -> [runnable]", current->id());
  };
  co_await std::suspend_always{};
}

AsyncFunction<void> Monitor::wait() {
  ASSERT(idle_ == false, "exit monitor without lock");
  auto current = ctx_->this_running_task();
  {
    std::unique_lock guard(mtx_);
    idle_ = true;
    notify_one();  // notify one immediately
  }
  // set current task's callback
  current->callback_ = [this, &current]() {
    this->blocked_set_.insert(current);
    ctx_->blocked_set_.insert(current);
    current->status_ = Task::Status::BLOCKED;
    DEBUG("task {%u} wait, [running] -> [blocked]", current->id());
  };
  co_await std::suspend_always{};
  co_await enter();
}

void Monitor::notify_one() {
  // monitor's blocked_set_ is only protected by context's mutex
  std::unique_lock guard(ctx_->mtx_);
  if (!this->blocked_set_.empty()) {
    auto next = *blocked_set_.begin();
    this->blocked_set_.erase(next);
    ctx_->blocked_set_.erase(next);
    ctx_->runnable_set_.insert(next);
    next->status_ = Task::Status::RUNNABLE;
    DEBUG("task {%u} is notified, [blocked] -> [runnable]", next->id());
  }
}

void Monitor::exit_nowait() {
  auto current = ctx_->this_running_task();
  ASSERT(current->callback_ == nullptr, "callback is not empty");
  ASSERT(idle_ == false, "exit monitor without lock");
  {
    std::unique_lock guard(mtx_);
    idle_ = true;
    notify_one();  // notify one immediately
  }
  DEBUG("task {%u} unlock", ctx_->this_running_task()->id());
}

}  // namespace cppgo
