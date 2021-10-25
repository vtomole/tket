#include "Mapping/LexiRoute.hpp"

#include "Mapping/MappingFrontier.hpp"

namespace tket {

LexiRoute::LexiRoute(
    const ArchitecturePtr& _architecture,
    std::shared_ptr<MappingFrontier>& _mapping_frontier)
    : architecture_(_architecture), mapping_frontier_(_mapping_frontier) {
  this->set_interacting_uids();
  // set initial logical->physical labelling
  for (const Qubit& qb : this->mapping_frontier_->circuit_.all_qubits()) {
    this->labelling_.insert({qb, qb});
    Node n(qb);
    // store which Node have been asigned to Circuit already
    if (this->architecture_->uid_exists(n)) {
      this->assigned_nodes_.insert(n);
    }
  }
}

void LexiRoute::merge_with_ancilla(const UnitID& merge, const UnitID& ancilla) {
  // get output and input vertices
  Vertex merge_v_in = this->mapping_frontier_->circuit_.get_in(merge);
  Vertex merge_v_out = this->mapping_frontier_->circuit_.get_out(merge);
  Vertex ancilla_v_out = this->mapping_frontier_->circuit_.get_out(ancilla);
  // find source vertex & port of merge_v_out
  // output vertex, so can assume single edge
  Edge merge_out_edge =
      this->mapping_frontier_->circuit_.get_nth_out_edge(merge_v_in, 0);
  Edge ancilla_in_edge =
      this->mapping_frontier_->circuit_.get_nth_in_edge(ancilla_v_out, 0);
  // Find port number
  port_t merge_target_port =
      this->mapping_frontier_->circuit_.get_target_port(merge_out_edge);
  port_t ancilla_source_port =
      this->mapping_frontier_->circuit_.get_source_port(ancilla_in_edge);
  // Find vertices
  Vertex merge_v_target =
      this->mapping_frontier_->circuit_.target(merge_out_edge);
  Vertex ancilla_v_source =
      this->mapping_frontier_->circuit_.source(ancilla_in_edge);

  // remove and replace edges
  this->mapping_frontier_->circuit_.remove_edge(merge_out_edge);
  this->mapping_frontier_->circuit_.remove_edge(ancilla_in_edge);
  this->mapping_frontier_->circuit_.add_edge(
      {ancilla_v_source, ancilla_source_port},
      {merge_v_target, merge_target_port}, EdgeType::Quantum);

  // instead of manually updating all boundaries, we change which output vertex
  // the qubit paths to
  Edge merge_in_edge =
      this->mapping_frontier_->circuit_.get_nth_in_edge(merge_v_out, 0);
  port_t merge_source_port =
      this->mapping_frontier_->circuit_.get_source_port(merge_in_edge);
  Vertex merge_v_source =
      this->mapping_frontier_->circuit_.source(merge_in_edge);

  this->mapping_frontier_->circuit_.remove_edge(merge_in_edge);
  this->mapping_frontier_->circuit_.add_edge(
      {merge_v_source, merge_source_port}, {ancilla_v_out, 0},
      EdgeType::Quantum);

  // remove empty vertex wire, relabel dag vertices
  this->mapping_frontier_->circuit_.dag[merge_v_in].op =
      get_op_ptr(OpType::noop);
  this->mapping_frontier_->circuit_.dag[merge_v_out].op =
      get_op_ptr(OpType::noop);
  this->mapping_frontier_->circuit_.remove_vertex(
      merge_v_in, Circuit::GraphRewiring::No, Circuit::VertexDeletion::Yes);
  this->mapping_frontier_->circuit_.remove_vertex(
      merge_v_out, Circuit::GraphRewiring::No, Circuit::VertexDeletion::Yes);

  // Can now just erase "merge" qubit from the circuit
  this->mapping_frontier_->circuit_.boundary.get<TagID>().erase(merge);

  if (this->mapping_frontier_->circuit_.unit_bimaps_.initial) {
    this->mapping_frontier_->circuit_.unit_bimaps_.initial->right.erase(merge);
  }
  if (this->mapping_frontier_->circuit_.unit_bimaps_.final) {
    this->mapping_frontier_->circuit_.unit_bimaps_.final->right.erase(merge);
  }
}

bool LexiRoute::assign_at_distance(
    const UnitID& assignee, const Node& root, unsigned distances) {
  node_set_t valid_nodes;
  for (const Node& neighbour :
       this->architecture_->uids_at_distance(root, distances)) {
    if (this->assigned_nodes_.find(neighbour) == this->assigned_nodes_.end() ||
        this->mapping_frontier_->ancilla_nodes_.find(neighbour) !=
            this->mapping_frontier_->ancilla_nodes_.end()) {
      valid_nodes.insert(neighbour);
    }
  }
  if (valid_nodes.size() == 1) {
    auto it = valid_nodes.begin();
    if (this->mapping_frontier_->ancilla_nodes_.find(*it) !=
        this->mapping_frontier_->ancilla_nodes_.end()) {
      // => node *it is already present in circuit, but as an ancilla
      this->merge_with_ancilla(assignee, *it);
      this->mapping_frontier_->ancilla_nodes_.erase(*it);
      this->labelling_.erase(*it);
      this->labelling_[assignee] = *it;
    } else {
      this->labelling_[assignee] = *it;
      this->assigned_nodes_.insert(*it);
    }
    return true;
  }
  if (valid_nodes.size() > 1) {
    auto it = valid_nodes.begin();
    lexicographical_distances_t winning_distances =
        this->architecture_->get_distances(*it);
    Node preserved_node = *it;
    ++it;
    for (; it != valid_nodes.end(); ++it) {
      lexicographical_distances_t comparison_distances =
          this->architecture_->get_distances(*it);
      if (comparison_distances < winning_distances) {
        preserved_node = *it;
        winning_distances = comparison_distances;
      }
    }
    if (this->mapping_frontier_->ancilla_nodes_.find(preserved_node) !=
        this->mapping_frontier_->ancilla_nodes_.end()) {
      // => node *it is already present in circuit, but as an ancilla
      this->merge_with_ancilla(assignee, preserved_node);
      this->mapping_frontier_->ancilla_nodes_.erase(preserved_node);
      this->labelling_.erase(preserved_node);
      this->labelling_[assignee] = preserved_node;
    } else {
      // add ancilla case
      this->labelling_[assignee] = preserved_node;
      this->assigned_nodes_.insert(preserved_node);
    }
    return true;
  }
  return false;
}

bool LexiRoute::update_labelling() {
  // iterate through interacting qubits, assigning them to an Architecture Node
  // if they aren't already
  bool relabelled = false;
  for (const auto& pair : this->interacting_uids_) {
    bool uid_0_exist =
        this->architecture_->uid_exists(Node(this->labelling_[pair.first]));
    bool uid_1_exist =
        this->architecture_->uid_exists(Node(this->labelling_[pair.second]));
    if (!uid_0_exist || !uid_1_exist) {
      relabelled = true;
    }
    if (!uid_0_exist && !uid_1_exist) {
      // Place one on free unassigned qubit
      // Then place second later
      // condition => No ancilla qubits assigned, so don't checl
      if (this->assigned_nodes_.size() == 0) {
        // find nodes with best averaged distance to other nodes
        // place it there...
        std::set<Node> max_degree_uids = this->architecture_->max_degree_uids();
        auto it = max_degree_uids.begin();
        lexicographical_distances_t winning_distances =
            this->architecture_->get_distances(*it);
        Node preserved_node = Node(*it);
        ++it;
        for (; it != max_degree_uids.end(); ++it) {
          lexicographical_distances_t comparison_distances =
              this->architecture_->get_distances(*it);
          if (comparison_distances < winning_distances) {
            preserved_node = Node(*it);
            winning_distances = comparison_distances;
          }
        }
        this->labelling_[pair.first] = preserved_node;
        this->assigned_nodes_.insert(preserved_node);
        uid_0_exist = true;
        // given best node, do something
      } else {
        auto root_it = this->assigned_nodes_.begin();
        while (!uid_0_exist && root_it != this->assigned_nodes_.end()) {
          Node root = *root_it;
          uid_0_exist = this->assign_at_distance(pair.first, root, 1);
          ++root_it;
        }
        if (!uid_0_exist) {
          throw LexiRouteError(
              "Unable to assign physical qubit - no free qubits remaining.");
        }
      }
    }
    if (!uid_0_exist && uid_1_exist) {
      Node root(this->labelling_[pair.second]);
      for (unsigned k = 1; k <= this->architecture_->get_diameter(); k++) {
        uid_0_exist = this->assign_at_distance(pair.first, root, k);
        if (uid_0_exist) {
          break;
        }
      }
      if (!uid_0_exist) {
        throw LexiRouteError(
            "Unable to assign physical qubit - no free qubits remaining.");
      }
    }
    if (uid_0_exist && !uid_1_exist) {
      Node root(this->labelling_[pair.first]);
      for (unsigned k = 1; k <= this->architecture_->get_diameter(); k++) {
        uid_1_exist = this->assign_at_distance(pair.second, root, k);
        if (uid_1_exist) {
          break;
        }
      }
      if (!uid_1_exist) {
        throw LexiRouteError(
            "Unable to assign physical qubit - no free qubits remaining.");
      }
    }
  }
  return relabelled;
}

/**
 * LexiRoute::set_interacting_uids
 * Updates this->interacting_uids_ with all "interacting" pairs
 * of UnitID in this->mapping_frontier_
 */
void LexiRoute::set_interacting_uids(bool assigned_only) {
  // return types
  this->interacting_uids_.clear();
  for (auto it =
           this->mapping_frontier_->quantum_boundary->get<TagKey>().begin();
       it != this->mapping_frontier_->quantum_boundary->get<TagKey>().end();
       ++it) {
    Edge e0 = this->mapping_frontier_->circuit_.get_nth_out_edge(
        it->second.first, it->second.second);
    Vertex v0 = this->mapping_frontier_->circuit_.target(e0);
    // should never be input vertex, so can always use in_edges
    int n_edges = this->mapping_frontier_->circuit_.n_in_edges_of_type(
        v0, EdgeType::Quantum);
    if (n_edges == 2) {
      auto jt = it;
      ++jt;
      for (;
           jt != this->mapping_frontier_->quantum_boundary->get<TagKey>().end();
           ++jt) {
        // i.e. if vertices match
        Edge e1 = this->mapping_frontier_->circuit_.get_nth_out_edge(
            jt->second.first, jt->second.second);
        Vertex v1 = this->mapping_frontier_->circuit_.target(e1);
        if (v0 == v1) {
          // we can assume a qubit will only be in one interaction
          // we can assume from how we iterate through pairs that each qubit
          // will only be found in one match
          if (!assigned_only ||
              (this->architecture_->uid_exists(Node(it->first)) &&
               this->architecture_->uid_exists(Node(jt->first)))) {
            interacting_uids_.insert({it->first, jt->first});
            interacting_uids_.insert({jt->first, it->first});
          }
        }
      }
    } else if (n_edges != 1) {
      TKET_ASSERT(!"Vertex should only have 1 or 2 edges.");
    }
  }
}

swap_set_t LexiRoute::get_candidate_swaps() {
  swap_set_t candidate_swaps;
  for (const auto& interaction : this->interacting_uids_) {
    Node assigned_first = Node(this->labelling_[interaction.first]);
    std::vector<Node> adjacent_uids_0 =
        this->architecture_->uids_at_distance(assigned_first, 1);
    if (adjacent_uids_0.size() == 0) {
      throw LexiRouteError(
          assigned_first.repr() + " has no adjacent Node in Architecture.");
    }
    for (const Node& neighbour : adjacent_uids_0) {
      if (candidate_swaps.find({neighbour, assigned_first}) ==
          candidate_swaps.end()) {
        candidate_swaps.insert({assigned_first, neighbour});
      }
    }
    Node assigned_second = Node(this->labelling_[interaction.second]);
    std::vector<Node> adjacent_uids_1 =
        this->architecture_->uids_at_distance(assigned_second, 1);
    if (adjacent_uids_1.size() == 0) {
      throw LexiRouteError(
          assigned_first.repr() + " has no adjacent Node in Architecture.");
    }
    for (const Node& neighbour : adjacent_uids_1) {
      if (candidate_swaps.find({neighbour, assigned_second}) ==
          candidate_swaps.end()) {
        candidate_swaps.insert({assigned_second, neighbour});
      }
    }
  }
  return candidate_swaps;
}

bool is_vertex_CX(const Circuit& circ_, const Vertex& v) {
  OpType ot = circ_.get_OpType_from_Vertex(v);
  if (ot != OpType::CX) {
    if (ot == OpType::Conditional) {
      const Conditional& b =
          static_cast<const Conditional&>(*circ_.get_Op_ptr_from_Vertex(v));
      if (b.get_op()->get_type() != OpType::CX) {
        return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

std::pair<bool, bool> LexiRoute::check_bridge(
    const std::pair<Node, Node>& swap, unsigned lookahead) {
  std::pair<bool, bool> output = {false, false};
  // first confirm whether it even has an interaction
  auto it = this->interacting_uids_.find(swap.first);
  if (it != this->interacting_uids_.end()) {  // => in interaction
    if (this->architecture_->get_distance(swap.first, Node(it->second)) ==
        2) {  // => could be bridge
      // below should always return correct object given prior checks
      VertPort vp =
          (*this->mapping_frontier_->quantum_boundary->find(swap.first)).second;
      Edge out_edge = this->mapping_frontier_->circuit_.get_nth_out_edge(
          vp.first, vp.second);
      output.first = is_vertex_CX(
          this->mapping_frontier_->circuit_,
          this->mapping_frontier_->circuit_.target(out_edge));
    }
  }
  // repeat for second swap
  it = this->interacting_uids_.find(swap.second);
  if (it != this->interacting_uids_.end()) {
    if (this->architecture_->get_distance(swap.second, Node(it->second)) == 2) {
      VertPort vp =
          (*this->mapping_frontier_->quantum_boundary->find(swap.second))
              .second;
      Edge out_edge = this->mapping_frontier_->circuit_.get_nth_out_edge(
          vp.first, vp.second);
      output.second = is_vertex_CX(
          this->mapping_frontier_->circuit_,
          this->mapping_frontier_->circuit_.target(out_edge));
    }
  }
  if ((output.first && output.second) || (!output.first && !output.second)) {
    return {0, 0};
  }
  // implies conditions are set to at least check if BRIDGE is better
  swap_set_t candidate_swaps = {
      swap,
      {swap.first,
       swap.first}};  // second swap here will just compare the base case

  // as with best swap finder, we create a set of candidate swap gates and
  // then find best, except with only 2 swap (best swap and no swap)
  while (candidate_swaps.size() > 1 /*some lookahead parameter*/) {
    this->mapping_frontier_->advance_next_2qb_slice(lookahead);
    // true bool means it only sets interacting uids if both uids are in
    // architecture
    this->set_interacting_uids(true);
    // if 0, just take first swap rather than place
    if (this->interacting_uids_.size() == 0) {
      candidate_swaps = {*candidate_swaps.begin()};
    } else {
      interacting_nodes_t convert_uids;
      for (const auto& p : this->interacting_uids_) {
        convert_uids.insert(
            {Node(this->labelling_[p.first]),
             Node(this->labelling_[p.second])});
      }
      LexicographicalComparison lookahead_lc(this->architecture_, convert_uids);
      lookahead_lc.remove_swaps_lexicographical(candidate_swaps);
    }
  }
  // condition implies bridge is chosen
  // if both remained then lexicographically equivalent under given conditions
  // so either can be added with same consequences (for given hyper
  // parameters)
  if (*candidate_swaps.begin() == swap) {
    output = {0, 0};
  }
  return output;
}

// Returns the distance between n1 and p1 and the distance between n2 and p2,
// distance ordered (greatest first)
const std::pair<size_t, size_t> LexiRoute::pair_distances(
    const Node& p0_first, const Node& p0_second, const Node& p1_first,
    const Node& p1_second) const {
  if (!this->architecture_->uid_exists(p0_first) ||
      !this->architecture_->uid_exists(p0_second) ||
      !this->architecture_->uid_exists(p1_first) ||
      !this->architecture_->uid_exists(p1_second)) {
    throw LexiRouteError(
        "Node passed to LexiRoute::pair_distances not in architecture.");
  }
  size_t curr_dist1 = this->architecture_->get_distance(p0_first, p0_second);
  size_t curr_dist2 = this->architecture_->get_distance(p1_first, p1_second);
  return (curr_dist1 > curr_dist2) ? std::make_pair(curr_dist1, curr_dist2)
                                   : std::make_pair(curr_dist2, curr_dist1);
}

void LexiRoute::remove_swaps_decreasing(swap_set_t& swaps) {
  swap_set_t remaining_swaps;
  Node pair_first, pair_second;
  for (const auto& swap : swaps) {
    auto it = this->interacting_uids_.find(swap.first);
    if (it != this->interacting_uids_.end()) {
      pair_first = Node(it->second);
    } else {
      pair_first = swap.first;
    }
    if (pair_first == swap.second) {
      continue;
    }
    auto jt = this->interacting_uids_.find(swap.second);
    if (jt != this->interacting_uids_.end()) {
      pair_second = Node(jt->second);
    } else {
      pair_second = swap.second;
    }
    if (pair_second == swap.first) {
      continue;
    }

    const std::pair<size_t, size_t>& curr_dists =
        this->pair_distances(swap.first, pair_first, swap.second, pair_second);
    const std::pair<size_t, size_t>& news_dists =
        this->pair_distances(swap.second, pair_first, swap.first, pair_second);
    if (news_dists >= curr_dists) {
      continue;
    }
    remaining_swaps.insert(swap);
  }
}

void LexiRoute::solve(unsigned lookahead) {
  // store a copy of the original this->mapping_frontier_->quantum_boundray
  // this object will be updated and reset throughout the swap picking procedure
  // so need to return it to original setting at end
  unit_vertport_frontier_t copy;
  for (const std::pair<UnitID, VertPort>& pair :
       this->mapping_frontier_->quantum_boundary->get<TagKey>()) {
    copy.insert({pair.first, pair.second});
  }
  // some Qubits in boundary of this->mapping_frontier_->circuit_ may not be
  // this->architecture_ Node If true, assign physical meaning by replacing with
  // Node from this->architecture_
  // "candidate_swaps" are connected pairs of Node in this->architecture_ s.t.
  // at least one is in an "interaction" and both are "assigned" i.e. present in
  // this->mapping_frontier_->circuit

  bool updated = this->update_labelling();
  if (updated) {
    // update unit id at boundary in case of relabelling
    this->mapping_frontier_->update_quantum_boundary_uids(this->labelling_);
    return;
  }
  swap_set_t candidate_swaps = this->get_candidate_swaps();
  this->remove_swaps_decreasing(candidate_swaps);
  // Only want to substitute a single swap
  // check next layer of interacting qubits and remove swaps until only one
  // lexicographically superior swap is left
  unsigned counter = 0;
  while (candidate_swaps.size() > 1 && counter < lookahead) {
    // if 0, just take first swap rather than place
    if (this->interacting_uids_.size() == 0) {
      break;
    } else {
      interacting_nodes_t convert_uids;
      for (const auto& p : this->interacting_uids_) {
        convert_uids.insert(
            {Node(this->labelling_[p.first]),
             Node(this->labelling_[p.second])});
      }
      LexicographicalComparison lookahead_lc(this->architecture_, convert_uids);
      lookahead_lc.remove_swaps_lexicographical(candidate_swaps);
    }
    counter++;
    this->mapping_frontier_->advance_next_2qb_slice(lookahead);
    // true bool means it only sets interacting uids if both uids are in
    // architecture
    this->set_interacting_uids(true);
  }

  auto it = candidate_swaps.end();
  --it;
  std::pair<Node, Node> chosen_swap = *it;
  this->mapping_frontier_->set_quantum_boundary(copy);

  this->set_interacting_uids();
  std::pair<bool, bool> check = this->check_bridge(chosen_swap, lookahead);

  // set for final time, to allow gates to be correctly inserted, but then leave
  // as is
  this->mapping_frontier_->set_quantum_boundary(copy);
  if (!check.first && !check.second) {
    // update circuit with new swap
    // final_labelling is initial labelling permuted by single swap
    this->mapping_frontier_->add_swap(chosen_swap.first, chosen_swap.second);
  } else {
    // only need to reset in bridge case
    this->set_interacting_uids();
    if (check.first) {
      Node target = Node(this->interacting_uids_[chosen_swap.first]);
      auto path = this->architecture_->get_path(chosen_swap.first, target);
      // does path include root and target?
      Node central = Node(path[1]);
      this->mapping_frontier_->add_bridge(chosen_swap.first, central, target);
    }
    if (check.second) {
      Node target = Node(this->interacting_uids_[chosen_swap.second]);
      auto path = this->architecture_->get_path(chosen_swap.second, target);
      // does path include root and target?
      Node central = Node(path[1]);
      this->mapping_frontier_->add_bridge(chosen_swap.second, central, target);
    }
  }
  // TODO: Refactor the following to happen during add_swap and add_bridge
  // methods
  if (copy.size() < this->mapping_frontier_->quantum_boundary->size()) {
    // implies ancilla qubit is added
    // find ancilla qubit, find swap vertex and port by looking at boundary,
    // store in ancillas type
    for (auto it =
             this->mapping_frontier_->quantum_boundary->get<TagKey>().begin();
         it != this->mapping_frontier_->quantum_boundary->get<TagKey>().end();
         ++it) {
      bool match = false;
      for (auto jt = copy.get<TagKey>().begin(); jt != copy.get<TagKey>().end();
           ++jt) {
        if (it->first == jt->first) {
          match = true;
          break;
        }
      }
      if (!match) {
        // extra will be added in it
        // This is same condition as SWAP case, which means "Ancilla" has
        // already moved to a new physical node
        if (!check.first && !check.second) {
          if (Node(it->first) == chosen_swap.first) {
            this->mapping_frontier_->ancilla_nodes_.insert(chosen_swap.second);
          } else {
            this->mapping_frontier_->ancilla_nodes_.insert(chosen_swap.first);
          }
        } else {
          this->mapping_frontier_->ancilla_nodes_.insert(Node(it->first));
        }
        break;
      }
    }
  }
  return;
}

LexiRouteRoutingMethod::LexiRouteRoutingMethod(unsigned _max_depth)
    : max_depth_(_max_depth){};

bool LexiRouteRoutingMethod::check_method(
    const std::shared_ptr<MappingFrontier>& /*mapping_frontier*/,
    const ArchitecturePtr& /*architecture*/) const {
  return true;
}

unit_map_t LexiRouteRoutingMethod::routing_method(
    std::shared_ptr<MappingFrontier>& mapping_frontier,
    const ArchitecturePtr& architecture) const {
  LexiRoute lr(architecture, mapping_frontier);
  lr.solve(this->max_depth_);
  return {};
}

}  // namespace tket