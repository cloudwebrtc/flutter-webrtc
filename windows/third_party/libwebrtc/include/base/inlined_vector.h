// Customise the behaviour of inlined_vector by defining these before including
// it: #define BSP_INLINED_VECTOR_THROWS to get runtime_error #define
// BSP_INLINED_VECTOR_LOG_ERROR(message) to log errors

#ifndef BSP_INLINED_VECTOR_H
#define BSP_INLINED_VECTOR_H

#include <algorithm>
#include <cassert>
#include <iterator>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef BSP_INLINED_VECTOR_THROWS
#include <stdexcept>
#endif

namespace bsp {
namespace detail {
template <class T, int Capacity>
class static_vector {
  static_assert(Capacity > 0, "Capacity is <= 0!");

 public:
  using value_type = T;
  using iterator = value_type*;
  using const_iterator = const value_type*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = int;

 public:
  static_vector() = default;

  explicit static_vector(size_type count, const T& value = T()) {
    size_ = count;
    for (size_type i = 0; i < size_; ++i) {
      new (data_ + i) T(value);
    }
  }

  static_vector(const static_vector& other) {
    size_ = other.size_;
    for (size_type i = 0; i < size_; ++i) {
      new (data_ + i) T(other[i]);
    }
  }

  static_vector(static_vector&& other) noexcept {
    size_ = other.size_;
    for (size_type i = 0; i < size_; ++i) {
      new (data_ + i) T(std::move(other[i]));
    }
    other.clear();
  }

  static_vector& operator=(const static_vector& other) {
    if (other.data_ == data_)
      return *this;

    destroy_all();
    size_ = other.size_;
    for (size_type i = 0; i < size_; ++i) {
      new (data_ + i) T(other[i]);
    }
    return *this;
  }

  static_vector& operator=(static_vector&& other) noexcept {
    if (other.data_ == data_)
      return *this;

    destroy_all();
    size_ = other.size_;
    for (size_type i = 0; i < size_; ++i) {
      new (data_ + i) T(std::move(other[i]));
    }
    other.clear();
    return *this;
  }

  ~static_vector() { destroy_all(); }

  size_type size() const { return size_; }

  constexpr static size_type max_size() { return Capacity; }

  void clear() { destroy_all(); }

  template <class U>
  void push_back(U&& value) {
#ifdef BSP_INLINED_VECTOR_THROWS
    if (size_ >= max_size())
      throw std::bad_alloc{};
#endif
    new (data_ + size_) T(std::forward<U>(value));
    ++size_;
  }

  template <typename... Args>
  void emplace_back(Args&&... args) {
#ifdef BSP_INLINED_VECTOR_THROWS
    if (size_ >= max_size())
      throw std::bad_alloc{};
#endif
    new (data_ + size_) T(std::forward<Args>(args)...);
    ++size_;
  }

  void pop_back() {
    assert(size_ > 0);
    destroy(data_ + size_);
    --size_;
  }

  T& operator[](size_type i) { return *launder(data_ + i); }

  const T& operator[](size_type i) const { return *launder(data_ + i); }

  iterator begin() { return launder(data_); }
  iterator end() { return begin() + size_; }

  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }

  const_iterator begin() const { return launder(data_); }
  const_iterator end() const { return begin() + size_; }

  reverse_iterator rbegin() { return rend() - size_; }
  reverse_iterator rend() { return std::reverse_iterator<iterator>(begin()); }

  const_reverse_iterator rbegin() const { return rend() - size_; }
  const_reverse_iterator rend() const {
    return std::reverse_iterator<const_iterator>(begin());
  }

  template <typename Container>
  void emplace_into(Container& container) {
    assert(container.size() == 0);
    container.resize(size_);
    std::move(begin(), end(), container.begin());
    destroy_all();
  }

  void fill_n(size_type count, const T& value) {
    destroy_all();
#ifdef BSP_INLINED_VECTOR_THROWS
    if (count > max_size())
      throw std::bad_alloc{};
#endif
    for (size_type i = 0; i < count; ++i) {
      new (data_ + i) T(value);
    }
    size_ = count;
  }

