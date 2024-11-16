/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#ifndef Log_HPP
#define Log_HPP

#define FMT_HEADER_ONLY
#include "fmt/core.h"

#ifdef LINA_DEBUG

#define LINA_ERR(...)	   Lina::Log::LogMessage(Lina::LogLevel::LOG_LEVEL_ERROR, __VA_ARGS__)
#define LINA_WARN(...)	   Lina::Log::LogMessage(Lina::LogLevel::LOG_LEVEL_WARNING, __VA_ARGS__)
#define LINA_INFO(...)	   Lina::Log::LogMessage(Lina::LogLevel::LOG_LEVEL_INFO, __VA_ARGS__)
#define LINA_TRACE(...)	   Lina::Log::LogMessage(Lina::LogLevel::LOG_LEVEL_TRACE, __VA_ARGS__)
#define LINA_CRITICAL(...) Lina::Log::LogMessage(Lina::LogLevel::LOG_LEVEL_CRITICAL, __VA_ARGS__)

#else

#define LINA_ERR(...)	   Lina::Log::LogMessage(Lina::LogLevel::Error, __VA_ARGS__)
#define LINA_WARN(...)	   Lina::Log::LogMessage(Lina::LogLevel::Warn, __VA_ARGS__)
#define LINA_INFO(...)	   Lina::Log::LogMessage(Lina::LogLevel::Info, __VA_ARGS__)
#define LINA_TRACE(...)	   Lina::Log::LogMessage(Lina::LogLevel::Trace, __VA_ARGS__)
#define LINA_CRITICAL(...) Lina::Log::LogMessage(Lina::LogLevel::Critical, __VA_ARGS__)

#endif

#ifdef LINA_PLATFORM_WINDOWS
#define DBG_BRK __debugbreak();
#else
#define DBG_BRK __builtin_trap();
#endif

#ifdef LINA_DEBUG
#define LINA_ASSERT(x, ...)                                                                                                                                                                                                                                        \
	if (!(x))                                                                                                                                                                                                                                                      \
	{                                                                                                                                                                                                                                                              \
		LINA_CRITICAL(__VA_ARGS__);                                                                                                                                                                                                                                \
		DBG_BRK                                                                                                                                                                                                                                                    \
	}

#define LINA_ASSERT_F(x)                                                                                                                                                                                                                                           \
	if (!(x))                                                                                                                                                                                                                                                      \
	{                                                                                                                                                                                                                                                              \
		DBG_BRK                                                                                                                                                                                                                                                    \
	}

#else
#define LINA_ASSERT(x, ...)
#endif

#define LINA_NOTIMPLEMENTED LINA_ASSERT(false, "Implementation missing!")

#include "Common/Data/Mutex.hpp"
#include "Common/Data/Vector.hpp"

namespace Lina
{
	enum LogLevel
	{
		LOG_LEVEL_INFO	   = 1 << 0,
		LOG_LEVEL_CRITICAL = 1 << 1,
		LOG_LEVEL_ERROR	   = 1 << 2,
		LOG_LEVEL_TRACE	   = 1 << 3,
		LOG_LEVEL_WARNING  = 1 << 4,
	};

	class LogListener
	{
	public:
		virtual void OnLog(LogLevel level, const char* msg) = 0;
	};
	class Log
	{
	public:
		static void		   AddLogListener(LogListener* listener);
		static void		   RemoveLogListener(LogListener* listener);
		static void		   LogImpl(LogLevel level, const char* msg);
		static const char* GetLogLevel(LogLevel level);

		template <typename... Args> static void LogMessage(LogLevel level, const Args&... args)
		{
			LogImpl(level, fmt::format(args...).c_str());
		}

		inline static Mutex& GetLogMutex()
		{
			return s_logMtx;
		}

	private:
		friend class Engine;
		static Mutex				s_logMtx;
		static Vector<LogListener*> s_logListeners;
	};
} // namespace Lina

#endif
