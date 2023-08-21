
# gRPC
# find_package(Threads REQUIRED)
# set(ABSL_ENABLE_INSTALL ON) 
# if(APPLE)
#     set(CMAKE_MACOSX_RPATH 1)
# endif()
# add_subdirectory(${CMAKE_SOURCE_DIR}/external/grpc ${CMAKE_BINARY_DIR}/grpc EXCLUDE_FROM_ALL)
# set(_PROTOBUF_LIBPROTOBUF libprotobuf)
# set(_REFLECTION grpc++_reflection)
# set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
# set(_GRPC_GRPCPP grpc++)
# set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:grpc_cpp_plugin>)
# target_link_libraries(${TARGET_NAME} PRIVATE grpc++)

# proto files
# get_filename_component(hw_proto "${CMAKE_SOURCE_DIR}/src/app/.proto/dummy.proto" ABSOLUTE)
# get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generated sources
# set(hw_proto_srcs "${CMAKE_SOURCE_DIR}/src/app/.proto/dummy.pb.cc")
# set(hw_proto_hdrs "${CMAKE_SOURCE_DIR}/src/app/.proto/dummy.pb.h")
# set(hw_grpc_srcs "${CMAKE_SOURCE_DIR}/src/app/.proto/dummy.grpc.pb.cc")
# set(hw_grpc_hdrs "${CMAKE_SOURCE_DIR}/src/app/.proto/dummy.grpc.pb.h")

# execute_process(
#     COMMAND ${CMAKE_BINARY_DIR}/grpc/third_party/protobuf/Debug/protoc 
#     --grpc_out=${CMAKE_SOURCE_DIR}/src/app/.proto
#     --cpp_out=${CMAKE_SOURCE_DIR}/src/app/.proto
#     --proto_path=${hw_proto_path}
#     --plugin=protoc-gen-grpc=${CMAKE_BINARY_DIR}/grpc/Debug/grpc_cpp_plugin
#     "dummy.proto"
#     RESULT_VARIABLE outt)

# hw_grpc_proto
# set(PROTO_SOURCES
#     ${hw_proto_srcs}
#     ${hw_proto_hdrs}
#     ${hw_grpc_srcs}
#     ${hw_grpc_hdrs}
# )
# add_library(grpc_proto STATIC ${PROTO_SOURCES})
# target_link_libraries(grpc_proto
#     ${_REFLECTION}
#     ${_GRPC_GRPCPP}
#     ${_PROTOBUF_LIBPROTOBUF})