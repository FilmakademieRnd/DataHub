set (target_name MessageRecorder)

qt_add_library(${target_name} SHARED
    MessageRecorder.cpp
	MessageRecorder.h

)
target_include_directories(${target_name} 
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_SOURCE_DIR}/core
)

target_compile_definitions(${target_name} PRIVATE PLUGININTERFACE_LIBRARY)

target_link_libraries(${target_name} PRIVATE 
	Core
	Qt6::Core
)

set_target_properties (${target_name} PROPERTIES
	FOLDER plugins
)

set (ENV{i_plugin_targets} "$ENV{i_plugin_targets};${target_name}")