#include "context.h"

// #define USE_DEBUG
#include "common/log.h"

namespace cppgo {

void Worker::start() {
  thread_ = std::thread([this]() {
    auto tid = this->id();
    status_ = Status::RUNNING;
    DEBUG("worker {%u} start", tid);
    while (!ctx_->done()) {
      Context::TaskPointer current = nullptr;
      {
        std::unique_lock guard(ctx_->mtx_);
        if (!ctx_->runnable_set_.empty()) {
          current = *ctx_->runnable_set_.begin();
          ctx_->runnable_set_.erase(current);
        }
      }
      DEBUG("worker {%u} get task {%u}", tid, current->id());
      if (!current) {
        DEBUG("worker {%u} sleep", tid);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        DEBUG("worker {%u} resume", tid);
        continue;
      }
      {
        std::unique_lock guard(ctx_->mtx_);
        ctx_->running_map_[current] = this;
        DEBUG("worker {%u} bind with {%u}", tid, current->id());
      }

      DEBUG("worker {%u} start execute task {%u}", tid, current->id());
      current->resume();
      DEBUG("worker {%u} execute task {%u} end", tid, current->id());

      // assert current only in running_map_ and tasks_
      if (current->done()) {
        DEBUG("task {%u} done, resource clean", current->id());
        std::unique_lock guard(ctx_->mtx_);
        ctx_->running_map_.erase(current);
        ctx_->tasks_.destroy(current);
        continue;
      }
      {
        std::unique_lock guard(ctx_->mtx_);
        if (current->callbacks_.empty() && !ctx_->delegate_map_.empty()) {
          auto& [task, callbacks] = *(ctx_->delegate_map_.begin());
          DEBUG("worker {%u} start delegate task {%u} callbacks", tid,
                task->id());
          for (auto& fn : callbacks) {
            fn();
          }
          ctx_->delegate_map_.erase(task);
          DEBUG("worker {%u} delegate task {%u} callbacks end", tid,
                current->id());
        }
        if (!current->callbacks_.empty()) {
          DEBUG("worker {%u} start execute task {%u} callbacks", tid,
                current->id());
          for (auto& fn : current->callbacks_) {
            fn();
          }
          current->callbacks_.clear();
          DEBUG("worker {%u} execute task {%u} callbacks end", tid,
                current->id());
        } else {
          // set current to runnable
          ctx_->runnable_set_.insert(current);
          current->status_ = Task::Status::RUNNABLE;
          DEBUG("worker {%u} set task {%u} to runnable", tid, current->id());
        }
        ctx_->running_map_.erase(current);
        DEBUG("worker {%u} unbind with {%u}", tid, current->id());
      }
    }
    status_ = Status::DONE;
    DEBUG("worker {%u} end", tid);
  });
  DEBUG("thread_ id=%u", thread_.get_id());
}

void Worker::join() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

}  // namespace cppgo
