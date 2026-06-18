#include "runtime/function/HID/HIDSystem.h"

#include "runtime/core/Utility.h"

namespace Lumen
{
    static Utility::MovingAverage<float, 10> averageFilter;

	void HIDSystem::StartUp()
	{
	}

	void HIDSystem::ShutDown()
	{
	}

	float HIDSystem::MovingAverageFilter(float input)
	{
        averageFilter.AddSample(input);
		return averageFilter.GetCurrentAverage();
	}

	float HIDSystem::LowPassFilter(float input)
	{
		float RC = 0.f;
		float alpha = 0.f;
		float lastFilteredInput = 0.f;

		lastFilteredInput = alpha * input + (1.f - alpha) * lastFilteredInput;

		return lastFilteredInput;
	}
}

// TODO:
/*#pragma once
#include <vector>
#include <functional>
#include <cmath>
#include <atomic>

namespace HID {
    constexpr float PI = 3.14159265358979323846f;

    // 移动平均滤波
    class MovingAverageFilter {
        std::vector<float> buffer;
        size_t index = 0;
        float sum = 0;
        
    public:
        explicit MovingAverageFilter(size_t window_size) {
            buffer.resize(window_size, 0.0f);
        }

        float apply(float input) {
            sum -= buffer[index];
            sum += input;
            buffer[index] = input;
            index = (index + 1) % buffer.size();
            return sum / buffer.size();
        }
    };

    // 死区滤波
    struct DeadZoneFilter {
        float dead_zone;
        float max_value;

        float operator()(float raw) const {
            const float norm = raw / max_value;
            if (std::abs(norm) < dead_zone) return 0.0f;
            return std::copysign((std::abs(norm) - dead_zone) / (1 - dead_zone), norm);
        }
    };

    // 自适应低通滤波
    class AdaptiveLowPassFilter {
        float alpha;
        float prev_out = 0;
        float cutoff;
        
    public:
        AdaptiveLowPassFilter(float initial_cutoff) : cutoff(initial_cutoff) {}

        float apply(float input, float delta_time) {
            alpha = 1 - std::exp(-delta_time * cutoff);
            prev_out = alpha * input + (1 - alpha) * prev_out;
            return prev_out;
        }

        void set_cutoff(float new_cutoff) { cutoff = new_cutoff; }
    };

    // 响应曲线
    namespace ResponseCurve {
        inline float linear(float x) { return x; }
        inline float quadratic(float x) { return x * std::abs(x); }
        inline float exponential(float x, float k=1.5f) { return std::pow(x, k); }
    }
}*/

/*class AnalogAxis {
    std::vector<std::function<float(float)>> filter_chain;
    float raw_value = 0;
    bool active = false;
    
public:
    void update(float new_value) {
        raw_value = new_value;
        active = (std::abs(new_value) > 0.001f);
    }

    float get() const {
        if (!active) return 0.0f;
        float result = raw_value;
        for (const auto& filter : filter_chain) {
            result = filter(result);
        }
        return result;
    }

    template<typename Filter, typename... Args>
    void add_filter(Args&&... args) {
        filter_chain.emplace_back(Filter{std::forward<Args>(args)...});
    }
};*/

/*#include "HIDConfig.h"
#include <unordered_map>
#include <mutex>

class DeviceManager {
    std::unordered_map<DeviceID, AnalogAxis> axes;
    std::mutex device_mutex;
    HIDConfig global_config;

public:
    void initialize() {
        load_config("default_hid.json");
        // 平台特定初始化
        platform_init();
    }

    void update(float delta_time) {
        std::lock_guard lock(device_mutex);
        for (auto& [id, axis] : axes) {
            float raw = platform_read_axis(id);
            axis.update(raw);
        }
    }

    void load_config(const std::string& path) {
        // 加载并应用JSON配置
        global_config = ConfigLoader::load(path);
        apply_config();
    }

private:
    void apply_config() {
        for (const auto& [device_id, cfg] : global_config.devices) {
            auto& axis = axes[device_id];
            axis.add_filter(DeadZoneFilter{cfg.dead_zone, cfg.max});
            axis.add_filter(MovingAverageFilter{cfg.avg_window});
            // ... 其他滤波
        }
    }
};*/

/*class ThreadSafeAxis {
    std::atomic<float> atomic_value;
    AnalogAxis axis;

public:
    void update(float value) {
        atomic_value.store(value, std::memory_order_relaxed);
    }

    float get() {
        axis.update(atomic_value.load(std::memory_order_relaxed));
        return axis.get();
    }
};*/

/*#ifdef USE_AVX
#include <immintrin.h>

class SIMDMovingAverage {
    __m256 buffer[8];
    size_t index = 0;
    
public:
    __m256 apply(__m256 input) {
        buffer[index] = input;
        index = (index + 1) % 8;
        
        __m256 sum = _mm256_setzero_ps();
        for (auto& vec : buffer) {
            sum = _mm256_add_ps(sum, vec);
        }
        return _mm256_div_ps(sum, _mm256_set1_ps(8.0f));
    }
};
#endif*/

/*#include <variant>

struct AxisConfig {
    float dead_zone = 0.1f;
    float max_value = 32767.0f;
    uint32_t avg_window = 5;
    float lowpass_cutoff = 10.0f;
    std::string response_curve = "quadratic";
};

struct DeviceConfig {
    std::string driver_type;
    std::map<std::string, AxisConfig> axes;
};

struct HIDConfig {
    std::unordered_map<std::string, DeviceConfig> devices;
    float global_sample_rate = 250.0f;
};*/

/*// 初始化系统
DeviceManager hid;
hid.initialize();

// 游戏循环
while (running) {
    float delta = get_frame_time();
    
    // 更新输入
    hid.update(delta);
    
    // 获取处理后的输入
    float steering = hid.get_axis("wheel", "steering");
    float throttle = hid.get_axis("pedals", "throttle");
    
    // 应用响应曲线
    throttle = ResponseCurve::exponential(throttle, 1.8f);
}*/