#include "json_builder.h"

#include <utility>

namespace json {

	KeyItemContext Builder::Key(std::string str) {
		if (root_.GetNodeValue().index() != 0 && nodes_stack_.empty())
			throw std::logic_error("Object is ready!");

		if (root_.GetNodeValue().index() == 0 || !nodes_stack_.back()->IsMap() || is_key_)
			throw std::logic_error("You can't call the \"Key\" method now!");

		is_key_ = true;
		key_ = str;
		return KeyItemContext(*this);
	}

	BasicContext Builder::Value(Node val) {
		if (root_.GetNodeValue().index() != 0 && nodes_stack_.empty())
			throw std::logic_error("Object is ready!");

		if (nodes_stack_.empty())
			root_ = val;
		else {
			if (nodes_stack_.back()->IsMap()) {
				if (!is_key_)
					throw std::logic_error("The key is empty!");

				std::get<Dict>(nodes_stack_.back()->GetNodeValue()).insert({ key_,val });
				is_key_ = false;
			}
			else {
				std::get<Array>(nodes_stack_.back()->GetNodeValue()).push_back(val);
			}
		}

		return *this;
	}

	DictItemContext Builder::StartDict() {
		if (root_.GetNodeValue().index() != 0 && nodes_stack_.empty())
			throw std::logic_error("Object is ready!");

		if (nodes_stack_.empty()) {
			root_ = json::Dict{};
			nodes_stack_.push_back(&root_);
		}
		else {
			if (nodes_stack_.back()->IsMap()) {
				if (!is_key_)
					throw std::logic_error("The key is empty!");

				std::get<Dict>(nodes_stack_.back()->GetNodeValue()).insert({ key_,json::Dict{} });
				nodes_stack_.push_back(&std::get<Dict>(nodes_stack_.back()->GetNodeValue()).at(key_));
				is_key_ = false;
			}
			else {
				std::get<Array>(nodes_stack_.back()->GetNodeValue()).push_back(json::Dict{});
				nodes_stack_.push_back(&std::get<Array>(nodes_stack_.back()->GetNodeValue()).back());
			}
		}

		return DictItemContext(*this);
	}

	ArrayItemContext Builder::StartArray() {
		if (root_.GetNodeValue().index() != 0 && nodes_stack_.empty())
			throw std::logic_error("Object is ready!");

		if (nodes_stack_.empty()) {
			root_ = json::Array{};
			nodes_stack_.push_back(&root_);
		}
		else {
			if (nodes_stack_.back()->IsMap()) {
				std::get<Dict>(nodes_stack_.back()->GetNodeValue()).insert({ key_,json::Array{} });
				if (!is_key_)
					throw std::logic_error("The key is empty!");

				nodes_stack_.push_back(&std::get<Dict>(nodes_stack_.back()->GetNodeValue()).at(key_));
				is_key_ = false;
			}
			else {
				std::get<Array>(nodes_stack_.back()->GetNodeValue()).push_back(json::Array{});
				nodes_stack_.push_back(&std::get<Array>(nodes_stack_.back()->GetNodeValue()).back());
			}
		}

		return ArrayItemContext(*this);
	}

	BasicContext Builder::EndDict() {
		if (root_.GetNodeValue().index() != 0 && nodes_stack_.empty())
			throw std::logic_error("Object is ready!");

		if (!nodes_stack_.back()->IsMap())
			throw std::logic_error("The container is not a Dict!");

		nodes_stack_.pop_back();
		return *this;
	}

	BasicContext Builder::EndArray() {
		if (root_.GetNodeValue().index() != 0 && nodes_stack_.empty())
			throw std::logic_error("Object is ready!");

		if (!nodes_stack_.back()->IsArray())
			throw std::logic_error("The container is not a Array!");

		nodes_stack_.pop_back();
		return *this;
	}

	json::Node Builder::Build() {
		if (!nodes_stack_.empty() || root_.GetNodeValue().index() == 0)
			throw std::logic_error("Construction is not completed!");
		return root_;
	}

	KeyItemContext BasicContext::Key(std::string str) {
		return builder_.Key(str);
	}

	BasicContext BasicContext::EndDict() {
		return builder_.EndDict();
	}

	BasicContext BasicContext::Value(Node val) {
		return builder_.Value(val);
	}

	DictItemContext BasicContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext BasicContext::StartArray() {
		return builder_.StartArray();
	}

	BasicContext BasicContext::EndArray() {
		return builder_.EndArray();
	}

	json::Node BasicContext::Build() {
		return builder_.Build();
	}

	ValueItemContext KeyItemContext::Value(Node val) {
		return ValueItemContext(BasicContext::Value(val));
	}

	ArrayItemContext ArrayItemContext::Value(Node val) {
		return ArrayItemContext(BasicContext::Value(val));
	}
}