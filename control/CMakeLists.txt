set(PROJECT_NAME led_control)

add_subdirectory(grpc)

add_executable(${PROJECT_NAME}
	main.cpp
	led_control.cpp
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		led_control_proto
)
