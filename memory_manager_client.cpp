#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "memory_manager.grpc.pb.h"

class MemoryManagerClient { // Clase para el RPC - cliente
public:
    MemoryManagerClient(const std::string& server_address) //Construcctor que inicializa el cliente contentándolo al server en específico.
        : stub_(memory_manager::MemoryService::NewStub(
            grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()))) {
        std::cout << "Cliente conectado a: " << server_address << std::endl;
        //Nota personal: El uso de InsecureChannelCredetials está ok, pero no es recoemdando para un proyecto real.
    }

    //Listado de métodos:

    // Crear un nuevo bloque de memoria
    uint64_t Create(uint32_t size, const std::string& type) {
        memory_manager::CreateRequest request;
        request.set_size(size);
        request.set_type(type);

        memory_manager::CreateResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Create(&context, request, &response);

        if (status.ok()) {
            if (response.success()) {
                std::cout << "Bloque creado con ID: " << response.id() << std::endl;
                return response.id();
            } else {
                std::cerr << "Error al crear bloque" << std::endl;
                return 0;
            }
        } else {
            std::cerr << "Error RPC: " << status.error_message() << std::endl;
            return 0;
        }
    }

    // Establecer un valor en un bloque de memoria
    bool Set(uint64_t id, const std::string& value) {
        memory_manager::SetRequest request;
        request.set_id(id);
        request.set_value(value);

        memory_manager::SetResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Set(&context, request, &response);

        if (status.ok()) {
            return response.success();
        } else {
            std::cerr << "Error RPC: " << status.error_message() << std::endl;
            return false;
        }
    }

    // Obtener un valor de un bloque de memoria
    std::string Get(uint64_t id) {
        memory_manager::GetRequest request;
        request.set_id(id);

        memory_manager::GetResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Get(&context, request, &response);

        if (status.ok() && response.success()) {
            return response.value();
        } else {
            std::cerr << "Error al obtener valor" << std::endl;
            return "";
        }
    }

    // Incrementar el contador de referencias
    bool IncreaseRefCount(uint64_t id) {
        memory_manager::RefCountRequest request;
        request.set_id(id);

        memory_manager::RefCountResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->IncreaseRefCount(&context, request, &response);

        if (status.ok()) {
            std::cout << "RefCount incrementado a: " << response.count() << std::endl;
            return response.success();
        } else {
            std::cerr << "Error RPC: " << status.error_message() << std::endl;
            return false;
        }
    }

    // Decrementar el contador de referencias
    bool DecreaseRefCount(uint64_t id) {
        memory_manager::RefCountRequest request;
        request.set_id(id);

        memory_manager::RefCountResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->DecreaseRefCount(&context, request, &response);

        if (status.ok()) {
            std::cout << "RefCount decrementado a: " << response.count() << std::endl;
            return response.success();
        } else {
            std::cerr << "Error RPC: " << status.error_message() << std::endl;
            return false;
        }
    }

private:
    std::unique_ptr<memory_manager::MemoryService::Stub> stub_;
};

// Esta es una función de prueba simple
int main(int argc, char** argv) {
    // Parametrizar el servidor si se proporciona, de lo contrario usar localhost:50051
    std::string server_address = "localhost:50051";
    if (argc > 1) {
        server_address = argv[1];
    }

    MemoryManagerClient client(server_address);

    // Ejemplo de uso del cliente
    uint64_t id = client.Create(sizeof(int), "int");
    if (id > 0) {
        std::string value = "42"; // En la implementación real, esto sería el valor serializado
        if (client.Set(id, value)) {
            std::string retrieved = client.Get(id);
            std::cout << "Valor recuperado: " << retrieved << std::endl;
        }

        client.IncreaseRefCount(id);
        client.DecreaseRefCount(id);
    }

    return 0;
}