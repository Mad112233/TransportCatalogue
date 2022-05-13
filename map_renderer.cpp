#include "map_renderer.h"

#include <sstream>
#include <utility>
#include <set>
#include <string_view>

using namespace std;

namespace renderer {
    void MapRenderer::SetRenderSettingsSVG(json::Document& doc) {
        const auto& map = doc.GetRoot().AsMap().at("render_settings").AsMap();

        settings_svg_.width = map.at("width").Asdouble();
        settings_svg_.height = map.at("height").Asdouble();

        settings_svg_.padding = map.at("padding").Asdouble();

        settings_svg_.line_width = map.at("line_width").Asdouble();
        settings_svg_.stop_radius = map.at("stop_radius").Asdouble();

        settings_svg_.bus_label_font_size = map.at("bus_label_font_size").AsInt();
        settings_svg_.bus_label_offset = { map.at("bus_label_offset").AsArray()[0].Asdouble(),map.at("bus_label_offset").AsArray()[1].Asdouble() };

        settings_svg_.stop_label_font_size = map.at("stop_label_font_size").AsInt();
        settings_svg_.stop_label_offset = { map.at("stop_label_offset").AsArray()[0].Asdouble(),map.at("stop_label_offset").AsArray()[1].Asdouble() };

        if (map.at("underlayer_color").IsString())
            settings_svg_.underlayer_color = map.at("underlayer_color").AsString();
        else if (map.at("underlayer_color").IsArray()) {
            const auto& arr = map.at("underlayer_color").AsArray();
            if (arr.size() == 3)
                settings_svg_.underlayer_color = move(svg::Rgb{ static_cast<uint8_t>(arr[0].AsInt()), static_cast<uint8_t>(arr[1].AsInt()), static_cast<uint8_t>(arr[2].AsInt()) });
            else
                settings_svg_.underlayer_color = move(svg::Rgba{ static_cast<uint8_t>(arr[0].AsInt()), static_cast<uint8_t>(arr[1].AsInt()),
                    static_cast<uint8_t>(arr[2].AsInt()), arr[3].Asdouble() });
        }
        settings_svg_.underlayer_width = map.at("underlayer_width").Asdouble();

        const auto& arr_color = map.at("color_palette").AsArray();
        settings_svg_.color_palette.reserve(arr_color.size());

        for (const auto& node : arr_color) {
            if (node.IsString())
                settings_svg_.color_palette.push_back(node.AsString());
            else {
                const auto& arr = node.AsArray();
                if (arr.size() == 3)
                    settings_svg_.color_palette.push_back(move(svg::Rgb{ static_cast<uint8_t>(arr[0].AsInt()), static_cast<uint8_t>(arr[1].AsInt()), static_cast<uint8_t>(arr[2].AsInt()) }));
                else
                    settings_svg_.color_palette.push_back(move(svg::Rgba{ static_cast<uint8_t>(arr[0].AsInt()), static_cast<uint8_t>(arr[1].AsInt()),
                        static_cast<uint8_t>(arr[2].AsInt()), arr[3].Asdouble() }));
            }
        }
    }

    string MapRenderer::BuildingMap(catalogue::TransportCatalogue& transport_catalogue) {
        vector<catalogue::Stop*>stops;
        stops.reserve(transport_catalogue.GetStops().size());
        for (auto &stop : transport_catalogue.GetStops())
            if (!transport_catalogue.FindStop(stop.name_stop)->empty())
                stops.push_back(&stop);
        sort(stops.begin(), stops.end(), [](const catalogue::Stop* lhs, const catalogue::Stop* rhs) {
            return lhs->name_stop < rhs->name_stop;
            });
        
        vector<catalogue::Bus*>buses;
        buses.reserve(transport_catalogue.GetBuses().size());
        for (auto& bus : transport_catalogue.GetBuses())
            if (!bus.route.empty())
                buses.push_back(&bus);
        sort(buses.begin(), buses.end(), [](catalogue::Bus* lhs, catalogue::Bus* rhs) {
            return (*lhs).number_bus < (*rhs).number_bus;
            });

        SphereProjector sphere_projector(stops.begin(), stops.end(),
            settings_svg_.width, settings_svg_.height, settings_svg_.padding);
        
        AddPolyline(transport_catalogue, sphere_projector, buses);
        AddRouteNames(transport_catalogue, sphere_projector, buses);
        AddCircle(transport_catalogue, sphere_projector, stops);
        AddNameStops(transport_catalogue, sphere_projector, stops);

        ostringstream oss;
        doc_svg_.Render(oss);

        return move(oss.str());
    }

