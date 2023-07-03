qt_add_library(Core SHARED
    core.cpp
	core.h
	plugininterface.h
)

target_compile_definitions(Core PRIVATE PLUGININTERFACE_LIBRARY)

target_link_libraries(Core PRIVATE Qt6::Core)

target_include_directories(Core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})