qt_add_library(SyncServer SHARED
    SyncServer.cpp
	SyncServer.h
)
target_include_directories(SyncServer 
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_SOURCE_DIR}/core
)
target_compile_definitions(SyncServer PRIVATE PLUGININTERFACE_LIBRARY)

target_link_libraries(SyncServer PRIVATE Qt6::Core Core)
set_target_properties (SyncServer PROPERTIES
	FOLDER plugins
)