 protected:
  using raw_type = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

  raw_type data_[Capacity];
  size_type size_ = 0;

 protected:
  T* launder(raw_type* rt) { return reinterpret_cast<T*>(rt); }

  const T* launder(const raw_type* rt) const {
    return reinterpret_cast<const T*>(rt);
  }

  void destroy(raw_type* rt) { launder(rt)->~T(); }

  void destroy_all() {
    for (size_type i = 0; i < size_; ++i) {
      destroy(data_ + i);
    }
    size_ = 0;
  }
};

template <class, class Enable = void>
struct is_iterator : std::false_type {};
template <typename T_>
struct is_iterator<
    T_,
    typename std::enable_if<
        std::is_base_of<
            std::input_iterator_tag,
            typename std::iterator_traits<T_>::iterator_category>::value ||
        std::is_same<
            std::output_iterator_tag,
            typename std::iterator_traits<T_>::iterator_category>::value>::type>
    : std::true_type {};

template <bool B, class T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <class T_>
using initializer_list_of_copyable =
    enable_if_t<std::is_copy_constructible<T_>::value,
                std::initializer_list<T_>>;

template <class T_>
using const_ref_if_copyable =
    enable_if_t<std::is_copy_constructible<T_>::value, const T_&>;
}  // namespace detail

// An inlined_vector is a fixed-size array with a vector-like interface
// that can optionally grow beyond its capacity and become a std::vector.
template <typename T, int Capacity, bool CanExpand = false>
class inlined_vector {
  static_assert(Capacity > 0, "Capacity is <= 0!");

 public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using iterator = value_type*;
  using const_iterator = const value_type*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = int;

 public:
  inlined_vector() = default;

  inlined_vector(size_type count, const T& value = T())
      : data_internal_(std::min(count, max_size()), value) {
    if (count > max_size()) {
      size_ = max_size();
      length_error("inlined_vector(count, value) got too many elements");
    } else {
      size_ = count;
    }
  }

  template <int Capacity_, bool CanExpand_>
  inlined_vector(const inlined_vector<T, Capacity_, CanExpand_>& other)
      : inlined_vector(other.begin(), other.size()) {
    inlined_vector<T, Capacity, CanExpand>::assert_integrity();
    other.assert_integrity();
  }

  template <int Capacity_, bool CanExpand_>
  inlined_vector(inlined_vector<T, Capacity_, CanExpand_>&& other)
      : inlined_vector(other.begin(), other.size()) {
    inlined_vector<T, Capacity, CanExpand>::assert_integrity();
    other.assert_integrity();
  }

  template <class Container>
  inlined_vector(const Container& els)
      : inlined_vector(els.begin(), static_cast<int>(els.size())) {
    inlined_vector<T, Capacity, CanExpand>::assert_integrity();
  }

  inlined_vector(std::initializer_list<T> els)
      : inlined_vector(els.begin(), static_cast<int>(els.size())) {
    inlined_vector<T, Capacity, CanExpand>::assert_integrity();
  }

  virtual ~inlined_vector() = default;

  constexpr static size_type max_size() { return Capacity; }

  virtual bool can_expand() const { return false; }

  virtual void clear() {
    size_ = 0;
    data_internal_.clear();
    this->assert_integrity();
  }

  size_type size() const { return size_; }

  bool empty() const { return size_ == 0; }

  bool full() const { return size_ >= max_size(); }

  virtual bool expanded() const { return false; }

  template <typename T_ = T>
  void push_back(detail::const_ref_if_copyable<T_> value) {
    // NB: Need this to support aggregates
    if (size_ >= max_size()) {
      length_error("inlined_vector::push_back exceeded capacity");
    } else {
      data_internal_.push_back(value);
      size_++;
      this->assert_integrity();
    }
  }

  template <typename U>
  void push_back(U&& value) {
    if (size_ >= max_size()) {
      length_error("inlined_vector::push_back exceeded capacity");
    } else {
      data_internal_.push_back(std::forward<U>(value));
      size_++;
      this->assert_integrity();
    }
  }

