qt_add_executable(DataHub
    DataHub.cpp
)

target_link_libraries(DataHub PRIVATE 
	Qt6::Core
	Core
	SyncServer
)