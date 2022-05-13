#include "transport_catalogue.h"
#include "geo.h"

#include <string>
#include <utility>

using namespace std;

namespace catalogue {
	void TransportCatalogue::AddBusRing(string& str, int idx) {
		str += " >";
		string name = {};
		for (; static_cast<size_t>(idx) < str.size(); ++idx) {
			if (str[idx] != '>')
				name += str[idx];
			else {
				name.pop_back();
				list_buses_for_stop_[name].insert(buses_.back().number_bus);
				buses_.back().route.push_back(move(name));
				name.clear();
				++idx;
			}
		}
	}

	void TransportCatalogue::AddBusStraight(string& str, int idx) {
		str += " `";
		string name = {};
		for (; static_cast<size_t>(idx) < str.size(); ++idx) {
			if (str[idx] != '`')
				name += str[idx];
			else {
				name.pop_back();
				list_buses_for_stop_[name].insert(buses_.back().number_bus);
				buses_.back().route.push_back(move(name));
				name.clear();
				++idx;
			}
		}
		const int size = static_cast<int>(buses_.back().route.size());
		for (int i = size - 2; i >= 0; --i)
			buses_.back().route.push_back(buses_.back().route[i]);
	}

	void TransportCatalogue::ParseBus(string& str) {
		string number = {};
		int idx = 0;
		while (str[idx] != ':')
			number += str[idx++];
		bool flag = static_cast<int>(str.find('>')) == -1;
		buses_.push_back({ number,!flag });
		pointer_bus_[buses_.back().number_bus] = &buses_.back();
		idx += 2;
		if (flag)
			AddBusStraight(str, idx);
		else
			AddBusRing(str, idx);
	}

	void TransportCatalogue::SetDistanceBetweenStops(string& str, string& name, int idx, int right_idx) {
		while (str[right_idx] == '~') {
			idx = right_idx + 2;
			right_idx = str.find('m', idx);
			const string distance(str.begin() + idx, str.begin() + right_idx);
			idx = right_idx + 2;
			right_idx = str.find('~', idx);
			if (right_idx == -1)
				right_idx = str.size();
			const string to_stop(str.begin() + idx, str.begin() + right_idx);
			distance_[{name, to_stop}] = stod(distance);
		}
	}

	void TransportCatalogue::AddStop(string& str, double lat, double lng) {
		string name = {};
		int idx = 0;
		while (str[idx] != ':')
			name += str[idx++];
		idx += 2;
		int right_idx = idx + 1;
		right_idx = str.find(',', right_idx);
		idx = right_idx + 2;
		right_idx = str.find('~', idx);
		if (right_idx == -1)
			right_idx = str.size();
		
		SetDistanceBetweenStops(str, name, idx, right_idx);
		stops_.push_back({ move(name),lat,lng });
		pointer_stop_[stops_.back().name_stop] = &stops_.back();
	}

	Bus* TransportCatalogue::FindBus(const string& number) const {
		if (pointer_bus_.count(number))
			return pointer_bus_.at(number);
		else
			return nullptr;
	}

	pair<double, double> TransportCatalogue::CompDistance(const Bus* it) {
		double geographical_distance = 0;
		double actual_distance = 0;
		for (size_t i = 0; i + 1 < it->route.size(); ++i) {
			geographical_distance += geo::ComputeDistance({ pointer_stop_[it->route[i]]->lat,pointer_stop_[it->route[i]]->lng },
				{ pointer_stop_[it->route[i + 1]]->lat,pointer_stop_[it->route[i + 1]]->lng });
			actual_distance += GetDistanceBetweenStops(it->route[i], it->route[i + 1]);
		}

		return { geographical_distance,actual_distance };
	}

	set<string_view>* TransportCatalogue::FindStop(const string& name) {
		if (pointer_stop_[name] == nullptr)
			return nullptr;
		else
			return &list_buses_for_stop_[name];
	}

	double TransportCatalogue::GetDistanceBetweenStops(const std::string& from, const std::string& to) const {
		if (distance_.count({ from,to }))
			return distance_.at({ from,to });
		else
			return distance_.at({ to,from });
	}

	deque<Bus>& TransportCatalogue::GetBuses() {
		return buses_;
	}

	deque<Stop>& TransportCatalogue::GetStops(){
		return stops_;
	}

	std::unordered_map<std::string_view, Stop*>& TransportCatalogue::GetPointerStop() {
		return pointer_stop_;
	}

	std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetPointerBus() {
		return pointer_bus_;
	}

	std::unordered_map<std::pair<std::string, std::string>, double, HashPair>& TransportCatalogue::GetDistances() {
		return distance_;
	}

	std::unordered_map<std::string, std::set<std::string_view>>& TransportCatalogue::GetListBusesForStop() {
		return list_buses_for_stop_;
	}
}