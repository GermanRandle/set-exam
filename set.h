#pragma once

#include <cassert>  // assert
#include <iterator> // std::reverse_iterator
#include <utility>  // std::pair, std::swap

template <typename T>
struct set {
  struct node;

  struct base_node
  {
    friend struct set;

    base_node(base_node* parent) : left(nullptr), right(nullptr), parent(parent) {}

  protected:
    node* left;
    node* right;
    base_node* parent;
  };

  struct node : base_node
  {
    friend struct set;

    node(base_node* parent, const T& t) : base_node(parent), value(t) {}

  private:
    T value;
  };

  struct iterator;
  using const_iterator = iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  // O(1) nothrow
  set() noexcept : fake(nullptr) {}

  // O(n) strong
  set(set const& other) : fake(nullptr) {
    node* copied = recursive_copy(other.fake.left, &fake);
    fake.left = copied;
  }

  // O(n) strong
  set& operator=(set const& other) {
    if (this == &other) {
      return *this;
    }
    set(other).swap(*this);
    return *this;
  }

  // O(n) nothrow
  ~set() noexcept {
    clear();
  }

  // O(n) nothrow
  void clear() {
    recursive_clear(fake.left);
    fake.left = nullptr;
  }

  // O(1) nothrow
  bool empty() {
    return fake.left == nullptr;
  }

  //      nothrow
  const_iterator begin() const {
    return const_iterator(find_min(&fake));
  }

  //      nothrow
  const_iterator end() const {
    return const_iterator(&fake);
  }

  //      nothrow
  const_reverse_iterator rbegin() const {
    return std::reverse_iterator(end());
  }

  //      nothrow
  const_reverse_iterator rend() const {
    return std::reverse_iterator(begin());
  }

  // O(h) strong
  std::pair<iterator, bool> insert(T const& el) {
    node** cur = &(fake.left);
    base_node* prev = &fake;
    while (*cur != nullptr) {
      prev = *cur;
      if (el < (*cur)->value) {
        cur = &((*cur)->left);
      } else if ((*cur)->value < el) {
        cur = &((*cur)->right);
      } else {
        return {iterator(*cur), false};
      }
    }
    node* new_node = new node(prev, el);
    *cur = new_node;
    return {iterator(new_node), true};
  }

  // O(h) nothrow
  iterator erase(iterator const& pos) noexcept {
    node* cur = static_cast<node*>(pos.node_);
    if (cur->right == nullptr || cur->left == nullptr) {
      if (cur == fake.left) {
        fake.left = (cur->left == nullptr ? cur->right : cur->left);
      }
      iterator it = next(cur);
      erase_good(cur);
      return it;
    } else {
      base_node* nxt = find_min(cur->right);
      if (cur == fake.left) {
        fake.left = static_cast<node*>(nxt);
      }
      swap_nodes(cur, static_cast<node*>(nxt));
      erase_good(cur);
      return iterator(nxt);
    }
  }

  // O(h) strong
  const_iterator find(T const& el) const {
    node* cur = fake.left;
    while (cur != nullptr && cur->value != el) {
      if (el < cur->value) {
        cur = cur->left;
      } else {
        cur = cur->right;
      }
    }
    return cur == nullptr ? end() : const_iterator(cur);
  }

  // O(h) strong
  const_iterator lower_bound(T const& el) const {
    return some_bound([](T x, T y) { return x < y; }, el);
  }

  // O(h) strong
  const_iterator upper_bound(T const& el) const {
    return some_bound([](T x, T y) { return x <= y; }, el);
  }

  // O(1) nothrow
  void swap(set& other) {
    if (fake.left != nullptr) {
      fake.left->parent = &other.fake;
    }
    if (other.fake.left != nullptr) {
      other.fake.left->parent = &fake;
    }
    std::swap(fake.left, other.fake.left);
  }

private:
  static void recursive_clear(node* cur) noexcept {
    if (cur == nullptr) {
      return;
    }
    recursive_clear(cur->left);
    recursive_clear(cur->right);
    delete cur;
  }

  static node* recursive_copy(node* cur, base_node* par) {
    if (cur == nullptr) {
      return nullptr;
    }
    node* new_node = new node(par, cur->value);
    try {
      new_node->left = recursive_copy(cur->left, new_node);
      try {
        new_node->right = recursive_copy(cur->right, new_node);
      } catch (...) {
        recursive_clear(new_node->left);
        throw;
      }
    } catch (...) {
      delete new_node;
      throw;
    }
    return new_node;
  }

