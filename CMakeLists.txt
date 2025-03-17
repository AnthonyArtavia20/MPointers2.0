cmake_minimum_required(VERSION 3.10)
project(mi_proyecto_grpc)

# Busca gRPC y Protobuf
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# Incluye los archivos generados por protoc
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/proto)

# Archivos generados por protoc
set(PROTO_SRCS
  src/proto/servicios.pb.cc
  src/proto/servicios.grpc.pb.cc
)

# Archivos del servidor
set(SERVER_SRCS
  src/server/server.cpp
)

# Compila el servidor
add_executable(server ${SERVER_SRCS} ${PROTO_SRCS})

# Vincula las bibliotecas de gRPC y Protobuf
target_link_libraries(server
  gRPC::grpc++
  gRPC::grpc
  gRPC::grpc++_reflection
  ${Protobuf_LIBRARIES}
)