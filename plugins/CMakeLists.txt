set (target_name SyncServer)

qt_add_library(${target_name} SHARED
    SyncServer.cpp
	SyncServer.h
)
target_include_directories(${target_name} 
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_SOURCE_DIR}/core
)
target_compile_definitions(${target_name} PRIVATE PLUGININTERFACE_LIBRARY)

target_link_libraries(${target_name} PRIVATE Qt6::Core Core)
set_target_properties (${target_name} PROPERTIES
	FOLDER plugins
)

set (i_targets "${i_targets};${target_name}" PARENT_SCOPE)