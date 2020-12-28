#pragma once
#include <cstdint>
#include <list>
#include <unordered_map>
#include <utility>

namespace voxer {

template <typename Key, typename Value> class LRUCache {
public:
  using ItemType = std::pair<Key, Value>;
  using Iterator = typename std::list<ItemType>::iterator;

  explicit LRUCache(size_t capacity = 10) : m_capacity(capacity) {}

  Value *get(const Key &key) {
    auto it = pos.find(key);
    if (it == pos.end()) {
      return nullptr;
    }

    move_to_head(it->second);
    return &(data.begin()->second);
  }

  void emplace(const Key &key, Value value) {
    auto size = pos.size();
    auto it = pos.find(key);
    if (it != pos.end()) {
      it->second->second = std::move(value);
      move_to_head(it->second);
      return;
    }

    if (size >= m_capacity) {
      auto &last = data.back();
      pos.erase(last.first);
      data.pop_back();
    }
    data.emplace_front(std::make_pair(key, std::move(value)));
    pos[key] = data.begin();
  }

  void move_to_head(typename std::list<ItemType>::iterator &it) {
    auto key = it->first;
    data.emplace_front(std::move(*it));
    data.erase(it);
    pos[key] = data.begin();
  }

  bool has(const Key &key) const noexcept { return pos.count(key); }

private:
  std::unordered_map<Key, Iterator> pos;
  std::list<ItemType> data;
  size_t m_capacity;
};

} // namespace voxer
