set(PROJECT_NAME led_control)

add_subdirectory(3rd_party)
add_subdirectory(grpc)
add_subdirectory(led)
add_subdirectory(telemetry)

add_executable(${PROJECT_NAME}
	main.cpp
	led_control.cpp
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		led_control_proto
		led
		telemetry
)
