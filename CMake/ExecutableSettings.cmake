
if(MSVC)
  set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/)
target_sources(${PROJECT_NAME} PRIVATE _Resources/Resources.rc)

endif()

if(APPLE)
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    XCODE_GENERATE_SCHEME TRUE
    XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
    XCODE_SCHEME_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/
)

set_target_properties(${PROJECT_NAME}  PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
)

set_source_files_properties(${PROJECT_NAME}.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

endif()



# Core resources to output directory
add_custom_command(
TARGET ${PROJECT_NAME}
POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/LinaCore/_Resources" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/Resources/")


# Copy Project resources to binary directory
add_custom_command(
TARGET ${PROJECT_NAME}
POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/_Resources" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/Resources/")

if(APPLE)

endif()