/*
Author: Inan Evin
www.inanevin.com

Copyright 2018 Inan Evin

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions
and limitations under the License.

Class: Layer
Timestamp: 1/2/2019 1:42:06 AM

*/

#pragma once
#ifndef Layer_HPP
#define Layer_HPP


#include "Core/Common.hpp"
#include <string>

namespace LinaEngine
{
	class Layer
	{
	public:

		Layer(const std::string& name = "Layer") : m_name(name) {};
		virtual ~Layer() {};

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnTick(float dt) {}

		FORCEINLINE const std::string& GetName() const { return m_name; }

	protected:

		std::string m_name = "";

	};
}


#endif