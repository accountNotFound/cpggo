// #define USE_DEBUG
#include "common/log.h"
#include "context.h"

namespace cppgo {

std::suspend_always HangupCtrl::await() {
  auto current = ctx_->this_running_task();
  current->callback_ = [this, current]() {
    this->blocked_set_.insert(current);
    ctx_->blocked_set_.insert(current);
    current->status_ = Task::Status::BLOCKED;
    DEBUG("task {%u} hangup, [running] -> [blocked]", current->id());
  };
  return {};
}

HangupCtrl& HangupCtrl::notify_one() {
  auto fn = [this]() {
    ASSERT(!this->blocked_set_.empty(),
           "HangupCtrl's blocked set should not be empty");
    if (ctx_->may_dead_set_.count(this)) {
      ctx_->may_dead_set_.erase(this);
    }
    auto next = *this->blocked_set_.get();
    this->blocked_set_.erase(next);
    ctx_->blocked_set_.erase(next);
    ctx_->runnable_set_.insert(next);
    next->status_ = Task::Status::RUNNABLE;
    DEBUG("task {%u} is notified, [blocked] -> [runnable]", next->id());
  };
  std::unique_lock guard(ctx_->mtx_);
  if (!blocked_set_.empty()) {
    fn();
  } else {
    notify_func_ = fn;
    ctx_->may_dead_set_.insert(this);
  }
  return *this;
}

void HangupCtrl::release() {
  std::unique_lock guard(ctx_->mtx_);
  if (ctx_->may_dead_set_.count(this)) {
    ctx_->may_dead_set_.erase(this);
  }
}

}  // namespace cppgo
