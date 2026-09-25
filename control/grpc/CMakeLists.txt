find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)


set(GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated")
add_library(led_control_proto
	${GENERATED_DIR}/led_control.pb.cc
	${GENERATED_DIR}/led_control.grpc.pb.cc
)

target_include_directories(led_control_proto
	PUBLIC
		${GENERATED_DIR}
)

target_link_libraries(led_control_proto
	PUBLIC
		protobuf::libprotobuf
		gRPC::grpc++
)
