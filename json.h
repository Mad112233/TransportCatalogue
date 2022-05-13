#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    using Number = std::variant<int, double>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        Node() = default;
        Node(nullptr_t ptr);
        Node(Array arr);
        Node(Dict dict);
        Node(bool val);
        Node(int val);
        Node(double val);
        Node(std::string str);

        bool operator==(const Node& other)const;
        bool operator!=(const Node& other)const;

        bool IsInt() const;
        bool Isdouble() const;
        bool IsPuredouble() const;
        bool IsNull() const;
        bool IsString() const;
        bool IsBool() const;
        bool IsArray() const;
        bool IsMap() const;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        double Asdouble() const;
        double AsPuredouble()const;
        const std::string& AsString() const;
        bool AsBool() const;

        NodeValue& GetNodeValue();

    private:
        NodeValue node_value_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other)const;
        bool operator!=(const Document& other)const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    std::string& DelNull(std::string& str);

}  // namespace json