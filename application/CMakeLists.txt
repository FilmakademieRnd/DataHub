set (target_name DataHub)

qt_add_executable(${target_name}
    DataHub.cpp
)

target_link_libraries(${target_name} PRIVATE 
	Qt6::Core
	Core
)