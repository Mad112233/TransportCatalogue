#include "serialization.h"

#include <utility>
#include <set>
#include <string_view>
#include <variant>

void Serialize(const std::string& path, catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map,
    const double bus_wait_time, const double bus_velocity) {

    std::ofstream fout(path, std::ios::binary);
    transport_catalogue_serialize::TransportCatalogue catalog;

    catalog.mutable_routing_settings()->set_bus_wait_time(bus_wait_time);
    catalog.mutable_routing_settings()->set_bus_velocity(bus_velocity);

    SerializeBusesAndStops(transport_catalogue, catalog);
    SerializeSettingsSVG(map, catalog);

    catalog.SerializeToOstream(&fout);
}

void SerializeBusesAndStops(catalogue::TransportCatalogue& transport_catalogue, transport_catalogue_serialize::TransportCatalogue& catalog) {
    for (auto& bus : transport_catalogue.GetBuses()) {
        transport_catalogue_serialize::Bus bus_other;
        bus_other.set_number_bus(bus.number_bus);
        bus_other.set_is_roundtrip(bus.is_roundtrip);

        for (auto& stop : bus.route) {
            bus_other.add_route(stop);
        }

        *catalog.add_buses() = bus_other;
    }

    for (auto& stop : transport_catalogue.GetStops()) {
        transport_catalogue_serialize::Stop stop_other;
        stop_other.set_name_stop(stop.name_stop);
        stop_other.set_lat(stop.lat);
        stop_other.set_lng(stop.lng);

        *catalog.add_stops() = stop_other;
    }

    for (auto& [stops, distance] : transport_catalogue.GetDistances()) {
        transport_catalogue_serialize::Distance distance_other;
        distance_other.set_first(stops.first);
        distance_other.set_last(stops.second);
        distance_other.set_distance(distance);

        *catalog.add_distance() = distance_other;
    }

    for (auto& [stop, buses] : transport_catalogue.GetListBusesForStop()) {
        transport_catalogue_serialize::BusesForStop buses_for_stop;
        buses_for_stop.set_stop(stop);

        for (auto& bus : buses) {
            *buses_for_stop.add_buses() = bus;
        }

        *catalog.add_list_buses_for_stop() = buses_for_stop;
    }
}

void SerializeSettingsSVG(renderer::MapRenderer& map, transport_catalogue_serialize::TransportCatalogue& catalog) {
    auto& link_settings = map.GetSettingsSVG();
    auto catalog_ptr = catalog.mutable_settings_svg();

    catalog_ptr->set_width(link_settings.width);
    catalog_ptr->set_height(link_settings.height);
    catalog_ptr->set_padding(link_settings.padding);
    catalog_ptr->set_line_width(link_settings.line_width);
    catalog_ptr->set_stop_radius(link_settings.stop_radius);
    catalog_ptr->set_bus_label_font_size(link_settings.bus_label_font_size);
    catalog_ptr->set_bus_label_offset_first(link_settings.bus_label_offset.first);
    catalog_ptr->set_bus_label_offset_second(link_settings.bus_label_offset.second);
    catalog_ptr->set_stop_label_font_size(link_settings.stop_label_font_size);
    catalog_ptr->set_stop_label_offset_first(link_settings.stop_label_offset.first);
    catalog_ptr->set_stop_label_offset_second(link_settings.stop_label_offset.second);

    if (link_settings.underlayer_color.index() == 1) {
        catalog_ptr->mutable_underlayer_color()->set_string_color(std::get<1>(link_settings.underlayer_color));
    }
    else if (link_settings.underlayer_color.index() == 2) {
        const auto& value = catalog_ptr->mutable_underlayer_color()->mutable_rgb_color();

        (*value).set_red(std::get<2>(link_settings.underlayer_color).red);
        (*value).set_green(std::get<2>(link_settings.underlayer_color).green);
        (*value).set_blue(std::get<2>(link_settings.underlayer_color).blue);
    }
    else if (link_settings.underlayer_color.index() == 3) {
        const auto& value = catalog_ptr->mutable_underlayer_color()->mutable_rgba_color();

        (*value).set_red(std::get<3>(link_settings.underlayer_color).red);
        (*value).set_green(std::get<3>(link_settings.underlayer_color).green);
        (*value).set_blue(std::get<3>(link_settings.underlayer_color).blue);
        (*value).set_opacity(std::get<3>(link_settings.underlayer_color).opacity);
    }
    catalog_ptr->set_underlayer_width(link_settings.underlayer_width);

    for (auto& val : link_settings.color_palette) {
        if (val.index() == 1) {
            catalog_ptr->add_color_palette()->set_string_color(std::get<1>(val));
        }
        else if (val.index() == 2) {
            const auto& value = catalog_ptr->add_color_palette()->mutable_rgb_color();

            (*value).set_red(std::get<2>(val).red);
            (*value).set_green(std::get<2>(val).green);
            (*value).set_blue(std::get<2>(val).blue);
        }
        else if (val.index() == 3) {
            const auto& value = catalog_ptr->add_color_palette()->mutable_rgba_color();

            (*value).set_red(std::get<3>(val).red);
            (*value).set_green(std::get<3>(val).green);
            (*value).set_blue(std::get<3>(val).blue);
            (*value).set_opacity(std::get<3>(val).opacity);
        }
    }
}

