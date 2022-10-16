#pragma once

namespace cppgo::__detail {

template <typename T>
concept HasImpl = true;

template <HasImpl T>
inline typename T::Impl& impl(T& wrapper) {
  return *wrapper._impl;
}

};  // namespace cppgo::__detail