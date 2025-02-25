#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Author: Inan Evin
# www.inanevin.com
# 
# Copyright (C) 2018 Inan Evin
# 
# Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions 
# and limitations under the License.
#-------------------------------------------------------------------------------------------------------------------------------------------------------------------------
cmake_minimum_required (VERSION 3.10...3.31)
project(LinaEditor)

#--------------------------------------------------------------------
# Set sources
#--------------------------------------------------------------------

set(HEADERS

# Core
include/Editor/CommonEditor.hpp
Include/Editor/CommonSettings.hpp
include/Editor/PayloadListener.hpp
include/Editor/EditorLocale.hpp
include/Editor/Editor.hpp
include/Editor/EditorTaskManager.hpp
include/Editor/Actions/EditorActionManager.hpp
include/Editor/Actions/EditorAction.hpp
include/Editor/Actions/EditorActionResources.hpp
include/Editor/Actions/EditorActionEntity.hpp
include/Editor/Actions/EditorActionWidget.hpp
include/Editor/Actions/EditorActionComponent.hpp
include/Editor/Actions/EditorActionWorld.hpp
include/Editor/Actions/EditorActionSettings.hpp
include/Editor/WindowPanelManager.hpp
include/Editor/World/CommonEditorWorld.hpp
include/Editor/World/EditorWorldManager.hpp
include/Editor/World/EditorWorldUserData.hpp

# Project
include/Editor/Project/ProjectManager.hpp

# Atlas
include/Editor/Atlas/AtlasManager.hpp

# Graphics
include/Editor/Graphics/EditorRenderer.hpp
include/Editor/Graphics/SurfaceRenderer.hpp
include/Editor/Graphics/EditorGfxHelpers.hpp
include/Editor/Graphics/EditorWorldRenderer.hpp
include/Editor/Graphics/GizmoRenderer.hpp
include/Editor/Graphics/OutlineSelectionRenderer.hpp
include/Editor/Graphics/MousePickRenderer.hpp

# IO
include/Editor/IO/ThumbnailGenerator.hpp
include/Editor/IO/ExtensionSupport.hpp

# Widgets
include/Editor/Widgets/CommonWidgets.hpp
include/Editor/Widgets/Testbed.hpp
include/Editor/Widgets/DockTestbed.hpp
include/Editor/Widgets/EditorRoot.hpp
include/Editor/Widgets/Graph/LineGraph.hpp
include/Editor/Widgets/Graph/CircleGauge.hpp
include/Editor/Widgets/Compound/ColorWheelCompound.hpp
include/Editor/Widgets/Compound/Tab.hpp
include/Editor/Widgets/Compound/Table.hpp
include/Editor/Widgets/Compound/TabRow.hpp
include/Editor/Widgets/Compound/IconTabs.hpp
include/Editor/Widgets/Compound/ResourceDirectoryBrowser.hpp
include/Editor/Widgets/Compound/EntityBrowser.hpp
include/Editor/Widgets/Compound/WidgetBrowser.hpp
include/Editor/Widgets/Compound/EntityDetails.hpp
include/Editor/Widgets/Compound/WindowBar.hpp
include/Editor/Widgets/Docking/DockArea.hpp
include/Editor/Widgets/Docking/DockBorder.hpp
include/Editor/Widgets/Docking/DockWidget.hpp
include/Editor/Widgets/Docking/DockPreview.hpp
include/Editor/Widgets/Popups/NotificationDisplayer.hpp
include/Editor/Widgets/Popups/InfoTooltip.hpp
include/Editor/Widgets/Panel/Panel.hpp
include/Editor/Widgets/Panel/PanelWorld.hpp
include/Editor/Widgets/Panel/PanelLog.hpp
include/Editor/Widgets/Panel/PanelWidgetEditor.hpp
include/Editor/Widgets/Panel/PanelWidgetEditorProperties.hpp
include/Editor/Widgets/Panel/PanelPerformance.hpp
include/Editor/Widgets/Panel/PanelEntities.hpp
include/Editor/Widgets/Panel/PanelDetails.hpp
include/Editor/Widgets/Panel/PanelTextureViewer.hpp
include/Editor/Widgets/Panel/PanelFontViewer.hpp
include/Editor/Widgets/Panel/PanelModelViewer.hpp
include/Editor/Widgets/Panel/PanelShaderViewer.hpp
include/Editor/Widgets/Panel/PanelSamplerViewer.hpp
include/Editor/Widgets/Panel/PanelPhysicsMaterialViewer.hpp
include/Editor/Widgets/Panel/PanelMaterialViewer.hpp
include/Editor/Widgets/Panel/PanelAudioViewer.hpp
include/Editor/Widgets/Panel/PanelResourceBrowser.hpp
include/Editor/Widgets/Panel/PanelColorWheel.hpp
include/Editor/Widgets/Panel/PanelGenericSelector.hpp
include/Editor/Widgets/Panel/PanelFactory.hpp
include/Editor/Widgets/Panel/PanelResourceViewer.hpp
include/Editor/Widgets/Panel/PanelProjectSettings.hpp
include/Editor/Widgets/Panel/PanelEditorSettings.hpp
include/Editor/Widgets/FX/LinaLoading.hpp
include/Editor/Widgets/FX/ColorWheel.hpp
include/Editor/Widgets/FX/ProgressCircleFill.hpp
include/Editor/Widgets/Layout/ItemController.hpp
include/Editor/Widgets/World/WorldDisplayer.hpp
include/Editor/Widgets/World/WorldController.hpp

# World
include/Editor/World/EditorCamera.hpp
include/Editor/World/EditorWorldUtility.hpp

# Meta
include/Editor/Meta/EditorSettings.hpp
include/Editor/Meta/EditorLayout.hpp

# Resources
include/Editor/Resources/ResourcePipeline.hpp
include/Editor/Resources/EditorResources.hpp
include/Editor/Resources/ShaderImport.hpp
)

