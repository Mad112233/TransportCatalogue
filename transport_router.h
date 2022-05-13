#pragma once

#include "graph.h"

#include <deque>
#include <string_view>
#include <unordered_map>

namespace router {
	struct Edge {
		std::string_view bus;
		std::string_view stop;
		double span_count;
		double weight;
	};

	class TransportRouter {
	public:
		TransportRouter(double bus_wait_time, double bus_velocity) :bus_wait_time_(bus_wait_time), bus_velocity_(bus_velocity) {}

		graph::Edge<double>& AddEdge(std::string_view from, std::string_view to, double weight);
		double GetBusWaitTime()const;
		double GetBusVelocity()const;
		size_t GetIdStops(std::string_view stop)const;
		void SetEdgeId(graph::EdgeId edge_id, std::string_view bus, std::string_view stop, double span_count, double weight);
		const Edge& GetInfoEdge(graph::EdgeId id_bus)const;
		size_t GetSizeIdStops()const;

	private:
		double bus_wait_time_;
		double bus_velocity_;
		size_t count_stops_ = 0;
		std::deque<graph::Edge<double>>edges_;
		std::unordered_map<std::string_view, size_t>id_stops_;
		std::unordered_map<graph::EdgeId, Edge>id_edge_;
	};
}