#pragma once

#include "json.h"

#include <string>
#include <vector>

namespace json {

	class DictItemContext;
	class ArrayItemContext;
	class KeyItemContext;
	class BasicContext;
	class ValueItemContext;

	class Builder {
	public:
		KeyItemContext Key(std::string str);
		BasicContext Value(Node val);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		BasicContext EndDict();
		BasicContext EndArray();
		json::Node Build();

	private:
		Node root_;
		std::vector<Node*> nodes_stack_;
		std::string key_;
		bool is_key_ = false;
	};

	class BasicContext {
	public:
		BasicContext(Builder& builder) :builder_(builder) {}

		KeyItemContext Key(std::string str);
		BasicContext Value(Node val);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		BasicContext EndDict();
		BasicContext EndArray();
		json::Node Build();

	private:
		Builder& builder_;
	};

	class ValueItemContext :public BasicContext {
	public:
		ValueItemContext(BasicContext builder) :BasicContext(builder) {}

		BasicContext Value(Node val) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		BasicContext EndArray() = delete;
		json::Node Build() = delete;
	};

	class DictItemContext :public BasicContext {
	public:
		DictItemContext(BasicContext builder) :BasicContext(builder) {}

		BasicContext Value(Node val) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		BasicContext EndArray() = delete;
		json::Node Build() = delete;
	};

	class ArrayItemContext :public BasicContext {
	public:
		ArrayItemContext(BasicContext builder) :BasicContext(builder) {}

		ArrayItemContext Value(Node val);
		KeyItemContext Key(std::string str) = delete;
		BasicContext EndDict() = delete;
		json::Node Build() = delete;
	};

	class KeyItemContext :public BasicContext {
	public:
		KeyItemContext(BasicContext builder) :BasicContext(builder) {}

		ValueItemContext Value(Node val);
		KeyItemContext Key(std::string str) = delete;
		BasicContext EndDict() = delete;
		BasicContext EndArray() = delete;
		json::Node Build() = delete;
	};
}