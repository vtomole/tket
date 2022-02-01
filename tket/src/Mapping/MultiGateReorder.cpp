#include "Mapping/MultiGateReorder.hpp"

#include "Mapping/MappingFrontier.hpp"

namespace tket {

MultiGateReorder::MultiGateReorder(
    const ArchitecturePtr &_architecture,
    std::shared_ptr<MappingFrontier> &_mapping_frontier)
    : architecture_(_architecture), mapping_frontier_(_mapping_frontier) {
  // This needs to be updated every time the frontier changes
  this->u_frontier_edges_ =
      convert_u_frontier_to_edges(*frontier_convert_vertport_to_edge(
          _mapping_frontier->circuit_, _mapping_frontier->quantum_boundary));
}

// Traverse the DAG to the quantum frontier encoded in q_boundary_map
// to find the UnitID associated with an VertPort
UnitID get_unitid_from_vertex_port(
    const Circuit &circ, const VertPort &vert_port,
    const std::map<VertPort, UnitID> &q_boundary_map) {
  VertPort current_vert_port = vert_port;
  while (true) {
    auto it = q_boundary_map.find(current_vert_port);
    if (it != q_boundary_map.end()) {
      return it->second;
    }
    Edge current_e = circ.get_nth_out_edge(
        current_vert_port.first, current_vert_port.second);
    Vertex prev_vert;
    Edge prev_e;
    std::tie(prev_vert, prev_e) =
        circ.get_prev_pair(current_vert_port.first, current_e);
    current_vert_port = {prev_vert, circ.get_source_port(prev_e)};
  }
}

bool is_multiq_quantum_gate(const Circuit &circ, const Vertex &vert) {
  Op_ptr op = circ.get_Op_ptr_from_Vertex(vert);
  return (
      op->get_desc().is_gate() && circ.n_in_edges(vert) > 1 &&
      circ.n_in_edges_of_type(vert, EdgeType::Quantum) ==
          circ.n_in_edges(vert) &&
      circ.n_out_edges_of_type(vert, EdgeType::Quantum) ==
          circ.n_out_edges(vert));
}

bool is_physically_permitted(
    const Circuit &circ, const ArchitecturePtr &arc_ptr, const Vertex &vert,
    const std::map<VertPort, UnitID> &q_boundary_map) {
  std::vector<Node> nodes;
  for (port_t port = 0; port < circ.n_ports(vert); ++port) {
    nodes.push_back(
        Node(get_unitid_from_vertex_port(circ, {vert, port}, q_boundary_map)));
  }

  return arc_ptr->valid_operation(nodes);
}

// This method will try to commute a vertex to the quantum frontier
std::optional<std::pair<EdgeVec, EdgeVec>> try_find_commute_edges(
    const Circuit &circ, const EdgeVec &frontier_edges, const Vertex &vert) {
  // Initialize to be the in_edges for the given vertex
  EdgeVec current_edges = circ.get_in_edges(vert);
  EdgeVec initial_edges(current_edges.begin(), current_edges.end());

  Op_ptr current_op = circ.get_Op_ptr_from_Vertex(vert);
  // Record the colour of each port of the vertex.
  std::vector<std::optional<Pauli>> colours;
  for (const Edge &edge : current_edges) {
    port_t target_port = circ.get_target_port(edge);
    std::optional<Pauli> colour = current_op->commuting_basis(target_port);
    colours.push_back(colour);
  }
  // Stores all edges which the vertex can be commuted to
  EdgeVec dest_edges;
  while (true) {
    // The vertex can be commuted to the front
    bool success = true;
    for (unsigned i = 0; i < current_edges.size(); ++i) {
      // Check if the edge is already in the quantum frontier
      if (std::find(
              frontier_edges.begin(), frontier_edges.end(), current_edges[i]) !=
          frontier_edges.end()) {
        dest_edges.push_back(current_edges[i]);
        continue;
      }
      // Check prev_op is a gate
      Vertex prev_vert = circ.source(current_edges[i]);
      Op_ptr prev_op = circ.get_Op_ptr_from_Vertex(prev_vert);
      if (!prev_op->get_desc().is_gate()) {
        // not commute
        return std::nullopt;
      }

      // Check commute
      port_t source_port = circ.get_source_port(current_edges[i]);
      if (!prev_op->commutes_with_basis(colours[i], source_port)) {
        // not commute
        return std::nullopt;
      } else {
        // Update dest_edges
        Vertex prev_prev_v;
        Edge prev_e;
        std::tie(prev_prev_v, prev_e) =
            circ.get_prev_pair(prev_vert, current_edges[i]);
        dest_edges.push_back(prev_e);
      }
      // Only true if all edges are in frontier
      success = false;
    }
    if (success) {
      std::pair<EdgeVec, EdgeVec> p(initial_edges, dest_edges);
      return p;
    } else {
      current_edges = dest_edges;
      dest_edges = {};
    }
  }
}

void partial_rewire(
    const Vertex &vert, Circuit &circ, EdgeVec &src_edges,
    EdgeVec &dest_edges) {
  // move the vertex to the frontier
  // Notice that if one of the vertex's in edge is already a destination
  // edge then the circuit::remove_vertex will delete the destination edge
  // hence circuit::rewire would result in an error due to the missing edge.
  // We need a partial rewire for that reason.
  // Example:
  // Moving the second vertex (CX gate) to the front we only need to rewire
  // the "x" part.
  // --o-----
  //   |
  // --x--x--
  //      |
  // -----o--

  for (unsigned i = 0; i < dest_edges.size(); i++) {
    Edge &dest_in_edge = dest_edges[i];
    Edge &curr_in_edge = src_edges[i];
    // If the vertex is already connected to an edge in the frontier, do
    // nothing.
    if (dest_in_edge != curr_in_edge) {
      // Add first edge
      Vertex dest_prev_vert = circ.source(dest_in_edge);
      circ.add_edge(
          {dest_prev_vert, circ.get_source_port(dest_in_edge)},
          {vert, circ.get_target_port(curr_in_edge)}, EdgeType::Quantum);
      // Add second edge
      Vertex curr_next_vert;
      Edge curr_out_edge;
      Vertex dest_next_vert = circ.target(dest_in_edge);
      std::tie(curr_next_vert, curr_out_edge) =
          circ.get_next_pair(vert, curr_in_edge);
      circ.add_edge(
          {vert, circ.get_source_port(curr_out_edge)},
          {dest_next_vert, circ.get_target_port(dest_in_edge)},
          EdgeType::Quantum);
      // Add third edge
      Vertex curr_prev_vert = circ.source(curr_in_edge);
      circ.add_edge(
          {curr_prev_vert, circ.get_source_port(curr_in_edge)},
          {curr_next_vert, circ.get_target_port(curr_out_edge)},
          EdgeType::Quantum);
      // Remove edges
      circ.remove_edge(dest_in_edge);
      circ.remove_edge(curr_in_edge);
      circ.remove_edge(curr_out_edge);
    }
  }
}

void MultiGateReorder::solve(unsigned max_depth, unsigned max_size) {
  // Assume the frontier has been advanced

  // store a copy of the original this->mapping_frontier_->quantum_boundray
  // this object will be updated and reset throughout the procedure
  // so need to return it to original setting at end
  // also create a map for getting UnitID from VertPort
  std::map<VertPort, UnitID> q_boundary_map;
  unit_vertport_frontier_t copy;
  for (const std::pair<UnitID, VertPort> &pair :
       this->mapping_frontier_->quantum_boundary->get<TagKey>()) {
    copy.insert({pair.first, pair.second});
    q_boundary_map.insert({pair.second, pair.first});
  }
  // Get a subcircuit only for iterating vertices
  Subcircuit circ =
      this->mapping_frontier_->get_frontier_subcircuit(max_depth, max_size);
  // since we assume that the frontier has been advanced
  // we are certain that any multi-q vert lies after the frontier
  for (const Vertex &vert : circ.verts) {
    // Check if the vertex is:
    //  1. physically permitted
    //  2. is a multi qubit quantum operation without classical controls
    if (is_multiq_quantum_gate(this->mapping_frontier_->circuit_, vert) &&
        is_physically_permitted(
            this->mapping_frontier_->circuit_, this->architecture_, vert,
            q_boundary_map)) {
      std::optional<std::pair<EdgeVec, EdgeVec>> commute_pairs =
          try_find_commute_edges(
              this->mapping_frontier_->circuit_, this->u_frontier_edges_, vert);

      if (commute_pairs != std::nullopt) {
        partial_rewire(
            vert, this->mapping_frontier_->circuit_, (*commute_pairs).first,
            (*commute_pairs).second);
        // Update the frontier
        this->mapping_frontier_->advance_frontier_boundary(this->architecture_);
        this->u_frontier_edges_ =
            convert_u_frontier_to_edges(*frontier_convert_vertport_to_edge(
                this->mapping_frontier_->circuit_,
                this->mapping_frontier_->quantum_boundary));
        // Update the map
        q_boundary_map.clear();
        for (const std::pair<UnitID, VertPort> &pair :
             this->mapping_frontier_->quantum_boundary->get<TagKey>()) {
          q_boundary_map.insert({pair.second, pair.first});
        }
      }
    }
  }
  // Return the quantum boundary to its original setting
  this->mapping_frontier_->set_quantum_boundary(copy);
}

MultiGateReorderRoutingMethod::MultiGateReorderRoutingMethod(
    unsigned _max_depth, unsigned _max_size)
    : max_depth_(_max_depth), max_size_(_max_size){};

bool MultiGateReorderRoutingMethod::check_method(
    const std::shared_ptr<MappingFrontier> &mapping_frontier,
    const ArchitecturePtr &architecture) const {
  const EdgeVec u_frontier_edges =
      convert_u_frontier_to_edges(*frontier_convert_vertport_to_edge(
          mapping_frontier->circuit_, mapping_frontier->quantum_boundary));
  std::map<VertPort, UnitID> q_boundary_map;
  for (const std::pair<UnitID, VertPort> &pair :
       mapping_frontier->quantum_boundary->get<TagKey>()) {
    q_boundary_map.insert({pair.second, pair.first});
  }

  Subcircuit circ = mapping_frontier->get_frontier_subcircuit(
      this->max_depth_, this->max_size_);
  // since we assume that the frontier has been advanced
  // we are certain that any multi-q vert lies after the frontier
  for (const Vertex &vert : circ.verts) {
    if (is_multiq_quantum_gate(mapping_frontier->circuit_, vert) &&
        is_physically_permitted(
            mapping_frontier->circuit_, architecture, vert, q_boundary_map)) {
      std::optional<std::pair<EdgeVec, EdgeVec>> commute_pairs =
          try_find_commute_edges(
              mapping_frontier->circuit_, u_frontier_edges, vert);

      if (commute_pairs != std::nullopt) {
        return true;
      }
    }
  }
  return false;
}

unit_map_t MultiGateReorderRoutingMethod::routing_method(
    std::shared_ptr<MappingFrontier> &mapping_frontier,
    const ArchitecturePtr &architecture) const {
  MultiGateReorder mr(architecture, mapping_frontier);
  mr.solve(this->max_depth_, this->max_size_);
  return {};
}

}  // namespace tket