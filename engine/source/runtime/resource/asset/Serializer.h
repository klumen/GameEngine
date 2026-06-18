#pragma once

#include "runtime/core/UUID.h"

#define YAML_CPP_STATIC_DEFINE
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::mat4>
	{
		static Node encode(const glm::mat4& mat)
		{
			Node node;
			const float* data = glm::value_ptr(mat);
			for (int i = 0; i < 16; ++i)
				node.push_back(data[i]);
			node.SetStyle(YAML::EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::mat4& mat)
		{
			if (!node.IsSequence() || node.size() != 16)
				return false;

			float* data = glm::value_ptr(mat);
			for (int i = 0; i < 16; ++i)
				data[i] = node[i].as<float>();

			return true;
		}
	};

	template<>
	struct convert<glm::mat3>
	{
		static Node encode(const glm::mat3& mat)
		{
			Node node;
			const float* data = glm::value_ptr(mat);
			for (int i = 0; i < 9; ++i)
				node.push_back(data[i]);
			node.SetStyle(YAML::EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::mat3& mat)
		{
			if (!node.IsSequence() || node.size() != 9)
				return false;

			float* data = glm::value_ptr(mat);
			for (int i = 0; i < 9; ++i)
				data[i] = node[i].as<float>();

			return true;
		}
	};

	template<>
	struct convert<glm::mat2>
	{
		static Node encode(const glm::mat2& mat)
		{
			Node node;
			const float* data = glm::value_ptr(mat);
			for (int i = 0; i < 4; ++i)
				node.push_back(data[i]);
			node.SetStyle(YAML::EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::mat2& mat)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			float* data = glm::value_ptr(mat);
			for (int i = 0; i < 4; ++i)
				data[i] = node[i].as<float>();

			return true;
		}
	};

	template<>
	struct convert<Lumen::UUID>
	{
		static Node encode(const Lumen::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, Lumen::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};
}

namespace Lumen
{
	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat2& mat) {
		out << YAML::Flow << YAML::BeginSeq;

		const float* data = glm::value_ptr(mat);
		for (int i = 0; i < 4; ++i)
			out << data[i];
		
		out << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat3& mat) {
		out << YAML::Flow << YAML::BeginSeq;

		const float* data = glm::value_ptr(mat);
		for (int i = 0; i < 9; ++i)
			out << data[i];

		out << YAML::EndSeq;
		return out;
	}

	inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::mat4& mat) {
		out << YAML::Flow << YAML::BeginSeq;

		const float* data = glm::value_ptr(mat);
		for (int i = 0; i < 16; ++i)
			out << data[i];

		out << YAML::EndSeq;
		return out;
	}
}