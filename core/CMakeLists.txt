set (target_name Core)

qt_add_library(${target_name} SHARED
	core.cpp
	core.h
)

target_compile_definitions(${target_name} PRIVATE CORE_LIBRARY)

target_link_libraries(${target_name} PRIVATE Qt6::Core)

target_include_directories(${target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

set (i_targets "${i_targets};${target_name}" PARENT_SCOPE)