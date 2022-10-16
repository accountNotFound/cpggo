#include <functional>
#include <memory>
#include <typeinfo>

namespace cppgo {

// std::any only supports copyable class. Using this NonCopyAny to contain uncopyable class
class NoCopyAny {
 public:
  NoCopyAny() = default;
  NoCopyAny(const NoCopyAny&) = delete;
  NoCopyAny(NoCopyAny&& rhs);
  ~NoCopyAny();

  void swap(NoCopyAny& rhs);

  const std::type_info& type() const { return _type ? *_type : typeid(void); }

  template <typename T>
  NoCopyAny(T&& value) {
    _value = new T(std::move(value));
    _deletor = [](void* ptr) { delete static_cast<T*>(ptr); };
    _type = &typeid(T);
  }

  template <typename T>
  T& cast() {
    if (typeid(T) != type()) throw std::bad_cast();
    return *static_cast<T*>(_value);
  }

 private:
  void* _value = nullptr;
  std::function<void(void*)> _deletor = nullptr;
  const std::type_info* _type = nullptr;
};

}  // namespace cppgo
