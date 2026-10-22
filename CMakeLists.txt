cmake_minimum_required(VERSION 3.15)

project(Periscope VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(PERISCOPE_BUILD_SANDBOX "Build the Periscope sandbox executable" ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin-int)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin-int)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Header-only library: consumers link Periscope::periscope and get include/
add_library(periscope INTERFACE)
add_library(Periscope::periscope ALIAS periscope)
target_include_directories(periscope INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(periscope INTERFACE cxx_std_20)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS periscope EXPORT PeriscopeTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT PeriscopeTargets
    FILE PeriscopeTargets.cmake
    NAMESPACE Periscope::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Periscope
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/PeriscopeConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/PeriscopeConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Periscope
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/PeriscopeConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/PeriscopeConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/PeriscopeConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Periscope
)

if(PERISCOPE_BUILD_SANDBOX)
    add_subdirectory(sandbox)
endif()
