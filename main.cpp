#include <iostream>
#include <queue>
#include <vector>
#include <set>

auto is_on_edge(int, int) {
  return true;
}


std::vector<std::pair<int, int>> get_neighbors() {
  return {};
}

bool do_you_flow_into_me(auto us, auto them) {
  return true;
}

std::vector<std::pair<int, int>> get_wet_list(auto dry_list, auto w, auto h) {
  return {};
}

std::vector<std::pair<int, int>> find_flooding(const std::vector<std::vector<int>>& map) {
  auto w = map.size();
  auto h = map[0].size();
  auto queue = std::queue<std::pair<int, int>>();

  for (int col = 0; col < map.size(); col++) {
    for (int row = 0; row < map[col].size(); row++) {
      if (is_on_edge(row, col)) {
        queue.push({row, col});
      }
    }
  }

  auto dry_list  = std::set<std::pair<int, int>>();

  while (!queue.empty()) {
    auto us = queue.front();
    queue.pop();
    for (auto n : get_neighbors()) {
      if (do_you_flow_into_me(us, n)) {
        queue.push(us);
        dry_list.insert(us);
      }
    }
  }

  return get_wet_list(dry_list, w, h);
}



int main() {
  std::cout << "Hello, World!" << std::endl;
  return 0;
}