set(SOURCES
# Core
src/CommonEditor.cpp
src/Actions/EditorActionManager.cpp
src/Actions/EditorAction.cpp
src/Actions/EditorActionResources.cpp
src/Actions/EditorActionEntity.cpp
src/Actions/EditorActionWidget.cpp
src/Actions/EditorActionComponent.cpp
src/Actions/EditorActionWorld.cpp
src/Actions/EditorActionSettings.cpp
src/EditorLocale.cpp
src/Editor.cpp
src/EditorTaskManager.cpp
src/WindowPanelManager.cpp

# Project
src/Project/ProjectManager.cpp

# Atlas
src/Atlas/AtlasManager.cpp

# Graphics
src/Graphics/EditorRenderer.cpp
src/Graphics/SurfaceRenderer.cpp
src/Graphics/EditorGfxHelpers.cpp
src/Graphics/EditorWorldRenderer.cpp
src/Graphics/GizmoRenderer.cpp
src/Graphics/OutlineSelectionRenderer.cpp
src/Graphics/MousePickRenderer.cpp


# IO
src/IO/ThumbnailGenerator.cpp
src/IO/ExtensionSupport.cpp

# Widgets
src/Widgets/CommonWidgets.cpp
src/Widgets/Testbed.cpp
src/Widgets/DockTestbed.cpp
src/Widgets/EditorRoot.cpp
src/Widgets/Graph/LineGraph.cpp
src/Widgets/Graph/CircleGauge.cpp
src/Widgets/Compound/ColorWheelCompound.cpp
src/Widgets/Compound/TabRow.cpp
src/Widgets/Compound/Tab.cpp
src/Widgets/Compound/Table.cpp
src/Widgets/Compound/IconTabs.cpp
src/Widgets/Compound/ResourceDirectoryBrowser.cpp
src/Widgets/Compound/EntityBrowser.cpp
src/Widgets/Compound/WidgetBrowser.cpp
src/Widgets/Compound/EntityDetails.cpp
src/Widgets/Compound/WindowBar.cpp
src/Widgets/Docking/DockWidget.cpp
src/Widgets/Docking/DockBorder.cpp
src/Widgets/Docking/DockArea.cpp
src/Widgets/Docking/DockPreview.cpp
src/Widgets/Popups/NotificationDisplayer.cpp
src/Widgets/Popups/InfoTooltip.cpp
src/Widgets/Panel/Panel.cpp
src/Widgets/Panel/PanelPerformance.cpp
src/Widgets/Panel/PanelWorld.cpp
src/Widgets/Panel/PanelLog.cpp
src/Widgets/Panel/PanelWidgetEditor.cpp
src/Widgets/Panel/PanelWidgetEditorProperties.cpp
src/Widgets/Panel/PanelEntities.cpp
src/Widgets/Panel/PanelDetails.cpp
src/Widgets/Panel/PanelTextureViewer.cpp
src/Widgets/Panel/PanelFontViewer.cpp
src/Widgets/Panel/PanelModelViewer.cpp
src/Widgets/Panel/PanelShaderViewer.cpp
src/Widgets/Panel/PanelSamplerViewer.cpp
src/Widgets/Panel/PanelPhysicsMaterialViewer.cpp
src/Widgets/Panel/PanelMaterialViewer.cpp
src/Widgets/Panel/PanelAudioViewer.cpp
src/Widgets/Panel/PanelResourceBrowser.cpp
src/Widgets/Panel/PanelColorWheel.cpp
src/Widgets/Panel/PanelGenericSelector.cpp
src/Widgets/Panel/PanelFactory.cpp
src/Widgets/Panel/PanelResourceViewer.cpp
src/Widgets/Panel/PanelProjectSettings.cpp
src/Widgets/Panel/PanelEditorSettings.cpp
src/Widgets/FX/LinaLoading.cpp
src/Widgets/FX/ColorWheel.cpp
src/Widgets/FX/ProgressCircleFill.cpp
src/Widgets/Layout/ItemController.cpp
src/Widgets/World/WorldDisplayer.cpp
src/Widgets/World/WorldController.cpp

# World
src/World/EditorCamera.cpp
src/World/EditorWorldUtility.cpp
src/World/EditorWorldManager.cpp
src/World/CommonEditorWorld.cpp
src/World/EditorWorldUserData.cpp

# Meta
src/Meta/EditorSettings.cpp
src/Meta/EditorLayout.cpp

# Resources
src/Resources/ResourcePipeline.cpp
src/Resources/EditorResources.cpp
src/Resources/ShaderImport.cpp
)

