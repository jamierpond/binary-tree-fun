#pragma once
#include <queue>

namespace traversal {

template <typename Node, typename Callback>
constexpr void in_order (Node *node, Callback &&callback) {
  if (!node) { return; }
  in_order(node->left, callback);
  callback(node);
  in_order(node->right, callback);
};

template <typename Node, typename Callback>
constexpr void pre_order (Node *node, Callback &&callback) {
  if (!node) { return; }
  callback(node);
  pre_order(node->left, callback);
  pre_order(node->right, callback);
};

template <typename Node, typename Callback>
constexpr void level_order (Node *node, Callback &&callback) {
  if (!node) { return; }
  std::queue<Node *> q{};
  q.push(node);
  while (!q.empty()) {
    auto *n = q.front();
    q.pop();
    callback(n);
    if (n->left) { q.push(n->left); }
    if (n->right) { q.push(n->right); }
  }
};

template <typename Node, typename Callback>
constexpr void post_order (Node *node, Callback &&callback) {
  if (!node) { return; }
  post_order(node->left, callback);
  post_order(node->right, callback);
  callback(node);
};


template <typename Node, typename Callback>
constexpr Node* extreme(Node *node, Callback &&callback, auto get_next) {
  auto *n = node;
  while (get_next(n)) {
    callback(n);
    n = get_next(n);
  }
  return n;
};

template <typename Node, typename Callback>
constexpr Node* extreme_left(Node *node, Callback &&callback = [](auto*){}) {
    return extreme(node, callback, [](auto *n) { return n->left; });
};

template <typename Node, typename Callback>
constexpr Node* extreme_right(Node *node, Callback &&callback) {
    return extreme(node, callback, [](auto *n) { return n->right; });
};

template <typename T, typename Node, typename Callback>
Node *find(Node* root, T value, Node *parent = nullptr, Callback &&on_found_node = [](auto*&, auto*&) {}) {
  Node *n = root;
  for (;;) {
    if (n->value == value) {
      on_found_node(n, parent);
      return n;
    }
    auto *next = n->should_step_left(value) ? n->left : n->right;
    if (!next) {
      return nullptr;
    }
    parent = n;
    n = next;
  }
}

} // namespace traversal

