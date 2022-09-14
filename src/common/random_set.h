#include <random>
#include <unordered_map>
#include <vector>

namespace cppgo {

template <typename T>
class RandomSet {
 public:
  void insert(T& data) {
    elements_.push_back(data);
    e2idx_[data] = elements_.size() - 1;
  }

  void erase(T& data) {
    swap_(elements_.size() - 1, e2idx_[data]);
    e2idx_.erase(data);
    elements_.pop_back();
  }

  T get_one() {
    size_t ri = e_() % elements_.size();
    return elements_[ri];
  }

  bool empty() { return elements_.empty(); }
  bool count(T& data) { return e2idx_.count(data); }

 private:
  void swap_(size_t i, size_t j) {
    e2idx_[elements_[i]] = j;
    e2idx_[elements_[j]] = i;
    std::swap(elements_[i], elements_[j]);
  }

 private:
  std::unordered_map<T, size_t> e2idx_;
  std::vector<T> elements_;
  std::default_random_engine e_;
};

}  // namespace cppgo
