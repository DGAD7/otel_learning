set(LIB telemetry)

add_library(${LIB}
	custom_exporter.cpp
	logging.cpp
)

target_include_directories(${LIB}
PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(${LIB}
PUBLIC
	opentelemetry-cpp::api

PRIVATE
	opentelemetry_logs
	opentelemetry_exporter_ostream_logs
	opentelemetry_exporter_otlp_grpc_log
)
