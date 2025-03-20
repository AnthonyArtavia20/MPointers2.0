#include <iostream> //Permite el uso de std::cout, imprimir mensajes por consola.
#include <memory> //Hay que ver si nos sirve esto para manejar punteros en memoria de manera segura.
#include <string> // para poder crear string, como convertir los argumentos en la lista de entrada en string para compararlos al momento de parsear.
#include <grpcpp/grpcpp.h> // Uso de gRPC
#include <grpcpp/health_check_service_interface.h> // de gRPC para verificar que el server funcione correctamente
#include <grpcpp/ext/proto_server_reflection_plugin.h> // Para gRPC
#include "../ProtoCompilation/memory_manager.grpc.pb.h" //Incluye el archivo generado por el compilador de gRPC a partir defl archivo .proto del servicio MemoryService. Este archivo contiene las definiciones de los mensajes y servicios utilizados en el código.

class MemoryServiceImpl final : public memory_manager::MemoryService::Service {
private: //Definición de atributos(varaibles miembro)
    void* memory_block; //Puntero a un bloque de memoria reservado, donde se almacenarán los datos.
    size_t memory_size; // Tamaño en bytes del bloque de memoria.
    std::string dump_folder; // Ruta de memoria donde se almacecnarán los registros de cada operación
    uint64_t next_id; // Contador para asignar identificadores únicos a los bloques de memoria.

public:
    MemoryServiceImpl(size_t size_mb, const std::string& dump_path) // Constructor que inicializa el objeto..
        : next_id(1) { // se inicializa next_id a 1.
        memory_size = size_mb * 1024 * 1024; // Convertir MB a bytes
        memory_block = malloc(memory_size);  // Única asignación de memoria permitida
        dump_folder = dump_path; //Folder para guardar el registro.

        std::cout << "Memoria reservada: " << size_mb << " MB" << std::endl;
        std::cout << "Carpeta de dumps donde se guardó el registro: " << dump_folder << std::endl;
    }

    ~MemoryServiceImpl() { // Destructor, encargado de liberar la memoria reservada.
        //En resumidas cuentas esa "~" indica al programa que eso es el destructor, y es llamado al parar el programa.
        free(memory_block);
        std::cout << "Memoria liberada" << std::endl;
    }

    //Primer método, creación:
    grpc::Status Create(grpc::ServerContext* context,
                       const memory_manager::CreateRequest* request,
                       memory_manager::CreateResponse* response) override {
        std::cout << "Create llamado - Tamaño: " << request->size() << ", Tipo: " << request->type() << std::endl;

        // Aquí se implementará la lógica real para reservar un bloque en memory_block
        // Por ahora, solo devolvemos un ID incremental
        response->set_id(next_id++);
        response->set_success(true);

        // Generar dump de memoria (implementar después)

        return grpc::Status::OK;
    }
    //Segundo método para poder establecer un valor a un bloque de memoria
    grpc::Status Set(grpc::ServerContext* context,
                    const memory_manager::SetRequest* request,
                    memory_manager::SetResponse* response) override {
        std::cout << "Set llamado - ID: " << request->id() << std::endl;

        // Aquí se implementará la lógica real para guardar el valor en memory_block
        response->set_success(true);

        // Generar dump de memoria (implementar después)

        return grpc::Status::OK;
    }

    //Tercer método, get: Permite obtener el valor almacenado en un bloque de memoria.
    grpc::Status Get(grpc::ServerContext* context,
                    const memory_manager::GetRequest* request,
                    memory_manager::GetResponse* response) override {
        std::cout << "Get llamado - ID: " << request->id() << std::endl;

        // Aquí se implementará la lógica real para obtener el valor de memory_block
        // Por ahora, solo devolvemos un valor vacío
        response->set_success(true);

        return grpc::Status::OK;
    }

    //cuarto método, incrementar las referencias: Permite incrementar las referencias de un bloque.
    grpc::Status IncreaseRefCount(grpc::ServerContext* context,
                               const memory_manager::RefCountRequest* request,
                               memory_manager::RefCountResponse* response) override {
        std::cout << "IncreaseRefCount llamado - ID: " << request->id() << std::endl;

        // Aquí hay que ver como implementamos ese contador de referencias del bloque
        response->set_count(1); // Valor ficticio por ahora
        response->set_success(true);

        return grpc::Status::OK;
    }

    //Quinto método, disminuir la cantidad de referencias de un bloque.
    grpc::Status DecreaseRefCount(grpc::ServerContext* context,
                               const memory_manager::RefCountRequest* request,
                               memory_manager::RefCountResponse* response) override {
        std::cout << "DecreaseRefCount llamado - ID: " << request->id() << std::endl;

        // Aquí se decrementaría la cantidad de referencias en un bloque dado
        // y se liberaría en algún momento por el garbage collector.
        response->set_count(0); // Valor ficticio por ahora
        response->set_success(true);

        // Generar dump de memoria (implementar después)

        return grpc::Status::OK;
    }
};

void RunServer(int port, size_t size_mb, const std::string& dump_folder) { //Recibimos los argumentos parseados del main.
    std::string server_address = "0.0.0.0:" + std::to_string(port); // 1.Crea la dirección del servidor.
    MemoryServiceImpl service(size_mb, dump_folder); // 2. Inicialización del servicio creado, MemoryServiceImpl

    grpc::EnableDefaultHealthCheckService(true); //3.configuraciones adicionales de gRPC
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    grpc::ServerBuilder builder; //4. Construcción del servidor.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart()); // 5. Inicialización del servidor.
    std::cout << "Server escuchando en " << server_address << std::endl;

    server->Wait(); //6. Mantiene el server en ejecución.
}

int main(int argc, char** argv) { //Ciclo principal del servidor.
    /*
    Explicación de como funciona este main:
    argc: es la cantidad de argumentos que llegan al ejecutar el programa: "./mem-mgr --port 8080 --memsize 512 --dumpFolder /mnt/mem-dumps"
        literal cada espacio es un argumen, "./mem-mgr" "--port". ...

    argv: array con cada uno de esos argumentos.
    */

    int port = 50051; //puerto definido por defecto, se actualiza luego si se ingresa otro.
    size_t size_mb = 100; //tamaño por defecto a almacenar.
    std::string dump_folder = "../dumpFolderRegistros"; //./dumps es una carpeta predeterminada donde se guardan cosas generadas por el programa.

    // Aquí se parsean los argumentos ingresados por línea de comandos
    for (int i = 1; i < argc; i++) { //Posterior se itera sobre argv para parsear los argumentos e ir configurando el server.
        std::string arg = argv[i];//va conviertiendo cada argumento almacenado en argv en una string para poder compararlo con el argumento esperado.

        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--memsize" && i + 1 < argc) {
            size_mb = std::stoi(argv[++i]);
        } else if (arg == "--dumpFolder" && i + 1 < argc) {
            dump_folder = argv[++i];//Aquí asignanmos "/mnt/mem-dumps" como la carpeta para guardar en lugar de la ./dumps
        } else { //En caso de que algún argumento no sea correcto, indicamos la estructura
            std::cerr << "Uso: ./mem-mgr --port PUERTO --memsize TAMAÑO_MB --dumpFolder CARPETA_DUMP" << std::endl;
            return 1;
        }
    }

    RunServer(port, size_mb, dump_folder); //Si todo bien, pasamos los argumentos parseados para configurar el server.

    return 0;
}