  static void swap_adjacent_nodes(node* first, node* second) noexcept {
    if (first->parent != nullptr) {
      (first->parent->left == first ? first->parent->left : first->parent->right) = second;
    }
    if (first->left != nullptr) {
      first->left->parent = second;
    }
    if (second->left != nullptr) {
      second->left->parent = first;
    }
    if (second->right != nullptr) {
      second->right->parent = first;
    }
    second->parent = first->parent;
    first->parent = second;
    if (first->left == second) {
      first->right = second->right;
      std::swap(first->left, second->left);
      second->left = first;
    } else {
      first->right = second->right;
      std::swap(first->left, second->left);
      second->right = first;
    }
  }

  static void swap_nodes(node* first, node* second) noexcept {
    if (first->left == second || first->right == second) {
      swap_adjacent_nodes(first, second);
    } else {
      if (first->parent != nullptr) {
        (first->parent->left == first ? first->parent->left : first->parent->right) = second;
      }
      if (second->parent != nullptr) {
        (second->parent->left == second ? second->parent->left : second->parent->right) = first;
      }
      if (first->left != nullptr) {
        first->left->parent = second;
      }
      if (first->right != nullptr) {
        first->right->parent = second;
      }
      if (second->left != nullptr) {
        second->left->parent = first;
      }
      if (second->right != nullptr) {
        second->right->parent = first;
      }
      std::swap(first->parent, second->parent);
      std::swap(first->left, second->left);
      std::swap(first->right, second->right);
    }
  }

  static void erase_good(node* cur) noexcept {
    if (cur->left == nullptr) {
      if (cur->right == nullptr) {
        if (cur->parent != nullptr) {
          (cur->parent->left == cur ? cur->parent->left : cur->parent->right) = nullptr;
        }
      } else {
        if (cur->parent != nullptr) {
          (cur->parent->left == cur ? cur->parent->left : cur->parent->right) = cur->right;
        }
        cur->right->parent = cur->parent;
      }
    } else {
      if (cur->parent != nullptr) {
        (cur->parent->left == cur ? cur->parent->left : cur->parent->right) = cur->left;
      }
      cur->left->parent = cur->parent;
    }
    delete cur;
  }

  static base_node* next(base_node* cur) noexcept {
    if (cur->right != nullptr) {
      return find_min(cur->right);
    }
    while (cur->parent != nullptr && cur->parent->left != cur) {
      cur = cur->parent;
    }
    return cur->parent;
  }

  static base_node* prev(base_node* cur) noexcept {
    if (cur->left != nullptr) {
      return find_max(cur->left);
    }
    while (cur->parent != nullptr && cur->parent->right != cur) {
      cur = cur->parent;
    }
    return cur->parent;
  }

  static base_node* find_min(base_node* cur) noexcept {
    return (cur->left == nullptr) ? cur : find_min(cur->left);
  }

  static base_node* find_max(base_node* cur) noexcept {
    return (cur->right == nullptr) ? cur : find_max(cur->right);
  }

  template <typename F>
  const_iterator some_bound(const F& f, T const& el) const {
    node* cur = fake.left;
    node* best = nullptr;
    while (cur != nullptr) {
      if (f(cur->value, el)) {
        cur = cur->right;
      } else {
        if (best == nullptr || best->value > cur->value) {
          best = cur;
        }
        cur = cur->left;
      }
    }
    if (best == nullptr) {
      return end();
    }
    return const_iterator(best);
  }

  mutable base_node fake;
};

template <typename T>
struct set<T>::iterator {
  friend struct set;

  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = T const;
  using difference_type = ptrdiff_t;
  using pointer = T const*;
  using reference = T const&;

  iterator() : node_(nullptr) {}

  // O(1) nothrow
  reference operator*() const {
    return static_cast<node*>(node_)->value;
  }

  // O(1) nothrow
  pointer operator->() const {
    return &operator*();
  }

  //      nothrow
  iterator& operator++() & {
    node_ = next(static_cast<node*>(node_));
    return *this;
  }

  //      nothrow
  iterator operator++(int) & {
    auto copy = *this;
    ++(*this);
    return copy;
  }

  //      nothrow
  iterator& operator--() & {
    node_ = prev(node_);
    return *this;
  }

  //      nothrow
  iterator operator--(int) & {
    auto copy = *this;
    --(*this);
    return copy;
  }

  bool operator==(iterator const& other) const {
    return node_ == other.node_;
  }

  bool operator!=(iterator const& other) const {
    return !operator==(other);
  }

private:
  iterator(base_node* node) : node_(node) {}

  base_node* node_;
};

template <typename T>
void swap(set<T>& s1, set<T>& s2) {
  s1.swap(s2);
}
