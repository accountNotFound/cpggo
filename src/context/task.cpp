#include "task.h"

// #define USE_DEBUG
#include "common/error.h"
#include "common/log.h"
#include "context.h"

namespace cppgo {

SpinLock Task::cls_;
Task::Id Task::cls_id_ = 0;

Task::Task(Context* ctx, AsyncFunction<void>&& func)
    : ctx_(ctx), func_(std::move(func)), status_(Status::RUNNABLE) {
  {
    std::unique_lock guard(cls_);
    id_ = cls_id_++;
  }
  func_.init();
}

void Task::change_status_(Status from, Status to) {
  if (from != status_ || to == Status::RNUNING) {
    RAISE("invalid status change");
  }
  auto get_set_ptr = [this](Status s) -> std::unordered_set<Task*>* {
    if (s == Status::RUNNABLE) {
      return &this->ctx_->runnable_set_;
    } else if (s == Status::BLOCKED) {
      return &this->ctx_->blocked_set_;
    }
    return nullptr;
  };

  if (from == Status::RNUNING) {
    ctx_->running_map_.erase(this);
  } else {
    get_set_ptr(from)->erase(this);
  }
  get_set_ptr(to)->insert(this);
  status_ = to;
  DEBUG("task {%u}, {%s} -> {%s}", id(), statu_str(from), statu_str(to));
}

}  // namespace cppgo