std::pair<double, double> Deserialize(const std::string& path, catalogue::TransportCatalogue& transport_catalogue,
    renderer::RenderSettingsSVG& link_settings) {

    std::ifstream fin(path, std::ios::binary);
    transport_catalogue_serialize::TransportCatalogue catalog;
    catalog.ParseFromIstream(&fin);

    DeserializeBusesAndStops(transport_catalogue, catalog);
    DeserializeSettingsSVG(link_settings, catalog);

    return { catalog.routing_settings().bus_wait_time() ,catalog.routing_settings().bus_velocity() };
}

void DeserializeBusesAndStops(catalogue::TransportCatalogue& transport_catalogue, transport_catalogue_serialize::TransportCatalogue& catalog) {
    for (auto& bus : catalog.buses()) {
        catalogue::Bus bus_other;
        bus_other.is_roundtrip = bus.is_roundtrip();
        bus_other.number_bus = bus.number_bus();

        for (auto& stop : bus.route()) {
            bus_other.route.push_back(stop);
        }

        transport_catalogue.GetBuses().push_back(bus_other);
        transport_catalogue.GetPointerBus()[transport_catalogue.GetBuses().back().number_bus] = &transport_catalogue.GetBuses().back();
    }

    for (auto& stop : catalog.stops()) {
        transport_catalogue.GetStops().push_back({ stop.name_stop() ,stop.lat() ,stop.lng() });
        transport_catalogue.GetPointerStop()[transport_catalogue.GetStops().back().name_stop] = &transport_catalogue.GetStops().back();
    }

    for (auto& value : catalog.distance()) {
        transport_catalogue.GetDistances()[{value.first(), value.last()}] = value.distance();
    }

    for (auto& value : catalog.list_buses_for_stop()) {
        std::set<std::string_view>buses;

        for (auto& bus : value.buses()) {
            buses.insert(transport_catalogue.GetPointerBus()[bus]->number_bus);
        }

        transport_catalogue.GetListBusesForStop()[value.stop()] = buses;
    }
}

void DeserializeSettingsSVG(renderer::RenderSettingsSVG& link_settings, transport_catalogue_serialize::TransportCatalogue& catalog) {
    const auto& catalog_link = catalog.settings_svg();

    link_settings.width = catalog_link.width();
    link_settings.height = catalog_link.height();
    link_settings.padding = catalog_link.padding();
    link_settings.line_width = catalog_link.line_width();
    link_settings.stop_radius = catalog_link.stop_radius();
    link_settings.bus_label_font_size = catalog_link.bus_label_font_size();
    link_settings.bus_label_offset.first = catalog_link.bus_label_offset_first();
    link_settings.bus_label_offset.second = catalog_link.bus_label_offset_second();
    link_settings.stop_label_font_size = catalog_link.stop_label_font_size();
    link_settings.stop_label_offset.first = catalog_link.stop_label_offset_first();
    link_settings.stop_label_offset.second = catalog_link.stop_label_offset_second();

    if (catalog_link.underlayer_color().underlayer_color_case() == transport_catalogue_serialize::Color::UnderlayerColorCase::kStringColor) {
        link_settings.underlayer_color = catalog_link.underlayer_color().string_color();
    }
    else if (catalog_link.underlayer_color().underlayer_color_case() == transport_catalogue_serialize::Color::UnderlayerColorCase::kRgbColor) {
        link_settings.underlayer_color = svg::Rgb{
            static_cast<uint8_t>(catalog_link.underlayer_color().rgb_color().red()),
            static_cast<uint8_t>(catalog_link.underlayer_color().rgb_color().green()),
            static_cast<uint8_t>(catalog_link.underlayer_color().rgb_color().blue())
        };
    }
    else if (catalog_link.underlayer_color().underlayer_color_case() == transport_catalogue_serialize::Color::UnderlayerColorCase::kRgbaColor) {
        link_settings.underlayer_color = svg::Rgba{
            static_cast<uint8_t>(catalog_link.underlayer_color().rgba_color().red()),
            static_cast<uint8_t>(catalog_link.underlayer_color().rgba_color().green()),
            static_cast<uint8_t>(catalog_link.underlayer_color().rgba_color().blue()),
            catalog_link.underlayer_color().rgba_color().opacity()
        };
    }
    link_settings.underlayer_width = catalog_link.underlayer_width();

    for (auto& color : catalog_link.color_palette()) {
        if (color.underlayer_color_case() == transport_catalogue_serialize::Color::UnderlayerColorCase::kStringColor) {
            link_settings.color_palette.push_back(color.string_color());
        }
        else if (color.underlayer_color_case() == transport_catalogue_serialize::Color::UnderlayerColorCase::kRgbColor) {
            link_settings.color_palette.push_back(svg::Rgb{
                static_cast<uint8_t>(color.rgb_color().red()),
                static_cast<uint8_t>(color.rgb_color().green()),
                static_cast<uint8_t>(color.rgb_color().blue())
                });
        }
        else if (color.underlayer_color_case() == transport_catalogue_serialize::Color::UnderlayerColorCase::kRgbaColor) {
            link_settings.color_palette.push_back(svg::Rgba{
                static_cast<uint8_t>(color.rgba_color().red()),
                static_cast<uint8_t>(color.rgba_color().green()),
                static_cast<uint8_t>(color.rgba_color().blue()),
                color.rgba_color().opacity()
                });
        }
    }
}