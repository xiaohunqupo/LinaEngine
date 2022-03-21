/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-2020] [Inan Evin]

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

/*
Class: FileWatcher



Timestamp: 3/21/2022 8:45:01 PM
*/

#pragma once

#ifndef FileWatcher_HPP
#define FileWatcher_HPP

// Headers here.
#include "EventSystem/MainLoopEvents.hpp"
#include <filesystem>

namespace Lina
{
    enum class FileWatchStatus
    {
        None,
        Created,
        Modified,
        Erased
    };

    class FileWatcher
    {

    public:
        FileWatcher(){};
        ~FileWatcher(){};

        void Initialize(const std::string& directory, float interval, FileWatchStatus targetStatus);

    public:
        std::function<void(FileWatchStatus status, const std::string& fullPath)> m_changeCallback;

    private:
        void OnTick(const Event::ETick&);
        bool Contains(const std::string& key);
        void ReplaceAndCall(FileWatchStatus status, const std::string& path);

    private:
        std::string                                                      m_directory        = "";
        float                                                            m_totalTime        = 0.0f;
        float                                                            m_interval         = 0.0f;
        float                                                            m_lastCheckTime    = 0.0f;
        FileWatchStatus                                                  m_targetStatus     = FileWatchStatus::None;
        std::unordered_map<std::string, std::filesystem::file_time_type> m_paths;
    };
} // namespace Lina

#endif
