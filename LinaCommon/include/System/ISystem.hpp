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

#ifndef ISystem_HPP
#define ISystem_HPP

#include "Event/ISystemEventDispatcher.hpp"
#include "System/ISubsystem.hpp"
#include "Data/HashMap.hpp"
#include "Core/Common.hpp"
#include "JobSystem/JobSystem.hpp"

namespace Lina
{
	class ISubsystem;
	class Application;

	class ISystem : public ISystemEventDispatcher
	{
	public:
		ISystem(Application* app) : m_app(app){};
		virtual ~ISystem() = default;

		void		 AddSubsystem(ISubsystem* sub);
		void		 RemoveSubsystem(ISubsystem* sub);
		ISubsystem*	 CastSubsystem(SubsystemType type);
		virtual void Initialize(const SystemInitializationInfo& initInfo)	  = 0;
		virtual void PostInitialize(const SystemInitializationInfo& initInfo) = 0;
		virtual void Shutdown()												  = 0;
		virtual void Tick(float dt)											  = 0;

		template <typename T> T* CastSubsystem(SubsystemType type)
		{
			return (T*)(m_subsystems[type]);
		}

		inline const SystemInitializationInfo& GetInitInfo()
		{
			return m_initInfo;
		}

		inline Executor* GetMainExecutor()
		{
			return &m_mainExecutor;
		}

		inline Application* GetApp()
		{
			return m_app;
		}

	protected:
		Application*						m_app = nullptr;
		Executor							m_mainExecutor;
		SystemInitializationInfo			m_initInfo;
		HashMap<SubsystemType, ISubsystem*> m_subsystems;
	};
} // namespace Lina

#endif