  template <class... Args>
  void emplace_back(Args&&... args) {
    if (size_ >= max_size()) {
      length_error("inlined_vector::emplace_back exceeded capacity");
    } else {
      data_internal_.emplace_back(std::forward<Args>(args)...);
      size_++;
      this->assert_integrity();
    }
  }

  template <class Container>
  void extend(const Container& other) {
    for (auto v : other) {
      push_back(std::move(v));
    }
    this->assert_integrity();
  }

  template <typename T_ = T>
  void extend(detail::initializer_list_of_copyable<T_> other) {
    for (auto v : other) {
      push_back(std::move(v));
    }
    this->assert_integrity();
  }

  virtual void pop_back() {
    if (!empty()) {
      data_internal_.pop_back();
      size_--;
      this->assert_integrity();
    }
  }

  const_reference back() const {
    if (!empty()) {
      return *std::prev(end());
    }
    return data_internal_[0];
  }

  reference back() {
    return const_cast<reference>(
        static_cast<const inlined_vector*>(this)->back());
  }

  const_reference front() const {
    if (!empty()) {
      return *begin();
    }
    return data_internal_[0];
  }

  reference front() {
    return const_cast<reference>(
        static_cast<const inlined_vector*>(this)->front());
  }

  reference operator[](size_type i) { return element(i); }

  const_reference operator[](size_type i) const { return element(i); }

  const_reference at(size_type i) const {
    if (i >= 0 && i < size_) {
      return element(i);
    } else {
#ifdef BSP_INLINED_VECTOR_THROWS
      throw std::out_of_range("inlined_vector::at");
#endif
    }
  }

  reference at(size_type i) {
    return const_cast<reference>(
        static_cast<const inlined_vector*>(this)->at(i));
  }

  virtual iterator begin() { return data_internal_.begin(); }
  iterator end() { return begin() + size_; }

  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }

  virtual const_iterator begin() const { return data_internal_.begin(); }
  const_iterator end() const { return begin() + size_; }

  reverse_iterator rbegin() { return rend() - size_; }
  virtual reverse_iterator rend() { return data_internal_.rend(); }

  const_reverse_iterator rbegin() const { return rend() - size_; }
  virtual const_reverse_iterator rend() const { return data_internal_.rend(); }

  virtual iterator erase(const_iterator it) {
    validate_iterator(it);

    if (it == end() || empty()) {
      out_of_range_error(
          "inlined_vector::erase it == end or container is empty");
      return end();
    }

    size_type i = iterator_index(it);
    if (i == size_) {
      out_of_range_error("inlined_vector::insert invalid iterator");
      return end();
    }
    for (size_type j = i; j < size_ - 1; ++j) {
      element(j) = std::move(element(j + 1));
    }
    data_internal_.pop_back();
    size_--;
    this->assert_integrity();
    return begin() + i;
  }

  template <typename T_ = T>
  iterator insert(iterator it, detail::const_ref_if_copyable<T_> value) {
    validate_iterator(it);

    if (full()) {
      length_error("inlined_vector::insert exceeded Capacity");
      return end();
    }

    if (it == end()) {
      push_back(value);
      this->assert_integrity();
      return std::prev(end(), 1);
    } else {
      // Insert at i and push everything back
      size_type i = iterator_index(it);
      if (i == size_) {
        out_of_range_error("inlined_vector::insert invalid iterator");
        return end();
      }
      data_internal_.push_back(std::move(element(size_ - 1)));
      for (size_type j = size_ - 1; j > i; j--) {
        element(j) = std::move(element(j - 1));
      }
      element(i) = value;
      size_++;
      this->assert_integrity();
      return std::next(begin(), i);
    }
  }

  bool contains(const_reference value) const {
    auto begin_ = begin();
    auto end_ = end();
    return std::find(begin_, end_, value) != end_;
  }

 protected:
  using array_type = detail::static_vector<T, Capacity>;

  array_type data_internal_;
  size_type size_ = 0;

 protected:
  // Helper constructor
  template <typename Iter,
            typename =
                typename std::enable_if<detail::is_iterator<Iter>::value>::type>
  inlined_vector(Iter begin_, int size) {
    if (size > max_size()) {
      length_error("inlined_vector() too many elements");
      size_ = max_size();
    } else {
      size_ = size;
    }

    auto end_ = std::next(begin_, size_);
    for (auto it = begin_; it != end_; ++it) {
      data_internal_.emplace_back(std::move(*it));
    }

    inlined_vector<T, Capacity, CanExpand>::assert_integrity();
  }

  // Helper constructor for sub-class
  inlined_vector(array_type&& array, int size, bool inlined)
      : data_internal_(std::move(array)), size_(size) {
    assert(array.size() == 0);
    assert(!inlined || size_ == data_internal_.size());
  }

  reference element(size_type index) { return *std::next(begin(), index); }

  const_reference element(size_type index) const {
    return *std::next(begin(), index);
  }

  size_type iterator_index(const_iterator it) const {
    auto nit = begin();
    for (size_type i = 0; i < size_; i++) {
      if (nit == it)
        return i;
      ++nit;
    }
    return size_;
  }

  void validate_iterator(const_iterator it) {
#ifndef NDEBUG
    if (it < begin() || it > end()) {
      out_of_range_error("inlined_vector::validate_iterator invalid iterator");
    }
#endif
  }

  void length_error(const char* message) const {
#ifdef BSP_INLINED_VECTOR_LOG_ERROR
    BSP_INLINED_VECTOR_LOG_ERROR(message);
#endif

#ifdef BSP_INLINED_VECTOR_THROWS
    throw std::length_error(message);
#endif
  }

  void out_of_range_error(const char* message) const {
#ifdef BSP_INLINED_VECTOR_LOG_ERROR
    BSP_INLINED_VECTOR_LOG_ERROR(message);
#endif

#ifdef BSP_INLINED_VECTOR_THROWS
    throw std::out_of_range(message);
#endif
  }

 public:
  virtual void assert_integrity() const {
#ifndef NDEBUG
    assert(size_ == data_internal_.size());
#endif
  }

  template <typename T_, int Capacity_>
  friend std::ostream& operator<<(
      std::ostream& out,
      const inlined_vector<T_, Capacity_, false>& vector);
};

