#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cppgo {

// template <typename T>
// class RefSet {
//  public:
//   using Iterator = std::unordered_set<T*>::iterator;

//   Iterator get() { return set_.find(i2e_[rand_() % i2e_.size()]); }
//   size_t count(T* data) { return set_.count(data); }
//   bool empty() { return set_.empty(); }

//   void insert(T* data) {
//     e2i_[data] = i2e_.size();
//     i2e_.push_back(data);
//     set_.insert(data);
//   }
//   void erase(T* data) {
//     swap_(e2i_[data], i2e_.size() - 1);
//     e2i_.erase(i2e_.back());
//     set_.erase(i2e_.back());
//     i2e_.pop_back();
//   }

//  private:
//   void swap_(size_t i, size_t j) {
//     e2i_[i2e_[i]] = j;
//     e2i_[i2e_[j]] = i;
//     std::swap(i2e_[i], i2e_[j]);
//   }

//  private:
//   std::default_random_engine rand_;
//   std::unordered_set<T*> set_;
//   std::unordered_map<T*, size_t> e2i_;
//   std::vector<T*> i2e_;
// };

template <typename T>
class RefSet : public std::unordered_set<T*> {
 public:
  std::unordered_set<T*>::iterator get() { return this->begin(); }
};

}  // namespace cppgo
