set (target_name DataHub)

qt_add_executable(${target_name}
    DataHub.cpp
)

set_target_properties(${target_name} PROPERTIES
                      INSTALL_RPATH "@loader_path")

target_link_libraries(${target_name} PRIVATE 
	Qt6::Core
	Core
)