template <typename T, int Capacity>
class inlined_vector<T, Capacity, true>
    : public inlined_vector<T, Capacity, false> {
  static_assert(Capacity > 0, "Capacity is <= 0!");

 public:
  using base_t = inlined_vector<T, Capacity, false>;
  using base_t::assert_integrity;
  using base_t::cbegin;
  using base_t::data_internal_;
  using base_t::element;
  using base_t::empty;
  using base_t::end;
  using base_t::length_error;
  using base_t::max_size;
  using base_t::out_of_range_error;
  using base_t::rbegin;
  using base_t::size_;
  using typename base_t::const_iterator;
  using typename base_t::const_reference;
  using typename base_t::const_reverse_iterator;
  using typename base_t::iterator;
  using typename base_t::reference;
  using typename base_t::reverse_iterator;
  using typename base_t::size_type;
  using typename base_t::value_type;

 public:
  inlined_vector() = default;

  inlined_vector(size_type count, const T& value = T()) {
    size_ = count;
    if (size_ <= max_size()) {
      data_internal_.fill_n(count, value);
    } else {
      data_external_.resize(size_);
      std::fill_n(data_external_.begin(), count, value);
      inlined_ = false;
    }

    this->assert_integrity();
  }

  template <int Capacity_, bool CanExpand_>
  inlined_vector(const inlined_vector<T, Capacity_, CanExpand_>& other)
      : inlined_vector(other.begin(), other.end(), other.size()) {
    other.assert_integrity();
    this->assert_integrity();
  }

  inlined_vector(inlined_vector&& other) noexcept
      : base_t(std::move(other.data_internal_), other.size_, other.inlined_),
        data_external_(std::move(other.data_external_)),
        inlined_(other.inlined_) {
    other.inlined_ = true;
    other.size_ = 0;

    other.assert_integrity();
    this->assert_integrity();
  }

  template <class Container>
  inlined_vector(const Container& els)
      : inlined_vector(els.begin(), els.end(), static_cast<int>(els.size())) {
    this->assert_integrity();
  }

  inlined_vector(std::initializer_list<T> els)
      : inlined_vector(els.begin(), els.end(), static_cast<int>(els.size())) {
    this->assert_integrity();
  }

  inlined_vector(const inlined_vector& other)
      : inlined_vector(other.begin(), other.end(), other.size()) {
    this->assert_integrity();
  }

  inlined_vector& operator=(inlined_vector&& other) noexcept {
    inlined_ = other.inlined_;
    size_ = other.size_;
    data_internal_ = std::move(other.data_internal_);
    data_external_ = std::move(other.data_external_);
    other.inlined_ = true;
    other.size_ = 0;
    other.assert_integrity();
    this->assert_integrity();
    return *this;
  }

  inlined_vector& operator=(const inlined_vector& other) {
    inlined_ = other.inlined_;
    size_ = other.size_;
    data_internal_ = other.data_internal_;
    data_external_ = other.data_external_;
    other.assert_integrity();
    this->assert_integrity();
    return *this;
  }

  template <class Container>
  void extend(const Container& other) {
    for (auto v : other) {
      push_back(std::move(v));
    }
    this->assert_integrity();
  }

  template <typename T_ = T>
  void extend(detail::initializer_list_of_copyable<T_> other) {
    for (auto v : other) {
      push_back(std::move(v));
    }
    this->assert_integrity();
  }

  bool can_expand() const override final { return true; }

  void clear() override final {
    if (inlined_) {
      base_t::clear();
    } else if (!inlined_) {
      inlined_ = true;
      size_ = 0;
      data_external_.clear();
      this->assert_integrity();
    }
  }

  bool expanded() const final override { return !inlined_; }

  template <typename T_ = T>
  void push_back(detail::const_ref_if_copyable<T_> value) {
    // NB: Need this overload to support aggregates in the parameter
    if (inlined_ && size_ >= max_size()) {
      grow_to_external_storage();
    }

    if (inlined_) {
      base_t::push_back(value);
    } else {
      data_external_.push_back(value);
      ++size_;
    }

    this->assert_integrity();
  }

  template <typename U>
  void push_back(U&& value) {
    if (inlined_ && size_ >= max_size()) {
      grow_to_external_storage();
    }

    if (inlined_) {
      base_t::push_back(std::forward<U>(value));
    } else {
      data_external_.push_back(std::forward<U>(value));
      ++size_;
    }

    this->assert_integrity();
  }

  template <class... Args>
  void emplace_back(Args&&... args) {
    if (inlined_ && size_ >= max_size()) {
      grow_to_external_storage();
    }

    if (inlined_) {
      base_t::emplace_back(std::forward<Args>(args)...);
    } else {
      data_external_.emplace_back(std::forward<Args>(args)...);
      ++size_;
    }

    this->assert_integrity();
  }

  void pop_back() override final {
    if (!empty()) {
      if (inlined_) {
        base_t::pop_back();
      } else {
        // TODO: become inlined again if small enough?
        data_external_.pop_back();
        --size_;
      }

      this->assert_integrity();
    }
  }

  iterator begin() override final {
    return inlined_ ? data_internal_.begin() : unwrap(data_external_.begin());
  }
  const_iterator begin() const override final {
    return inlined_ ? data_internal_.begin() : unwrap(data_external_.begin());
  }
  reverse_iterator rend() override final {
    return inlined_ ? data_internal_.rend() : unwrap(data_external_.rend());
  }
  const_reverse_iterator rend() const override final {
    return inlined_ ? data_internal_.rend() : unwrap(data_external_.rend());
  }

  iterator erase(const_iterator it) override final {
    base_t::validate_iterator(it);

    if (it == end() || empty()) {
      out_of_range_error(
          "inlined_vector::erase it == end or container is empty");
    }

    if (inlined_) {
      size_type i = base_t::iterator_index(it);
      if (i == size_) {
        out_of_range_error("inlined_vector::erase invalid iterator");
        return end();
      }
      for (size_type j = i; j < size_ - 1; ++j) {
        element(j) = std::move(element(j + 1));
      }
      data_internal_.pop_back();
      --size_;
      this->assert_integrity();
      return begin() + i;
    } else {
      --size_;
      // Note: a bug in gcc 4.8.4 means we have to use a non-const iterator here
      auto vit = std::next(data_external_.begin(), std::distance(cbegin(), it));
      auto res = unwrap(data_external_.erase(vit));
      this->assert_integrity();
      return res;
    }
  }

  template <typename T_ = T>
  iterator insert(iterator it, detail::const_ref_if_copyable<T_> value) {
    base_t::validate_iterator(it);

    if (inlined_ && size_ < max_size()) {
      return base_t::insert(it, value);
    } else if (inlined_ && size_ >= max_size()) {
      size_type index_ = base_t::iterator_index(it);
      grow_to_external_storage();
      it = std::next(begin(), index_);
    }

    if (it == end()) {
      push_back(value);
      this->assert_integrity();
      return end();
    } else {
      ++size_;
      // NB: dataVector may not have a T* iterator
      auto vit = std::next(data_external_.begin(), std::distance(begin(), it));
      auto res = unwrap(data_external_.insert(vit, value));
      this->assert_integrity();
      return res;
    }
  }

 protected:
  std::vector<T> data_external_;
  bool inlined_ = true;

 protected:
  // Helper constructor
  template <typename Iter>
  inlined_vector(Iter begin_, Iter end_, size_type size) {
    size_ = size;
    if (size_ <= max_size()) {
      for (auto it = begin_; it != end_; ++it) {
        data_internal_.emplace_back(std::move(*it));
      }
    } else {
      data_external_.resize(size_);
      std::move(begin_, end_, data_external_.begin());
      inlined_ = false;
    }

    this->assert_integrity();
  }

  // Sometimes std::vector<T>::iterator isn't a T* (e.g., in clang/libcxx)
  // so we need to unwrap the iterator
  iterator unwrap(typename std::vector<T>::iterator it) const { return &*it; }
  const_iterator unwrap(typename std::vector<T>::const_iterator it) const {
    return &*it;
  }
  reverse_iterator unwrap(typename std::vector<T>::reverse_iterator it) const {
    return reverse_iterator(&*it);
  }
  const_reverse_iterator unwrap(
      typename std::vector<T>::const_reverse_iterator it) const {
    return const_reverse_iterator(&*it);
  }

  void grow_to_external_storage() {
    assert(inlined_);
    data_internal_.emplace_into(data_external_);
    inlined_ = false;

    this->assert_integrity();
  }

 public:
  void assert_integrity() const override final {
#ifndef NDEBUG
    if (inlined_) {
      assert(size_ == static_cast<size_type>(data_internal_.size()));
      assert(0 == data_external_.size());
    } else {
      assert(0 == data_internal_.size());
      assert(size_ == static_cast<size_type>(data_external_.size()));
    }
#endif
  }

  template <typename T_, int Capacity_>
  friend std::ostream& operator<<(
      std::ostream& out,
      const inlined_vector<T_, Capacity_, true>& vector);
};

template <typename T, int Capacity>
std::ostream& operator<<(std::ostream& out,
                         const inlined_vector<T, Capacity, false>& vector) {
  out << "inlined_vector ";
  out << "(inlined):  [";
  if (vector.empty())
    out << "]";
  else {
    for (auto it = vector.begin(); it != vector.end(); ++it) {
      if (std::next(it) != vector.end())
        out << *it << ", ";
      else
        out << *it << "]";
    }
  }
  return out;
}

template <typename T, int Capacity>
std::ostream& operator<<(std::ostream& out,
                         const inlined_vector<T, Capacity, true>& vector) {
  out << "inlined_vector ";
  if (vector.inlined_)
    out << "(inlined):  [";
  else
    out << "(external): [";
  if (vector.empty())
    out << "]";
  else {
    for (auto it = vector.begin(); it != vector.end(); ++it) {
      if (std::next(it) != vector.end())
        out << *it << ", ";
      else
        out << *it << "]";
    }
  }
  return out;
}
}  // namespace bsp

#endif