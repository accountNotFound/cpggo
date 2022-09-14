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

AsyncFunction<void> Monitor::wait() {
  auto current = ctx_->this_running_task();
  exit();
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
  std::unique_lock gaurd(mtx_);
  if (!notify_callback_queue_.empty()) {
    return;
  }
  notify_callback_queue_.push([this]() {
    if (!blocked_set_.empty()) {
      auto next = blocked_set_.get_one();
      this->blocked_set_.erase(next);
      ctx_->blocked_set_.erase(next);
      ctx_->runnable_set_.insert(next);
      next->status_ = Task::Status::RUNNABLE;
      DEBUG("task {%u} is notified, [blocked] -> [runnable]", next->id());
    }
  });
}

void Monitor::exit() {
  auto current = ctx_->this_running_task();
  std::queue<std::function<void()>> callback_queue;
  ASSERT(current->callback_ == nullptr, "callback is not empty");
  ASSERT(idle_ == false, "exit monitor without lock");
  {
    std::unique_lock guard(mtx_);
    idle_ = true;
    if (notify_callback_queue_.empty()) {
      notify_callback_queue_.push([this]() {
        if (!blocked_set_.empty()) {
          auto next = blocked_set_.get_one();
          this->blocked_set_.erase(next);
          ctx_->blocked_set_.erase(next);
          ctx_->runnable_set_.insert(next);
          next->status_ = Task::Status::RUNNABLE;
          DEBUG("task {%u} is notified, [blocked] -> [runnable]", next->id());
        }
      });
    }
    callback_queue.swap(notify_callback_queue_);
  }
  {
    std::unique_lock guard(ctx_->mtx_);
    while (!callback_queue.empty()) {
      callback_queue.front()();
      callback_queue.pop();
    }
  }
  DEBUG("task {%u} unlock", current->id());
}

void Monitor::notify_one_for(Monitor& rhs) {
  std::unique_lock guard(rhs.ctx_->mtx_);
  if (!rhs.blocked_set_.empty()) {
    auto next = rhs.blocked_set_.get_one();
    rhs.blocked_set_.erase(next);
    rhs.ctx_->blocked_set_.erase(next);
    rhs.ctx_->runnable_set_.insert(next);
    next->status_ = Task::Status::RUNNABLE;
    DEBUG("task {%u} is notified, [blocked] -> [runnable]", next->id());
  }
}

}  // namespace cppgo
