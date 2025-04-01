cmake_minimum_required(VERSION 3.10)
project(MemoryManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Rutas para evitar conflictos
set(CMAKE_PREFIX_PATH
  "/usr/local/opt/grpc"
  "/usr/local/opt/protobuf"
  "/usr/local/opt/abseil"
)

# ⚠️ IMPORTANTE: primero gRPC (para que él haga el find_package(Protobuf) con su versión)
find_package(gRPC REQUIRED)
find_package(absl REQUIRED)
find_package(Protobuf REQUIRED)  # Este va al final para evitar conflictos

# Ruta al .proto
get_filename_component(hw_proto "${CMAKE_SOURCE_DIR}/memory_manager.proto" ABSOLUTE)
get_filename_component(hw_proto_path "${hw_proto}" PATH)

# Generar código en build/
add_custom_command(
  OUTPUT
    "${CMAKE_CURRENT_BINARY_DIR}/memory_manager.pb.h"
    "${CMAKE_CURRENT_BINARY_DIR}/memory_manager.pb.cc"
    "${CMAKE_CURRENT_BINARY_DIR}/memory_manager.grpc.pb.h"
    "${CMAKE_CURRENT_BINARY_DIR}/memory_manager.grpc.pb.cc"
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
  ARGS
    --proto_path=${hw_proto_path}
    --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
    --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
    --plugin=protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
    ${hw_proto}
  DEPENDS ${hw_proto}
)

# Biblioteca proto
add_library(memory_proto
  ${CMAKE_CURRENT_BINARY_DIR}/memory_manager.pb.cc
  ${CMAKE_CURRENT_BINARY_DIR}/memory_manager.grpc.pb.cc
)

target_include_directories(memory_proto PUBLIC
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(memory_proto
  gRPC::grpc++
  gRPC::grpc++_reflection
  ${Protobuf_LIBRARIES}

  absl::strings
  absl::log
)

# Servidor
add_executable(mem-mgr
  Server/memory_manager_server.cpp
)

target_link_libraries(mem-mgr
  memory_proto
  gRPC::grpc++
  gRPC::grpc++_reflection
  ${Protobuf_LIBRARIES}

  absl::strings
  absl::log
)

# Cliente
add_executable(mem-client
  Client/main.cpp
  Client/memory_manager_client.cpp
)

target_link_libraries(mem-client
  memory_proto
  gRPC::grpc++
  ${Protobuf_LIBRARIES}

  absl::strings
  absl::log
)

# Linux solamente
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(mem-mgr stdc++fs)
  target_link_libraries(mem-client stdc++fs)
endif()
