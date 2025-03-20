cmake_minimum_required(VERSION 3.10)
project(MemoryManager)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontrar Protobuf y gRPC
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Generar código desde .proto
get_filename_component(hw_proto "memory_manager.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generar archivos .pb.{h,cc}
add_custom_command(
      OUTPUT "memory_manager.pb.h" "memory_manager.pb.cc"
      COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS --proto_path=${hw_proto_path}
           --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
           ${hw_proto}
      DEPENDS ${hw_proto})

# Generar archivos .grpc.pb.{h,cc}
add_custom_command(
      OUTPUT "memory_manager.grpc.pb.h" "memory_manager.grpc.pb.cc"
      COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS --proto_path=${hw_proto_path}
           --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
           --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
           ${hw_proto}
      DEPENDS ${hw_proto})

# Definir biblioteca proto
add_library(memory_proto
  ${CMAKE_CURRENT_BINARY_DIR}/memory_manager.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/memory_manager.pb.h
  ${CMAKE_CURRENT_BINARY_DIR}/memory_manager.grpc.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/memory_manager.grpc.pb.h
)

target_link_libraries(memory_proto
  ${PROTOBUF_LIBRARIES}
  gRPC::grpc++
  gRPC::grpc++_reflection
)

target_include_directories(memory_proto PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

# Servidor
add_executable(mem-mgr
  Server/memory_manager_server.cpp
)

target_link_libraries(mem-mgr
  memory_proto
  ${PROTOBUF_LIBRARIES}
  gRPC::grpc++
  gRPC::grpc++_reflection
)

# Cliente de prueba(modificación: usando main.cpp como punto de entrada, ya no el memory_manager_client.cpp)
add_executable(mem-client
  Client/main.cpp
  Client/memory_manager_client.cpp
)

target_link_libraries(mem-client
  memory_proto
  ${PROTOBUF_LIBRARIES}
  gRPC::grpc++
)