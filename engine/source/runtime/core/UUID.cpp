#include "runtime/core/UUID.h"

#include <random>
#include <chrono>
#include <mutex>

namespace Lumen
{
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_RandomEngine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;
	static std::mutex s_UUIDMutex;
	static uint64_t s_LastTimestamp = 0;
	static uint64_t s_Counter = 0;

	UUID::UUID()
	{
		std::scoped_lock<std::mutex> lock(s_UUIDMutex);

		uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::high_resolution_clock::now().time_since_epoch()).count();

		if (timestamp == s_LastTimestamp)
			s_Counter++;
		else
			s_Counter = 0;
		s_LastTimestamp = timestamp;

		uint64_t randomPart = s_UniformDistribution(s_RandomEngine);

		m_UUID = (timestamp << 32) | (randomPart | (s_Counter & 0xFFFFFFFF));
	}

	UUID::UUID(uint64_t uuid) : m_UUID(uuid) {}
}