#include "svg.h"

#include <utility>
#include <memory>
#include <cmath>

namespace svg {

    using namespace std::literals;

    void PrintColor(std::ostream& out, std::monostate) {
        out << "none";
    }

    void PrintColor(std::ostream& out, std::string& str) {
        out << str;
    }

    void PrintColor(std::ostream& out, Rgb& rgb) {
        out << "rgb(" << static_cast<int>(rgb.red) << ',' << static_cast<int>(rgb.green) << ',' << static_cast<int>(rgb.blue) << ")";
    }

    void PrintColor(std::ostream& out, Rgba& rgba) {
        out << "rgba(" << static_cast<int>(rgba.red) << ',' << static_cast<int>(rgba.green) << ',' <<
            static_cast<int>(rgba.blue) << ',' << static_cast<double>(rgba.opacity) << ")";
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        PathProps::RenderAttrs(out);
        out << "/>"sv;
    }

    //----------- Polyline --------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\"";
        if (!points_.empty()) {
            out << points_[0].x << ',' << points_[0].y;
            for (size_t i = 1; i < points_.size(); ++i)
                out << ' ' << points_[i].x << ',' << points_[i].y;
        }
        out << "\"";
        PathProps::RenderAttrs(out);
        out << "/>";
    }

    //----------- Text ---------

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        PathProps::RenderAttrs(out);
        out << " x=\"" << position_.x << "\" y=\"" << position_.y
            << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y
            << "\" font-size=\"" << font_size_;
        if (!font_family_.empty())
            out << "\" font-family=\"" << font_family_;
        if (!font_weight_.empty())
            out << "\" font-weight=\"" << font_weight_;
        out << "\">" << data_ << "</text>";
    }

    //---------- Document -----------

    // Добавляет в svg-документ объект-наследник svg::Object
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        for (auto& ptr : objects_) {
            (*ptr).Render({ out,0,2 });
        }

        out << "</svg>"sv;
    }

}  // namespace svg