#include "worker.h"

#include <chrono>
#include <random>

#include "context.h"
#include "monitor.h"
#include "task.h"

// #define USE_DEBUG
#include "common/log.h"

namespace cppgo {

SpinLock Worker::cls_;
Worker::Id Worker::cls_id_ = 0;

std::default_random_engine rand_;

Worker::Worker(Context* ctx)
    : ctx_(ctx), status_(Status::INIT), stop_flag_(false) {
  std::unique_lock guard(cls_);
  id_ = cls_id_++;
}

void Worker::start() {
  thread_ = std::thread([this]() {
    status_ = Status::RUNNING;
    while (!stop_flag_) {
      Task* current = nullptr;
      {
        std::unique_lock guard(ctx_->self_);
        if (!ctx_->runnable_set_.empty()) {
          current = *ctx_->runnable_set_.begin();
          ctx_->runnable_set_.erase(current);
        }
      }
      if (!current) {
        DEBUG("worker {%u} get no task, sleep", id());
        DEBUG("len(running_map)=%d, len(blocked_set)=%d, len(runnable_set)=%d",
              ctx_->running_map_.size(), ctx_->blocked_set_.size(),
              ctx_->runnable_set_.size());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        DEBUG("worker {%u} resume", id());
        continue;
      }
      DEBUG("worker {%u} get task {%u}", id(), current->id());
      {
        std::unique_lock guard(ctx_->self_);
        ctx_->running_map_[current] = this;
        current->status_ = Task::Status::RNUNING;
        DEBUG("worker {%u} bind with {%u}", id(), current->id());
      }

      DEBUG("worker {%u} start execute task {%u}", id(), current->id());
      while (true) {
        DEBUG("worker {%u} resume task {%u}", id(), current->id());
        current->resume();
        DEBUG("worker {%u} resume task {%u} end", id(), current->id());

        std::unique_lock guard(ctx_->self_);
        if (current->done()) {
          DEBUG("task {%u} done, resource clean", current->id());
          ctx_->running_map_.erase(current);
          ctx_->task_mgr_.destroy(current);
          break;
        } else if (current->callback_) {
          DEBUG("worker {%u} execute task {%u} callback", id(), current->id());
          bool runnable = current->callback_();
          current->callback_ = nullptr;

          // current's status has been changed with alternative valid
          // notification done after callback
          if (!runnable) {
            break;
          }
        }
      }
      DEBUG("worker {%u} execute task {%u} end", id(), current->id());
    }
    status_ = Status::DONE;
  });
}

void Worker::join() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

}  // namespace cppgo
