#include "json.h"

#include <variant>
#include <utility>
#include <sstream>

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            char c;
            for (; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }

            if (c != ']')
                throw ParsingError("Invalid format!"s);
            return Node(move(result));
        }

        Node LoadNumber(istream& input) {
            if (!isdigit(input.peek()) && input.peek() != '-')
                throw ParsingError("Invalid format!"s);
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(stoi(parsed_num));
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(stod(parsed_num));
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(istream& input) {
            string line;
            char c;
            while (input.get(c)) {
                if (c == '\\') {
                    char next;
                    input.get(next);
                    if (next == '\"')
                        line += '\"';
                    else if (next == 'r')
                        line += '\r';
                    else if (next == 'n')
                        line += '\n';
                    else if (next == 't')
                        line += '\t';
                    else if (next == '\\')
                        line += '\\';
                }
                else {
                    if (c == '\"')
                        break;
                    line += c;
                }
            }

            if (c != '\"') {
                throw ParsingError("Invalid format!"s);
            }

            return Node(move(line));
        }

        Node LoadDict(istream& input) {
            Dict result;

            char c;
            for (; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }

            if (c != '}')
                throw ParsingError("Invalid format!"s);

            return Node(move(result));
        }

        Node LoadBool(istream& input) {
            string str;
            char c;
            for (int i = 0; i < 3; ++i) {
                input >> c;
                str += c;
            }
            if (str == "rue")
                return Node(true);
            input >> c;
            if (str + c == "alse")
                return Node(false);
            else
                throw ParsingError("Invalid format!"s);
        }

        Node LoadNull(istream& input) {
            string str;
            char c;
            for (int i = 0; input >> c && i < 3; ++i)
                str += c;

            if (str != "ull")
                throw ParsingError("Invalid format!"s);

            return Node(nullptr);
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 't' || c == 'f')
                return LoadBool(input);
            else if (c == 'n')
                return LoadNull(input);
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    Node::Node(nullptr_t ptr) : node_value_(ptr) {}
    Node::Node(Array arr) : node_value_(arr) {}
    Node::Node(Dict dict) : node_value_(dict) {}
    Node::Node(bool val) : node_value_(val) {}
    Node::Node(int val) : node_value_(val) {}
    Node::Node(double val) : node_value_(val) {}
    Node::Node(string str) : node_value_(str) {}

    bool Node::operator==(const Node& other)const {
        return node_value_ == other.node_value_;
    }

    bool Node::operator!=(const Node& other)const {
        return node_value_ != other.node_value_;
    }

    bool Node::IsInt()const {
        return node_value_.index() == 4;
    }

    bool Node::IsString()const {
        return node_value_.index() == 6;
    }

    bool Node::Isdouble() const {
        return node_value_.index() == 5 || node_value_.index() == 4;
    }

    bool Node::IsPuredouble() const {
        return node_value_.index() == 5;
    }

    bool Node::IsNull() const {
        return node_value_.index() == 0;
    }

    bool Node::IsBool() const {
        return node_value_.index() == 3;
    }

    bool Node::IsArray() const {
        return node_value_.index() == 1;
    }

    bool Node::IsMap() const {
        return node_value_.index() == 2;
    }

    double Node::Asdouble() const {
        if (IsPuredouble())
            return get<double>(node_value_);
        else {
            if (!IsInt())
                throw logic_error("Invalid type!");
            return static_cast<double>(AsInt());
        }
    }

    double Node::AsPuredouble()const {
        if (!IsPuredouble())
            throw logic_error("Invalid type!");
        return get<double>(node_value_);
    }

    int Node::AsInt() const {
        if (!IsInt())
            throw logic_error("Invalid type!");
        return get<int>(node_value_);
    }

    const string& Node::AsString() const {
        if (!IsString())
            throw logic_error("Invalid type!");
        return get<string>(node_value_);
    }
    bool Node::AsBool() const {
        if (!IsBool())
            throw logic_error("Invalid type!");
        return get<bool>(node_value_);
    }

    const Array& Node::AsArray() const {
        if (!IsArray())
            throw logic_error("Invalid type!");
        return get<Array>(node_value_);
    }

    const Dict& Node::AsMap() const {
        if (!IsMap())
            throw logic_error("Invalid type!");
        return get<Dict>(node_value_);
    }

    NodeValue& Node::GetNodeValue() {
        return node_value_;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    bool Document::operator==(const Document& other)const {
        return root_ == other.root_;
    }

    bool Document::operator!=(const Document& other)const {
        return root_ != other.root_;
    }

    void Print(const Document& doc, std::ostream& out) {
        (void)&doc;
        (void)&out;

        if (doc.GetRoot().IsString()) {
            string line = {};
            for (const auto c : doc.GetRoot().AsString()) {
                if (c == '"')
                    line += "\\\"";
                else if (c == '\n')
                    line += "\\n";
                else if (c == '\r')
                    line += "\\r";
                else if (c == '\t')
                    line += "\\t";
                else
                    line += c;
            }
            out << '"' + move(line) + '"';
        }
        else if (doc.GetRoot().IsInt())
            out << move(to_string(doc.GetRoot().AsInt()));
        else if (doc.GetRoot().Isdouble()) {
            string str = move(to_string(doc.GetRoot().Asdouble()));
            out << DelNull(str);
        }
        else if (doc.GetRoot().IsPuredouble()) {
            string str = move(to_string(doc.GetRoot().AsPuredouble()));
            out << DelNull(str);
        }
        else if (doc.GetRoot().IsArray()) {
            const size_t size = doc.GetRoot().AsArray().size();
            out << "[\n";
            for (size_t i = 0; i + 1 < size; ++i) {
                Print(Document(doc.GetRoot().AsArray()[i]), out);
                out << ",\n";
            }
            if (size)
                Print(Document(doc.GetRoot().AsArray()[size - 1]), out);
            out << "\n]";
        }
        else if (doc.GetRoot().IsMap()) {
            const size_t size = doc.GetRoot().AsMap().size();
            out << "{\n";
            size_t j = 1;
            for (auto it = doc.GetRoot().AsMap().begin(); j < size; ++j, ++it) {
                out << "\"" << it->first << "\": ";
                Print(Document(it->second), out);
                out << ",\n";
            }
            if (size) {
                const auto it = prev(doc.GetRoot().AsMap().end());
                out << "\"" << it->first << "\": ";
                Print(Document(it->second), out);
            }
            out << "\n}";
        }
        else if (doc.GetRoot().IsNull())
            out << "null";
        else
            out << boolalpha << doc.GetRoot().AsBool();
    }

    string& DelNull(string& str) {
        while (str.back() == '0')
            str.erase(prev(str.end()));
        if (str.back() == '.')
            str.erase(prev(str.end()));
        return str;
    }

}  // namespace json