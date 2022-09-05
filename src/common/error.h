#pragma once

#include <exception>
#include <sstream>
#include <string>

#define RAISE(msg)                                                       \
  {                                                                      \
    std::stringstream ss;                                                \
    ss << __FILE__ << " -> " << __FUNCTION__ << "() [line: " << __LINE__ \
       << "] " << msg << "\n";                                           \
    throw std::runtime_error(ss.str());                                  \
  }

namespace cppgo {}  // namespace cppgo
