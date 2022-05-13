#include "json_reader.h"
#include "request_handler.h"
#include "json_builder.h"
#include "graph.h"
#include "serialization.h"

#include <sstream>
#include <optional>
#include <cstdint>
#include <utility>
#include <vector>

using namespace std;

namespace renderer {
    void LoadJSON(catalogue::TransportCatalogue& transport_catalogue, istream& input) {
        json::Document doc = json::Load(input);

        BuildingCatalog(transport_catalogue, doc);

        MapRenderer map;
        map.SetRenderSettingsSVG(doc);

        Serialize(doc.GetRoot().AsMap().at("serialization_settings").AsMap().at("file").AsString(), transport_catalogue, map,
            doc.GetRoot().AsMap().at("routing_settings").AsMap().at("bus_wait_time").Asdouble(),
            doc.GetRoot().AsMap().at("routing_settings").AsMap().at("bus_velocity").Asdouble());
    }

    void ProcessRequests(catalogue::TransportCatalogue& transport_catalogue, istream& input) {
        json::Document doc = json::Load(input);
        MapRenderer map;
        
        auto value = Deserialize(doc.GetRoot().AsMap().at("serialization_settings").AsMap().at("file").AsString(),
            transport_catalogue, map.GetSettingsSVG());

        router::TransportRouter transport_router(value.first, value.second);
        graph::DirectedWeightedGraph<double> weighted_graph(transport_catalogue.GetStops().size());

        const double bus_wait_time = transport_router.GetBusWaitTime();
        const double bus_velocity = transport_router.GetBusVelocity();

        for (auto& bus : transport_catalogue.GetBuses()) {
            for (size_t i = 0; i < bus.route.size(); ++i) {
                double weight = bus_wait_time;

                for (size_t u = i + 1; u < bus.route.size(); ++u) {
                    weight += transport_catalogue.GetDistanceBetweenStops(bus.route[u - 1], bus.route[u]) /
                        1000 / bus_velocity * 60.0;

                    transport_router.SetEdgeId(weighted_graph.AddEdge(transport_router.AddEdge(bus.route[i], bus.route[u], weight)),
                        bus.number_bus, bus.route[i], u - i, move(weight));
                }
            }
        }

        graph::Router<double> router(weighted_graph);

        PrintAnswer(transport_catalogue, doc, map, transport_router, router);
    }

    void BuildingCatalog(catalogue::TransportCatalogue& transport_catalogue, json::Document& doc) {
        for (const auto& node_map : doc.GetRoot().AsMap().at("base_requests").AsArray()) {
            if (node_map.AsMap().at("type").AsString()[0] == 'S') {
                string str = node_map.AsMap().at("name").AsString() + ": " + to_string(node_map.AsMap().at("latitude").Asdouble()) + ", " +
                    to_string(node_map.AsMap().at("longitude").Asdouble());

                for (const auto& [key, val] : node_map.AsMap().at("road_distances").AsMap())
                    str += "~ " + to_string(val.AsInt()) + "m " + key;

                transport_catalogue.AddStop(str, node_map.AsMap().at("latitude").Asdouble(), node_map.AsMap().at("longitude").Asdouble());
            }
            else {
                string str = node_map.AsMap().at("name").AsString() + ": ";
                const char separator = node_map.AsMap().at("is_roundtrip").AsBool() ? '>' : '`';
                auto it = node_map.AsMap().at("stops").AsArray().begin();
                const size_t size = node_map.AsMap().at("stops").AsArray().size();

                for (size_t i = 0; i + 1 < size; ++i, ++it)
                    str += (*it).AsString() + ' ' + separator + ' ';
                str += (*prev(node_map.AsMap().at("stops").AsArray().end())).AsString();

                transport_catalogue.ParseBus(str);
            }
        }
    }

