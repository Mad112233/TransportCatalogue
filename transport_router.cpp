#include "transport_router.h"
#include "graph.h"

#include <string_view>

namespace router {
	graph::Edge<double>& TransportRouter::AddEdge(std::string_view from, std::string_view to, double weight) {
		if (!id_stops_.count(from)) {
			id_stops_[from] = count_stops_++;
		}
		if (!id_stops_.count(to)) {
			id_stops_[to] = count_stops_++;
		}		

		edges_.push_back({ id_stops_[from] ,id_stops_[to],weight});
		return edges_.back();
	}

	size_t TransportRouter::GetIdStops(std::string_view stop)const {
		if (id_stops_.count(stop))
			return id_stops_.at(stop);
		return id_stops_.size();
	}

	double TransportRouter::GetBusWaitTime()const {
		return bus_wait_time_;
	}

	double TransportRouter::GetBusVelocity()const {
		return bus_velocity_;
	}

	void TransportRouter::SetEdgeId(graph::EdgeId edge_id, std::string_view bus, std::string_view stop, double span_count, double weight) {
		id_edge_[edge_id] = { bus,stop,span_count,weight };
	}

	const Edge& TransportRouter::GetInfoEdge(graph::EdgeId id_edge)const {
		return id_edge_.at(id_edge);
	}

	size_t TransportRouter::GetSizeIdStops()const {
		return id_stops_.size();
	}
}