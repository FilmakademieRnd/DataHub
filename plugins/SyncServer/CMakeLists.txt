set (target_name SyncServer)

qt_add_library(${target_name} SHARED
    	SyncServer.cpp
	SyncServer.h
	src/messageReceiver.cpp
	src/messageSender.cpp
	src/commandHandler.cpp
	src/sceneReceiver.cpp
	src/sceneSender.cpp
	include/messageSender.h
	include/messageReceiver.h
	include/zeroMQHandler.h
	include/commandHandler.h
	include/sceneReceiver.h
	include/sceneSender.h
	include/sceneDataHandler.h
)
target_include_directories(${target_name} 
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
	PRIVATE ${CMAKE_SOURCE_DIR}/core
	PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/cppzmq/include
	PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/zeromq/include
)
target_link_directories(${target_name} 
	PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/zeromq/lib
)
target_compile_definitions(${target_name} PRIVATE PLUGININTERFACE_LIBRARY)

target_link_libraries(${target_name} PRIVATE 
	Core
	Qt6::Core
	Qt6::Network
	libzmq-v143-mt-4_3_5
)

set_target_properties (${target_name} PROPERTIES
	FOLDER plugins
)

set (ENV{i_plugin_targets} "$ENV{i_plugin_targets};${target_name}")