#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>

namespace Lumen
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		std::string ToString() const 
		{
			std::ostringstream oss;
			oss << std::hex << std::setfill('0')
				<< std::setw(16) << m_UUID;
			return oss.str();
		}

		operator uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID;

	};
}

namespace std
{
	template <typename T> struct hash;

	template<>
	struct hash<Lumen::UUID>
	{
		std::size_t operator()(const Lumen::UUID& uuid) const
		{
			return std::hash<uint64_t>()(uuid);
		}
	};
}