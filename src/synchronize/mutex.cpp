#include "mutex.h"

// #define USE_DEBUG
#include "common/log.h"

namespace cppgo {

AsyncFunction<void> Mutex::lock() {
  auto current = ctx_->this_running_task();
  while (true) {
    {
      std::unique_lock guard(mtx_);
      if (idle_) {
        idle_ = false;
        DEBUG("task {%u} lock, [running] -> [running]", current->id());
        co_return;
      }
    }

    /*
     * an invalid notify may occur here:
     *
     * task A see idle_=false, prepare to co_await
     * task B set idle_=true, prepare to notify
     * task B's notify has no effection since A is still not inserted into blocked_set by co_await
     * task A co_await and be inserted into blocked set now
     * there is no more notify and task A will never be executed
     * 
     * therefore, an empty notify must be register to context so workers can be access to these notify function later
     */

    DEBUG("task {%u} lock failed", current->id());
    co_await hangup_ctrl_.await();
  }
}

void Mutex::unlock() {
  auto current = ctx_->this_running_task();
  {
    std::unique_lock guard(mtx_);
    idle_ = true;
  }
  DEBUG("task {%u} unlock", current->id());
  hangup_ctrl_.notify_one();
}

AsyncFunction<void> ConditionVariable::wait() {
  auto current = ctx_->this_running_task();
  {
    std::unique_lock guard(mtx_);
    idle_ = true;
  }
  DEBUG("task {%u} unlock and wait", current->id());
  co_await hangup_ctrl_.notify_one().await();
  co_await lock();
}

void ConditionVariable::notify_one() { hangup_ctrl_.notify_one(); }

}  // namespace cppgo