    void PrintAnswer(catalogue::TransportCatalogue& transport_catalogue, json::Document& doc,
        MapRenderer& map, router::TransportRouter& transport_router, graph::Router<double>& router) {
        json::Array arr;
        ::RequestHandler requests(transport_catalogue);

        for (const auto& node_map : doc.GetRoot().AsMap().at("stat_requests").AsArray()) {
            bool flag = false;

            if (node_map.AsMap().at("type").AsString() == "Bus") {
                if (const optional<BusStat> bus_stat = requests.GetBusStat(node_map.AsMap().at("name").AsString())) {
                    arr.push_back(json::Builder{}.StartDict()
                        .Key("stop_count").Value(bus_stat->stop_count)
                        .Key("unique_stop_count").Value(bus_stat->unique_stop_count)
                        .Key("route_length").Value(bus_stat->route_length)
                        .Key("curvature").Value(bus_stat->curvature)
                        .Key("request_id").Value(node_map.AsMap().at("id").AsInt())
                        .EndDict().Build()
                    );
                }
                else
                    flag = true;
            }
            else if (node_map.AsMap().at("type").AsString() == "Stop") {
                if (const optional<set<string_view>> stop_stat = requests.GetBusesByStop(node_map.AsMap().at("name").AsString())) {
                    json::Array arr_buses;

                    for (auto& bus : stop_stat.value())
                        arr_buses.push_back(json::Node(string(bus)));

                    arr.push_back(json::Builder{}.StartDict()
                        .Key("request_id").Value(node_map.AsMap().at("id").AsInt())
                        .Key("buses").Value(arr_buses)
                        .EndDict().Build()
                    );
                }
                else
                    flag = true;
            }
            else if (node_map.AsMap().at("type").AsString() == "Route") {
                const size_t from = transport_router.GetIdStops(node_map.AsMap().at("from").AsString());
                const size_t to = transport_router.GetIdStops(node_map.AsMap().at("to").AsString());

                const auto route = router.BuildRoute(from, to);

                if (route && from != transport_router.GetSizeIdStops()) {
                    struct Item{
                        bool wait;
                        string_view name;
                        double span_count;
                        double time;
                    };

                    const double wait_time = transport_router.GetBusWaitTime();
                    vector<Item>arr_items;

                    for (size_t i = 0; i < (*route).edges.size(); ++i) {
                        const auto& edge = transport_router.GetInfoEdge((*route).edges[i]);

                        arr_items.push_back({ true,edge.stop ,0,wait_time });
                        arr_items.push_back({ false,edge.bus, edge.span_count ,edge.weight - wait_time });
                    }

                    auto builder = json::Builder{};
                    auto json_obj = builder.StartDict()
                        .Key("request_id").Value(node_map.AsMap().at("id").AsInt())
                        .Key("total_time").Value((*route).weight)
                        .Key("items").StartArray();

                    for (auto [wait, name, cnt_stops, time_edge] : arr_items) {
                        if (wait) {
                            json_obj.StartDict()
                                .Key("type").Value("Wait"s)
                                .Key("stop_name").Value(string(name))
                                .Key("time").Value(time_edge)
                                .EndDict();
                        }
                        else {
                            json_obj.StartDict()
                                .Key("type").Value("Bus"s)
                                .Key("bus").Value(string(name))
                                .Key("span_count").Value(cnt_stops)
                                .Key("time").Value(time_edge)
                                .EndDict();
                        }
                    }

                    arr.push_back(json_obj.EndArray().EndDict().Build());
                }
                else
                    flag = true;
            }
            else {
                arr.push_back(json::Builder{}.StartDict()
                    .Key("map").Value(map.BuildingMap(transport_catalogue))
                    .Key("request_id").Value(node_map.AsMap().at("id").AsInt())
                    .EndDict().Build()
                );
            }

            if (flag) {
                arr.push_back(json::Builder{}.StartDict()
                    .Key("request_id").Value(node_map.AsMap().at("id").AsInt())
                    .Key("error_message").Value("not found"s)
                    .EndDict().Build()
                );
            }
        }

        json::Print(json::Document(arr), cout);
    }
}