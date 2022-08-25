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

#ifndef ResourcePackager_HPP
#define ResourcePackager_HPP

// Headers here.
#include "ResourceCommon.hpp"
#include "Data/String.hpp"
#include "Utility/StringId.hpp"
#include "Data/HashSet.hpp"
#include "Data/HashMap.hpp"
#include "Core/CommonMemory.hpp"

namespace Lina
{
    namespace Resources
    {
        class ResourceLoader;
    }
} // namespace Lina
namespace Lina::Resources
{
    class ResourcePackager
    {
    private:
        friend class Engine;
        friend class EditorResourceLoader;
        friend class StandaloneResourceLoader;

        void LoadPackage(const String& packageName, ResourceLoader* loader, Memory::ResourceAllocator allocator = Memory::ResourceAllocator::None);
        void LoadFilesFromPackage(const String& packageName, const HashSet<StringID>& filesToLoad, ResourceLoader* loader, Memory::ResourceAllocator allocator = Memory::ResourceAllocator::None);
        void UnpackAndLoad(const String& filePath, ResourceLoader* loader, Memory::ResourceAllocator allocator = Memory::ResourceAllocator::None);
        void UnpackAndLoad(const String& filePath, const HashSet<StringID>& filesToLoad, ResourceLoader* loader, Memory::ResourceAllocator allocator = Memory::ResourceAllocator::None);
        void PackageProject(const String& path, const Vector<String>& levelsToPackage, const HashMap<TypeID, Vector<String>>& resourceMap);
        void PackageFileset(Vector<String> files, const String& output);
    };

}; // namespace Lina::Resources

#endif
