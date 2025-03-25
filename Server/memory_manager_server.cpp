#include <iostream> //Permite el uso de std::cout, imprimir mensajes por consola.
#include <memory> //Hay que ver si nos sirve esto para manejar punteros en memoria de manera segura.
#include <string> // para poder crear string, como convertir los argumentos en la lista de entrada en string para compararlos al momento de parsear.
#include <grpcpp/grpcpp.h> // Uso de gRPC
#include <grpcpp/health_check_service_interface.h> // de gRPC para verificar que el server funcione correctamente
#include <grpcpp/ext/proto_server_reflection_plugin.h> // Para gRPC
#include <csignal> // para usar señales como SIGNINT para el  Ctrl + c
#include <chrono>
#include <thread>
#include "../ProtoCompilation/memory_manager.grpc.pb.h" //Incluye el archivo generado por el compilador de gRPC a partir defl archivo .proto del servicio MemoryService. Este archivo contiene las definiciones de los mensajes y servicios utilizados en el código.
#include <vector> // para la parte del allocator, para crear la lista de bloques libres de memoria en Create.
#include <fstream> // para usar std::ofstream
#include <filesystem>
using namespace std;

//Se declara el servidor com global para que pueda ser accedido desde el manejador de señales:
std::unique_ptr<grpc::Server> server;

//Variable atómica para indicar que el server debe cerarse:
std::atomic<bool> shutdown_requested(false);

// Manejador de señales para capturar SIGINT (Ctrl+C)
void handle_signal(int signal) {
    if (signal == SIGINT) {
        std::cout << "Recibida señal SIGINT, cerrando el servidor..." << std::endl;
        shutdown_requested = true; // Indicar que el servidor debe cerrarse
    }
}

//Creación de la estructura que ayudará a definir los bloques para almacenar espacios de la memoria reservada:
struct BloquesMemoria {
    uint64_t id;
    size_t size;
    bool is_free;
    void* start;
};

class MemoryServiceImpl final : public memory_manager::MemoryService::Service {
private: //Definición de atributos(varaibles miembro)
    void* memory_block; //Puntero a un bloque de memoria reservado, donde se almacenarán los datos.
    size_t memory_size; // Tamaño en bytes del bloque de memoria.
    string dump_folder; // Ruta de memoria donde se almacecnarán los registros de cada operación
    uint64_t next_id; // Contador para asignar identificadores únicos a los bloques de memoria.
    std::vector<BloquesMemoria> bloques_memoria; //Estructura para almacenar los bloques de memoria.

public:
    MemoryServiceImpl(size_t size_mb, const std::string& dump_path) // Constructor que inicializa el objeto..
        : next_id(1) { // se inicializa next_id a 1.
        memory_size = size_mb * 1024 * 1024; // Convertir MB a bytes
        memory_block = malloc(memory_size);  // Única asignación de memoria permitida
        dump_folder = dump_path; //Folder para guardar el registro.

        //Creación del dumpFolder en caso de que no exista(medida para generarlo de todos modos...)
        if (!std::filesystem::exists(dump_folder)) {
            if (!std::filesystem::create_directories(dump_folder)) {
                std::cerr << "Error al crear directorio de dump: " << dump_folder << std::endl;
            }
        }

        cout << "Memoria reservada: " << size_mb << " MB" << std::endl;
        cout << "Carpeta de dumps donde se guardó el registro: " << dump_folder << std::endl;
    }

    ~MemoryServiceImpl() { // Destructor, encargado de liberar la memoria reservada.
        //En resumidas cuentas esa "~" indica al programa que eso es el destructor, y es llamado al parar el programa.
        cout << "Ejecutando destructor de MemoryServiceImpl..." << std::endl;
        free(memory_block);
        cout << "Memoria liberada" << std::endl << std::endl << std::flush; //forzar la salida por la consola.
    }

    //Primer método, creación:
    grpc::Status Create(grpc::ServerContext* context,
                       const memory_manager::CreateRequest* request,
                       memory_manager::CreateResponse* response) override {
        cout << "Create llamado - Tamaño en bytes: " << request->size() << ", Tipo: " << request->type() << endl;

        //Verificación del tamaño disponible para confirmar la reservar si el disponible es suficiente.
        size_t size_needed = request->size();
        if(size_needed > memory_size) {
            response->set_success(false); // En caso de que el tamaño no sea suficiente.
            return grpc::Status::OK;
        }

        //Allocator: Implemementación que evita la desfragmentación, busca un bloque libre que sea lo suficientemente grande.
        for (auto& block : bloques_memoria) {
            if (block.is_free && block.size >= size_needed) {
                block.is_free = false;
                response->set_id(block.id);
                response->set_success(true);
                return grpc::Status::OK;
            }
        }

        //Si no se encuentra un bloque libre lo que hacemos es crear uno:
        if (next_id + size_needed > memory_size) {
            response->set_success(false);
            return grpc::Status::OK;
        }

        void* block_start = static_cast<char*>(memory_block) + next_id;
        BloquesMemoria new_block = {next_id, size_needed,false, block_start};
        bloques_memoria.push_back(new_block);
        next_id += size_needed;

        //Una vez asignado, se devuelve el id del bloque, para ponerle un valor o así...
        response->set_id(new_block.id);
        response->set_success(true);

        //Generación del DumpDeMemoria:
        generarDumpsMemoria();

        return grpc::Status::OK;
    }
    //Segundo método para poder establecer un valor a un bloque de memoria
    grpc::Status Set(grpc::ServerContext* context,
                    const memory_manager::SetRequest* request,
                    memory_manager::SetResponse* response) override {
        cout << "Set llamado - ID: " << request->id() << endl;

        // Aquí se implementará la lógica real para guardar el valor en memory_block
        response->set_success(true);

        //Generación del DumpDeMemoria:
        generarDumpsMemoria();

        return grpc::Status::OK;
    }

