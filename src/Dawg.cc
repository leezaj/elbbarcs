#include "Dawg.h"
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

Dawg::Dawg(const std::filesystem::path& dict) :
  Dawg(dict, StateToIndex{})
{}

Dawg::Dawg(const std::filesystem::path &dict, StateToIndex all_states):
  all_nodes_(1)  /* start with one node (the root node) */
{
  std::ifstream file{dict};
  std::vector<std::string> words{std::istream_iterator<std::string>{file}, {}};
  for (std::string_view previous{}; std::string_view word : words) {
    std::string_view common_prefix{previous.begin(), std::ranges::mismatch(previous, word).in1};
    std::string_view common_suffix = word.substr(common_prefix.size());
    auto prefix_idx = get_last_state_idx(common_prefix);
    if (not all_nodes_[prefix_idx].edges.empty()) {
      replace_or_register(prefix_idx, all_states);
    }
    add_suffix(prefix_idx, common_suffix);
    previous = word;
  }
  replace_or_register(0, all_states);
}

std::uint32_t Dawg::get_last_state_idx(std::string_view common_prefix) {
  std::uint32_t current = 0;
  for (char ch : common_prefix){
    current = std::ranges::find(all_nodes_[current].edges, ch, &Edge::letter)->next;
  }
  return current;
}

void Dawg::replace_or_register(std::uint32_t last_state_idx, StateToIndex& all_states) {
  auto next_idx = all_nodes_[last_state_idx].edges.rbegin()->next;
  if (not all_nodes_[next_idx].edges.empty()) {
    replace_or_register(next_idx, all_states);
  }
  auto state_representation = all_nodes_[next_idx].state();
  if(auto it = all_states.find(state_representation); it!=all_states.end()){
    all_nodes_.pop_back();
    all_nodes_[last_state_idx].edges.rbegin()->next = it->second;
  } else {
    all_states.emplace(std::move(state_representation), next_idx);
  }
}

void Dawg::add_suffix(std::uint32_t state_idx, std::string_view suffix) {
  for(char ch: suffix) {
    auto new_idx = static_cast<std::uint32_t>(all_nodes_.size());
    all_nodes_[state_idx].edges.emplace_back(ch, new_idx);
    state_idx = new_idx;
    all_nodes_.emplace_back(/* new node*/);
  }
  all_nodes_[state_idx].final = true;
}