    void MapRenderer::AddPolyline(catalogue::TransportCatalogue& transport_catalogue, SphereProjector& sphere_projector, vector<catalogue::Bus*>&buses) {
        int j = 0;
        const int mod = settings_svg_.color_palette.size();

        for (const auto&bus : buses) {
            svg::Polyline polyline;

            for (const auto& stop : bus->route) {
                polyline.AddPoint(sphere_projector({ transport_catalogue.GetPointerStop().at(stop)->lat,
                    transport_catalogue.GetPointerStop().at(stop)->lng }));
            }
            doc_svg_.Add(polyline.SetStrokeColor(settings_svg_.color_palette[j % mod]).SetFillColor("none"s)
                .SetStrokeWidth(settings_svg_.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            ++j;
        }
    }

    void MapRenderer::AddRouteNames(catalogue::TransportCatalogue& transport_catalogue, SphereProjector& sphere_projector, vector<catalogue::Bus*>& buses) {
        int j = -1;
        const int mod = settings_svg_.color_palette.size();

        for (const auto&bus : buses) {
            svg::Text text_background;
            svg::Text text;
            ++j;

            text_background.SetPosition(sphere_projector({ transport_catalogue.GetPointerStop().at(bus->route[0])->lat,transport_catalogue.GetPointerStop().at(bus->route[0])->lng }))
                .SetOffset({ settings_svg_.bus_label_offset.first,settings_svg_.bus_label_offset.second })
                .SetFontSize(settings_svg_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->number_bus)
                .SetFillColor(settings_svg_.underlayer_color).SetStrokeColor(settings_svg_.underlayer_color)
                .SetStrokeWidth(settings_svg_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            text.SetPosition(sphere_projector({ transport_catalogue.GetPointerStop().at(bus->route[0])->lat,transport_catalogue.GetPointerStop().at(bus->route[0])->lng }))
                .SetOffset({ settings_svg_.bus_label_offset.first,settings_svg_.bus_label_offset.second })
                .SetFontSize(settings_svg_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->number_bus)
                .SetFillColor(settings_svg_.color_palette[j % mod]);

            doc_svg_.Add(text_background);
            doc_svg_.Add(text);

            if (bus->route.size() != 1 && !bus->is_roundtrip) {
                const int finish = static_cast<int>(bus->route.size()) / 2;

                if (bus->route[0] == bus->route[finish])
                    continue;

                text_background.SetPosition(sphere_projector({ transport_catalogue.GetPointerStop().at(bus->route[finish])->lat,transport_catalogue.GetPointerStop().at(bus->route[finish])->lng }))
                    .SetOffset({ settings_svg_.bus_label_offset.first,settings_svg_.bus_label_offset.second })
                    .SetFontSize(settings_svg_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->number_bus)
                    .SetFillColor(settings_svg_.underlayer_color).SetStrokeColor(settings_svg_.underlayer_color)
                    .SetStrokeWidth(settings_svg_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                text.SetPosition(sphere_projector({ transport_catalogue.GetPointerStop().at(bus->route[finish])->lat,transport_catalogue.GetPointerStop().at(bus->route[finish])->lng }))
                    .SetOffset({ settings_svg_.bus_label_offset.first,settings_svg_.bus_label_offset.second })
                    .SetFontSize(settings_svg_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(bus->number_bus)
                    .SetFillColor(settings_svg_.color_palette[j % mod]);

                doc_svg_.Add(text_background);
                doc_svg_.Add(text);
            }
        }
    }

    void MapRenderer::AddCircle(catalogue::TransportCatalogue& transport_catalogue, SphereProjector& sphere_projector, vector<catalogue::Stop*>&stops) {
        for (const auto&stop : stops) {
            svg::Circle circle;
            circle.SetCenter(sphere_projector({ transport_catalogue.GetPointerStop().at(stop->name_stop)->lat, transport_catalogue.GetPointerStop().at(stop->name_stop)->lng }))
                .SetRadius(settings_svg_.stop_radius).SetFillColor("white");
            doc_svg_.Add(circle);
        }
    }

    void MapRenderer::AddNameStops(catalogue::TransportCatalogue& transport_catalogue, SphereProjector& sphere_projector, vector<catalogue::Stop*>& stops) {
        for (const auto&stop : stops) {
            svg::Text text_background;
            svg::Text text;

            text_background.SetPosition(sphere_projector({ transport_catalogue.GetPointerStop().at(stop->name_stop)->lat,transport_catalogue.GetPointerStop().at(stop->name_stop)->lng }))
                .SetOffset({ settings_svg_.stop_label_offset.first,settings_svg_.stop_label_offset.second })
                .SetFontSize(settings_svg_.stop_label_font_size).SetFontFamily("Verdana").SetData(stop->name_stop)
                .SetFillColor(settings_svg_.underlayer_color).SetStrokeColor(settings_svg_.underlayer_color)
                .SetStrokeWidth(settings_svg_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            text.SetPosition(sphere_projector({ transport_catalogue.GetPointerStop().at(stop->name_stop)->lat,transport_catalogue.GetPointerStop().at(stop->name_stop)->lng }))
                .SetOffset({ settings_svg_.stop_label_offset.first,settings_svg_.stop_label_offset.second })
                .SetFontSize(settings_svg_.stop_label_font_size).SetFontFamily("Verdana").SetData(stop->name_stop)
                .SetFillColor("black");

            doc_svg_.Add(text_background);
            doc_svg_.Add(text);
        }
    }

    RenderSettingsSVG& MapRenderer::GetSettingsSVG() {
        return settings_svg_;
    }
}