#include "request_handler.h"
#include "transport_catalogue.h"
#include "geo.h"

#include <iostream>
#include <unordered_set>

using namespace std;

RequestHandler::RequestHandler(catalogue::TransportCatalogue& transport_catalogue) :link_catalog_(transport_catalogue) {}

optional<BusStat> RequestHandler::GetBusStat(const string_view& bus_name) const {
	string name(bus_name.begin(), bus_name.end());
	catalogue::Bus* it = link_catalog_.FindBus(name);
	if (!it)
		return nullopt;
	unordered_set<string>stops(it->route.begin(), it->route.end());
	const pair<double, double> distance = link_catalog_.CompDistance(it);

	BusStat bus = { distance.second / distance.first, static_cast<int>(distance.second),
		static_cast<int>(it->route.size()), static_cast<int>(stops.size()) };
	return bus;
}

const optional<set<string_view>> RequestHandler::GetBusesByStop(const string_view& stop_name) const {
	string name(stop_name.begin(), stop_name.end());
	auto it = link_catalog_.FindStop(name);
	if (!it)
		return nullopt;
	
	set<string_view>buses;
	for (auto& element : *it) {
		buses.insert(element);
	}
	return buses;
}