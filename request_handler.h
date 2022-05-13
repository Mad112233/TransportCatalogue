#pragma once

#include "transport_catalogue.h"

#include <optional>
#include <set>
#include <string_view>
#include <cstdint>
 
struct BusStat {
    double curvature = 0;
    int route_length = 0;
    int stop_count = 0;
    int unique_stop_count = 0;
};

 class RequestHandler {
 public:
     RequestHandler(catalogue::TransportCatalogue& transport_catalogue);

     // Возвращает информацию о маршруте (запрос Bus)
     std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

     // Возвращает маршруты, проходящие через
     const std::optional<std::set<std::string_view>> GetBusesByStop(const std::string_view& stop_name) const;

 private:
     // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
     catalogue::TransportCatalogue& link_catalog_;
 };