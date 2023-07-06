cmake_minimum_required(VERSION 3.16)

project(DataHub VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE PATH "set config types" FORCE)
set(CMAKE_DEBUG_POSTFIX _d)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "set install prefix" FORCE)

find_package(Qt6 REQUIRED COMPONENTS Core)
qt_standard_project_setup()

set_property (GLOBAL PROPERTY USE_FOLDERS ON) 

set (i_targets DataHub)

add_subdirectory(core)
add_subdirectory(plugins)
add_subdirectory(application)

install(TARGETS ${i_targets}
		RUNTIME 
		DESTINATION ${INSTALL_DIR})

