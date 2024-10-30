cmake_minimum_required(VERSION 3.16...3.22)

project(DataHub VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE PATH "set config types" FORCE)
set(CMAKE_DEBUG_POSTFIX _d)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "set install prefix" FORCE)

find_package(Qt6 REQUIRED COMPONENTS Core Network)
qt_standard_project_setup()

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_OSX_ARCHITECTURES "x86_64")


set_property (GLOBAL PROPERTY USE_FOLDERS ON) 

set (i_targets DataHub)

add_subdirectory(core)
add_subdirectory(plugins)
add_subdirectory(application)

install(TARGETS ${i_targets}
                DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
)

install(TARGETS $ENV{i_plugin_targets} 
                DESTINATION "${CMAKE_INSTALL_PREFIX}/bin/plugins"
)

#qt_generate_deploy_app_script(
#    TARGET SyncServer
#    OUTPUT_SCRIPT deploy_script
#    NO_UNSUPPORTED_PLATFORM_ERROR
#	NO_TRANSLATIONS
#)
#install(SCRIPT ${deploy_script}
#        DESTINATION "${CMAKE_INSTALL_PREFIX"}
#)



