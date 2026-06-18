#include "runtime/core/Log.h"

//#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

namespace Lumen
{
	void Log::StartUp()
	{
		auto consoleSink = MakeShared<spdlog::sinks::stdout_color_sink_mt>();
		auto fileSink = MakeShared<spdlog::sinks::basic_file_sink_mt>("Lumen.log", true);
		consoleSink->set_pattern("%^[%T] %n: %v%$");
		fileSink->set_pattern("[%T] [%l] %n: %v");
		const spdlog::sinks_init_list sinksList = { consoleSink, fileSink };

		// spdlog::init_thread_pool()
		// thread safe
		s_RuntimeLogger = MakeShared<spdlog::logger>("LumenRuntime", sinksList.begin(), sinksList.end());
		s_RuntimeLogger->set_level(spdlog::level::trace);
		s_RuntimeLogger->flush_on(spdlog::level::trace);
		spdlog::register_logger(s_RuntimeLogger);

		s_EditorLogger = MakeShared<spdlog::logger>("LumenEditor", sinksList.begin(), sinksList.end());
		s_EditorLogger->set_level(spdlog::level::trace);
		s_EditorLogger->flush_on(spdlog::level::trace);
		spdlog::register_logger(s_EditorLogger);
	}

	void Log::ShutDown()
	{
		s_RuntimeLogger->flush();
		s_EditorLogger->flush();

		spdlog::drop_all();

		s_RuntimeLogger.reset();
		s_EditorLogger.reset();
	}
}