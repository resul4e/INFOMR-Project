cmake_minimum_required(VERSION 3.8)

# Include the INSTALL target in the build step
set(CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD 1)

project(INFOMR-Project)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set executable directory
set(BINARY_DIR ${CMAKE_SOURCE_DIR}/Binary)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Set our include folders as the place to look for library includes
include_directories(${CMAKE_SOURCE_DIR}/ThirdParty/assimp/include/)
include_directories(${CMAKE_SOURCE_DIR}/ThirdParty/glm/include/)

# Add source subdirectory which contains the source files
add_subdirectory(Source)

add_executable(${PROJECT_NAME}
    ${BASE_SOURCES}
)

source_group(Source FILES ${BASE_SOURCES})

target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/ThirdParty/assimp/lib/assimp.lib)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR}/$<CONFIG>/)

# Install binaries
install(FILES ThirdParty/assimp/bin/assimp.dll DESTINATION ${BINARY_DIR}/$<CONFIG>/)

#set startup project to be the main executable instead of ALL_BUILD
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}")

#message("${BINARY_DIR}")
add_custom_command(TARGET "${PROJECT_NAME}" POST_BUILD COMMAND ${Qt5_DIR}/../../../bin/windeployqt.exe --quick ${BINARY_DIR}/$<CONFIG>/)