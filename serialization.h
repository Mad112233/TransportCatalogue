#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <transport_catalogue.pb.h>
#include <fstream>
#include <string>

void Serialization(const std::string& path, catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map,
	const double bus_wait_time, const double bus_velocity);
std::pair<double, double> Deserialize(const std::string& path, catalogue::TransportCatalogue& transport_catalogue,
	renderer::RenderSettingsSVG& link_settings);