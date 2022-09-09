#include "context.h"

// #define USE_DEBUG
#include "common/log.h"

namespace cppgo {

AsyncFunction<void> Monitor::enter() {
  while (true) {
    {
      std::unique_lock guard(mtx_);
      if (idle_) {
        idle_ = false;
        DEBUG("task {%u} lock", ctx_->this_running_task()->id());
        co_return;
      }
    }
    auto current = ctx_->this_running_task();
    current->add_callback([this, &current]() {
      this->blocked_set_.insert(current);
      ctx_->blocked_set_.insert(current);
      current->status_ = Task::Status::BLOCKED;
      DEBUG("task {%u} try lock, [running] -> [blocked]", current->id());
    });
    co_await std::suspend_always{};  // yield to scheduler to notify others
  }
}

AsyncFunction<void> Monitor::exit() {
  {
    std::unique_lock guard(mtx_);
    idle_ = true;
  }
  DEBUG("task {%u} unlock", ctx_->this_running_task()->id());
  // don't use callback to change idle_, since there need to be at least one
  // runnable task after idle_ becomes true

  auto current = ctx_->this_running_task();
  current->add_callback([this, &current]() {
    ctx_->runnable_set_.insert(current);
    current->status_ = Task::Status::RUNNABLE;
    DEBUG("task {%u} [running] -> [runnable]", current->id());
  });
  notify_one();                    // add notify callback
  co_await std::suspend_always{};  // yield to scheduler to notify others
}

AsyncFunction<void> Monitor::wait() {
  {
    std::unique_lock guard(mtx_);
    idle_ = true;
  }
  DEBUG("task {%u} unlock", ctx_->this_running_task()->id());
  // don't use callback, since there need to be at least one runnable
  // task after idle_ becomes true

  auto current = ctx_->this_running_task();
  current->add_callback([this, &current]() {
    ctx_->blocked_set_.insert(current);
    current->status_ = Task::Status::BLOCKED;
    DEBUG("task {%u} [running] -> [blocked]", current->id());
  });

  // normally the notify_one() should be call before wait(), so there will be
  // at least one runnable task

  co_await std::suspend_always{};  // yield to scheduler to notify others
}

void Monitor::notify_one() {
  // no need to lock
  // simply add callback
  auto current = ctx_->this_running_task();
  current->add_callback([this]() {
    if (!this->blocked_set_.empty()) {
      auto next = *blocked_set_.begin();
      this->blocked_set_.erase(next);
      ctx_->blocked_set_.erase(next);
      ctx_->runnable_set_.insert(next);
      next->status_ = Task::Status::RUNNABLE;
      DEBUG("task {%u} [blocked] -> [runnable]", next->id());
    }
  });
}

void Monitor::exit_nowait() {
  {
    std::unique_lock guard(mtx_);
    idle_ = true;
  }
  DEBUG("task {%u} unlock", ctx_->this_running_task()->id());
  // don't use callback to change idle_, since there need to be at least one
  // runnable task after idle_ becomes true

  auto current = ctx_->this_running_task();
  notify_one();  // add notify callback

  current->submit_callback_delegate();
  // this exit_nowait() doesn't co_await to worker's schedule code, which means
  // this thread will not execute this task's callbacks to update other tasks'
  // status. 
  // so you have to submit callbacks to context so that other worker can
  // execute current task's callback
}

}  // namespace cppgo
