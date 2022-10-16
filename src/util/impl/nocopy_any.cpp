#include "../nocopy_any.h"

namespace cppgo {

NoCopyAny::NoCopyAny(NoCopyAny&& rhs) {
  this->~NoCopyAny();
  this->swap(rhs);
}

NoCopyAny::~NoCopyAny() {
  if (_value) _deletor(_value);
  _value = nullptr;
  _deletor = nullptr;
  _type = nullptr;
}

void NoCopyAny::swap(NoCopyAny& rhs) {
  std::swap(_value, rhs._value);
  std::swap(_deletor, rhs._deletor);
  std::swap(_type, rhs._type);
}

}  // namespace cppgo
