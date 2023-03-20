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

#ifndef Event_HPP
#define Event_HPP

#include "Core/SizeDefinitions.hpp"

namespace Lina
{
	struct Event
	{
		void*  pParams[2];
		float  fParams[4];
		uint32 iParams[4];
	};

	enum SystemEvent
	{
		EVS_ResourceLoaded		  = 1 << 0,
		EVS_ResourceLoadTaskCompleted	  = 1 << 1,
		EVS_ResourceUnloaded	  = 1 << 2,
		EVS_ResourceBatchUnloaded = 1 << 3,
		EVS_LevelInstalled		  = 1 << 4,
		EVS_LevelUninstalled	  = 1 << 5,
		EVS_WindowResized		  = 1 << 6,
		EVS_VsyncModeChanged	  = 1 << 7,
	};

	enum GameEvent
	{
		EVG_Start			   = 1 << 0,
		EVG_PostStart		   = 1 << 1,
		EVG_Tick			   = 1 << 2,
		EVG_PostTick		   = 1 << 3,
		EVG_Physics			   = 1 << 4,
		EVG_PostPhysics		   = 1 << 5,
		EVG_ComponentCreated   = 1 << 6,
		EVG_ComponentDestroyed = 1 << 7,
	};
} // namespace Lina

#endif
