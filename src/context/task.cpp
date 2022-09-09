#include "context.h"

// #define USE_DEBUG
#include "common/log.h"

namespace cppgo {

void Task::submit_callback_delegate() {
  std::unique_lock guard(ctx_->mtx_);
  ctx_->delegate_map_[this] = std::move(callbacks_);
  callbacks_.clear();
}

}  // namespace cppgo
