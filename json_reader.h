#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "router.h"

#include <iostream>

namespace renderer {
	void LoadJSON(catalogue::TransportCatalogue& transport_catalogue, std::istream& input);
	void ProcessRequests(catalogue::TransportCatalogue& transport_catalogue, std::istream& input);
	void BuildingCatalog(catalogue::TransportCatalogue& transport_catalogue, json::Document& doc);
	void PrintAnswer(catalogue::TransportCatalogue& transport_catalogue, json::Document& doc,
		MapRenderer& map, router::TransportRouter& transport_router, graph::Router<double>& router);
}