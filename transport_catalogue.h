#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <deque>
#include <functional>
#include <string_view>

namespace catalogue {
	struct Stop {
		std::string name_stop;
		double lat;
		double lng;
	};

	struct Bus {
		std::string number_bus;
		bool is_roundtrip = false;
		std::vector<std::string>route = {};
	};

	struct HashPair {
		size_t operator()(const std::pair<std::string, std::string>& val)const {
			return std::hash<std::string>{}(val.first) * 1000000007 + std::hash<std::string>{}(val.second) * 11;
		}
	};

	class TransportCatalogue {
	public:
		void ParseBus(std::string& str);
		void AddStop(std::string& str, double lat, double lng);
		void AddBusRing(std::string& str, int idx);
		void AddBusStraight(std::string& str, int idx);
		Bus* FindBus(const std::string& number)const;
		std::pair<double, double> CompDistance(const Bus* it);
		std::set<std::string_view>* FindStop(const std::string& name);
		double GetDistanceBetweenStops(const std::string& from, const std::string& to)const;
		void SetDistanceBetweenStops(std::string& str, std::string& name, int idx, int right_idx);
		std::deque<Bus>& GetBuses();
		std::deque<Stop>& GetStops();
		std::unordered_map<std::string_view, Stop*>& GetPointerStop();
		std::unordered_map<std::string_view, Bus*>& GetPointerBus();
		std::unordered_map<std::pair<std::string, std::string>, double, HashPair>& GetDistances();
		std::unordered_map<std::string, std::set<std::string_view>>& GetListBusesForStop();

	private:
		std::unordered_map<std::string_view, Stop*>pointer_stop_;
		std::unordered_map<std::string_view, Bus*>pointer_bus_;
		std::unordered_map<std::string, std::set<std::string_view>>list_buses_for_stop_;
		std::unordered_map<std::pair<std::string, std::string>, double, HashPair>distance_;
		std::deque<Bus>buses_;
		std::deque<Stop>stops_;
	};
}