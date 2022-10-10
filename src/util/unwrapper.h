#pragma once

namespace cppgo::__detail {

template <typename T>
concept HasImpl = true;

template <HasImpl T>
typename T::Impl& impl(T& wrapper) {
  return *wrapper._impl.get();
}

};  // namespace cppgo::__detail