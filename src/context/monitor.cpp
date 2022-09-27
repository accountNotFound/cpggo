#include "monitor.h"

#include "context.h"
#include "task.h"

// #define USE_DEBUG
#include "common/log.h"

namespace cppgo {

SpinLock Monitor::cls_;
Monitor::Id Monitor::cls_id_ = 0;

Monitor::Monitor(Context* ctx, Resource* resource)
    : ctx_(ctx), resource_(resource) {
  std::unique_lock guard(cls_);
  id_ = cls_id_++;
}

AsyncFunction<void> Monitor::enter() {
  auto current = ctx_->this_running_task();
  while (true) {
    resource_->lock();
    if (resource_->available_()) {
      current->callback_ = [this, current]() -> bool {
        DEBUG("task {%u} lock success", current->id());
        this->resource_->acquire_();
        this->resource_->unlock();
        return true;
      };
      co_return;
    } else {
      current->callback_ = [this, current]() -> bool {
        DEBUG("task {%u} lock failed, [running] -> [blocked]", current->id());

        this->blocked_set_.insert(current);
        current->change_status_(Task::Status::RNUNING, Task::Status::BLOCKED);

        this->resource_->unlock();
        return false;
      };
      co_await std::suspend_always{};
    }
  }
}

void Monitor::exit() {
  auto current = ctx_->this_running_task();
  {
    std::unique_lock guard(*resource_);
    resource_->release_();
    notify_one_with_guard();
    DEBUG("task {%u} unlock", current->id());
  }
}

void Monitor::notify_one_with_guard() {
  auto current = ctx_->this_running_task();
  {
    std::unique_lock guard(ctx_->self_);

    if (blocked_set_.empty() && !resource_->available_()) {
      RAISE("invlalid notification");
    }

    DEBUG("task {%u} try notify from monitor {%u}, len(set)=%d", current->id(),
          id(), blocked_set_.size());
    if (!blocked_set_.empty()) {
      auto next = *blocked_set_.begin();
      DEBUG("task {%u} is notified, [blocked] -> [runnable]", next->id());
      blocked_set_.erase(next);
      next->change_status_(Task::Status::BLOCKED, Task::Status::RUNNABLE);
    }
  }
}

AsyncFunction<void> Monitor::suspend_with_guard_unlock() {
  auto current = ctx_->this_running_task();
  if (!resource_->available_()) {
    resource_->release_();
    this->notify_one_with_guard();
  }
  current->callback_ = [this, current]() -> bool {
    DEBUG("task {%u} wait, [running] -> [blocked]", current->id());

    this->blocked_set_.insert(current);
    current->change_status_(Task::Status::RNUNING, Task::Status::BLOCKED);

    this->resource_->unlock();
    return false;
  };
  co_await std::suspend_always{};
}

}  // namespace cppgo