#--------------------------------------------------------------------
# Create executable project
#--------------------------------------------------------------------

if(WIN32)
set(ICON ${PROJECT_SOURCE_DIR}/_Resources/${PROJECT_NAME}.rc)
endif()

if(APPLE)
set(ICON ${PROJECT_SOURCE_DIR}/_Resources/${PROJECT_NAME}.icns)
endif()

# WIN32 Console Subsystem
if(WIN32)
	add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${ICON})
else()
	add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${ICON})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER LinaEngine)


#--------------------------------------------------------------------
# Config & Options & Compile Definitions
#--------------------------------------------------------------------
include(${CMAKE_SOURCE_DIR}/CMake/ProjectSettings.cmake)
include(${CMAKE_SOURCE_DIR}/CMake/ExecutableSettings.cmake)
target_compile_definitions(LinaCommon PUBLIC LINA_ENABLE_PROFILER)
target_compile_definitions(LinaCore PUBLIC LINA_ENABLE_PROFILER)
target_compile_definitions(${PROJECT_NAME} PUBLIC LINA_ENABLE_PROFILER)

#--------------------------------------------------------------------
# Set include directories
#--------------------------------------------------------------------
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/LinaCore/include)

#--------------------------------------------------------------------
# Links
#--------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} 
PUBLIC Lina::Core
)

#--------------------------------------------------------------------
# Folder structuring in visual studio
#--------------------------------------------------------------------

include(${CMAKE_SOURCE_DIR}/CMake/CommonFunctions.cmake)
group_sources(HEADERS SOURCES)

# Copy Lina libraries to output for game dll
# include(${CMAKE_SOURCE_DIR}/Dependencies/DependenciesEditorPackaging.cmake)