    //Tercer método, get: Permite obtener el valor almacenado en un bloque de memoria.
    grpc::Status Get(grpc::ServerContext* context,
                    const memory_manager::GetRequest* request,
                    memory_manager::GetResponse* response) override {
        cout << "Get llamado - ID: " << request->id() << endl;

        // Aquí se implementará la lógica real para obtener el valor de memory_block
        // Por ahora, solo devolvemos un valor vacío
        response->set_success(true);

        return grpc::Status::OK;
    }

    //cuarto método, incrementar las referencias: Permite incrementar las referencias de un bloque.
    grpc::Status IncreaseRefCount(grpc::ServerContext* context,
                               const memory_manager::RefCountRequest* request,
                               memory_manager::RefCountResponse* response) override {
        cout << "IncreaseRefCount llamado - ID: " << request->id() << endl;

        // Aquí hay que ver como implementamos ese contador de referencias del bloque
        response->set_count(1); // Valor ficticio por ahora
        response->set_success(true);

        return grpc::Status::OK;
    }

    //Quinto método, disminuir la cantidad de referencias de un bloque.
    grpc::Status DecreaseRefCount(grpc::ServerContext* context,
                               const memory_manager::RefCountRequest* request,
                               memory_manager::RefCountResponse* response) override {
        cout << "DecreaseRefCount llamado - ID: " << request->id() << endl;

        // Aquí se decrementaría la cantidad de referencias en un bloque dado
        // y se liberaría en algún momento por el garbage collector.
        response->set_count(0); // Valor ficticio por ahora
        response->set_success(true);

        //Generación del DumpDeMemoria:
        generarDumpsMemoria();

        return grpc::Status::OK;
    }

    //función para generar los dumps de memoria;
    void generarDumpsMemoria() {
        //Primero creamos el título de cada archivo txt, esto se hace ocn timestamp
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto epoch = now_ms.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        std::string filename = dump_folder + "/dump_" + std::to_string(value.count()) + ".txt";

        // Abrir archivo
        std::ofstream dump_file(filename);
        if (!dump_file.is_open()) {
            std::cerr << "Error al crear dump: " << filename << std::endl;
            return;
        }

        // Escribir metadatos
        dump_file << "===== Memory Dump =====\n";
        dump_file << "Timestamp: " << value.count() << " ms\n";
        dump_file << "Memoria total reservada: " << memory_size << " bytes\n";

        // Calcular memoria libre/ocupada
        size_t used_memory = 0;
        for (const auto& block : bloques_memoria) { // Se itera por cada bloque y vamos contando de aquellos que estén ocupados lo que tienen reservado.
            if (!block.is_free) used_memory += block.size;
        }
        dump_file << "Memoria Usada: " << used_memory << " bytes ("
                  << (used_memory * 100 / memory_size) << "%)\n\n";

        // Listar bloques
        dump_file << "Blocks:\n";
        dump_file << "ID\tStart Address\tSize\tStatus\tType\n"; // Añade "Value" luego
        for (const auto& block : bloques_memoria) {
            dump_file << block.id << "\t"
                      << block.start << "\t"
                      << block.size << " bytes\t"
                      << (block.is_free ? "FREE" : "OCCUPIED") << "\t"
                      << "Unknown\n"; // Reemplaza "Unknown" con el tipo cuando lo tengas
        }

        dump_file.close();
        std::cout << "Dump generado: " << filename << std::endl;
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

    // Usar la variable global `server` en lugar de crear una local
    server = builder.BuildAndStart(); // <-- Aquí se usa la variable global
    cout << "Server escuchando en " << server_address << endl;

    // Bucle principal para verificar si se ha solicitado el cierre
    while (!shutdown_requested) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Esperar 100 ms
    }

    // Cerrar el servidor de manera controlada
    server->Shutdown();
    server->Wait();
    cout << "Servidor cerrado correctamente" << endl;
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
            cerr << "Uso: ./mem-mgr --port PUERTO --memsize TAMAÑO_MB --dumpFolder CARPETA_DUMP" << endl;
            return 1;
        }
    }

    //Se configura el manejador de señales para captuurar SIGINT (Ctrl+C)
    std::signal(SIGINT, handle_signal);

    RunServer(port, size_mb, dump_folder); //Si todo bien, pasamos los argumentos parseados para configurar el server.

    return 0;
}