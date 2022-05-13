#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <transport_catalogue.pb.h>
#include <fstream>
#include <string>

void Serialize(const std::string& path, catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map,
	const double bus_wait_time, const double bus_velocity);
void SerializeBusesAndStops(catalogue::TransportCatalogue& transport_catalogue, transport_catalogue_serialize::TransportCatalogue& catalog);
void SerializeSettingsSVG(renderer::MapRenderer& map, transport_catalogue_serialize::TransportCatalogue& catalog);

std::pair<double, double> Deserialize(const std::string& path, catalogue::TransportCatalogue& transport_catalogue,
	renderer::RenderSettingsSVG& link_settings);
void DeserializeBusesAndStops(catalogue::TransportCatalogue& transport_catalogue, transport_catalogue_serialize::TransportCatalogue& catalog);
void DeserializeSettingsSVG(renderer::RenderSettingsSVG& link_settings, transport_catalogue_serialize::TransportCatalogue& catalog);