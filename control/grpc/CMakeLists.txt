find_package(Protobuf CONFIG)
if(NOT Protobuf_FOUND)
	find_package(Protobuf REQUIRED)
endif()
find_package(gRPC CONFIG REQUIRED)

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)

set(PROTO_DIR "${CMAKE_SOURCE_DIR}/proto")
set(PROTO_FILE "${PROTO_DIR}/led_control.proto")

set(GENERATED_ROOT "${CMAKE_CURRENT_BINARY_DIR}/include")
set(GENERATED_DIR "${GENERATED_ROOT}/grpc/generated")

set(GENERATED_SRCS
	${GENERATED_DIR}/led_control.pb.cc
	${GENERATED_DIR}/led_control.pb.h
	${GENERATED_DIR}/led_control.grpc.pb.cc
	${GENERATED_DIR}/led_control.grpc.pb.h
)

add_custom_command(
	OUTPUT ${GENERATED_SRCS}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
	COMMAND protobuf::protoc
		--proto_path=${PROTO_DIR}
		--cpp_out=${GENERATED_DIR}
		--grpc_out=${GENERATED_DIR}
		--plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
		${PROTO_FILE}
	DEPENDS ${PROTO_FILE}
	COMMENT "Generating C++ code from led_control.proto"
)

add_library(led_control_proto ${GENERATED_SRCS})

target_include_directories(led_control_proto
	PUBLIC
		${GENERATED_ROOT}
)

target_link_libraries(led_control_proto
	PUBLIC
		protobuf::libprotobuf
		gRPC::grpc++
)
