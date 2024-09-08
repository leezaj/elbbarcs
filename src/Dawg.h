#ifndef DAWG_H
#define DAWG_H

#include <algorithm>
#include <filesystem>
#include <glaze/glaze.hpp>
#include <unordered_map>
#include <vector>

/**
 * @brief Class that implements a Directed Acyclic Word Graph (DAWG). More information can be found at
 * https://en.wikipedia.org/wiki/Deterministic_acyclic_finite_state_automaton .
 *
 * @details Nodes are either final or not, representing whether a given state constitutes a valid word.
 *
 * All the nodes of the DAWG are stored contiguously in one vector. The vector of nodes is an adjacency list where
 * every node has a vector of edges, and where each edge corresponds to a letter and an *index*.
 * The index leads to the next node in the vector of nodes. The first node in the vector (index 0) is the root.
 *
 * The edges are represented as a vector, instead of something like an unordered_map or a fixed size array, 
 * because a linear search for a letter will be at most 26 elements in the worst case for the English alphabet, which
 * is fast enough for our purposes. A vector with only the necessary letters is also more memory efficient than both.
 * This model also has the added advantage of making serialization trivial.
 *
 * The algorithm used for creating the DAWG is outlined in the paper
 * "Incremental Construction of Minimal Acyclic Finite-State Automata" by Jan Daciuk et al. 
 * It's important to note that the dictionary file provided must be lexographically sorted, and that the Dawg is 
 * 'build-once' with no support for modification or deletion afterwards.
 */
class Dawg final {
public:
  struct Edge final {
    char letter;
    // We use uint32_t instead of size_t for space efficiency and because
    // we know that our dictionary is small enough to not overflow (it won't have more than 0xFFFFFFFF nodes).
    std::uint32_t next; // offset from the root node.
  };
  struct Node final {
    /**
     * @brief A way to represent the state of a node, in order to reduce redundancies of nodes with the same state
     */
    [[nodiscard]] std::string state() const {
      std::string out{static_cast<char>(final)};
      for (auto [letter, next_idx] : edges) {
        out += letter;
        out += std::to_string(next_idx);
      }
      return out;
    }
    std::vector<Edge> edges;
    bool final{false};
  };

  Dawg() = default;

  explicit Dawg(const std::filesystem::path& dict);

  [[nodiscard]] const Node* search_edges_for(const Node* node, char letter) const {
    if (auto it = std::ranges::find(node->edges, letter, &Edge::letter); it != node->edges.end()) {
      return &all_nodes_[it->next];
    }
    return nullptr;
  }

  [[nodiscard]] const Node* lookup_prefix(const auto& query) const {
    const Node *current = root();
    for (char ch : query) {
      if (const Node *next = search_edges_for(current, ch); next != nullptr) {
        current = next;
      } else {
        return nullptr;
      }
    }
    return current;
  }

  [[nodiscard]] bool word_exists(const auto& query) const {
    if(const auto *result = lookup_prefix(query); result != nullptr) {
      return result->final;
    }
    return false;
  }

  [[nodiscard]] const Node* root() const {return all_nodes_.data();}

  [[nodiscard]] const Node* node_at_index(std::uint32_t index) const {
    assert(index <= all_nodes_.size() - 1);
    return &all_nodes_[index];
  }

private:
  // Needed for serialization
  friend class glz::meta<Dawg>;

  using StateToIndex = std::unordered_map<std::string, std::uint32_t>;

  Dawg(const std::filesystem::path &dict, StateToIndex all_states);

  std::uint32_t get_last_state_idx(std::string_view common_prefix);

  void replace_or_register(std::uint32_t last_state_idx, StateToIndex &all_states);

  void add_suffix(std::uint32_t state_idx, std::string_view suffix);

  std::vector<Node> all_nodes_;
};

// Serialization instructions
template <> struct glz::meta<Dawg> {
  static constexpr auto value = glz::object("Dawg", &Dawg::all_nodes_);
};
#endif // DAWG_H
