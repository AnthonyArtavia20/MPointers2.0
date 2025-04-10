#include <iostream> //Permite el uso de std::cout, imprimir mensajes por consola.
#include <memory> //Hay que ver si nos sirve esto para manejar punteros en memoria de manera segura.
#include <string> // para poder crear string, como convertir los argumentos en la lista de entrada en string para compararlos al momento de parsear.
#include <grpcpp/grpcpp.h> // Uso de gRPC
#include <grpcpp/health_check_service_interface.h> // de gRPC para verificar que el server funcione correctamente
#include <grpcpp/ext/proto_server_reflection_plugin.h> // Para gRPC
#include <csignal> // para usar señales como SIGNINT para el  Ctrl + c
#include <chrono>
#include <thread>
#include "memory_manager.grpc.pb.h" //Incluye el archivo generado por el compilador de gRPC a partir defl archivo .proto del servicio MemoryService. Este archivo contiene las definiciones de los mensajes y servicios utilizados en el código.
#include <vector> // para la parte del allocator, para crear la lista de bloques libres de memoria en Create.
#include <fstream> // para usar std::ofstream
#include <filesystem>
#include <unordered_map> // uso de un mapa para rastrear ID-contador
using namespace std;

//Se declara el servidor com global para que pueda ser accedido desde el manejador de señales:
std::unique_ptr<grpc::Server> server;

//Variable atómica para indicar que el server debe cerarse:
std::atomic<bool> shutdown_requested(false);

//Se crea el mapa usado en IncreaseRefCount y DecreaseRefCount para mapear los ID
std::unordered_map<uint64_t, int> ref_counts;

// Manejador de señales para capturar SIGINT (Ctrl+C)
void handle_signal(int signal) {
    if (signal == SIGINT) {
        std::cout << "Recibida señal SIGINT, cerrando el servidor..." << std::endl;
        shutdown_requested = true; // Indicar que el servidor debe cerrarse
    }
}

//Creación de la estructura que ayudará a definir los bloques para almacenar espacios de la memoria reservada:
struct BloquesMemoria {
    uint64_t id; //Para poder setear y demás.
    size_t size; //tamaño para almacenar
    bool is_free; //LIbre o ocupado
    void* start; // Donde comienza el bloque
    std::string type; //Indica el tipo de dato para el bloque int, float...
    std::string valueMemory; //El valor del dato que tiene guardado
};

class MemoryServiceImpl final : public memory_manager::MemoryService::Service {
private: //Definición de atributos(variables miembro)
    void* memory_block; //Puntero a un bloque de memoria reservado, donde se almacenarán los datos.
    size_t memory_size; // Tamaño en bytes del bloque de memoria.
    string dump_folder; // Ruta de memoria donde se almacecnarán los registros de cada operación
    uint64_t next_id; // Contador para asignar identificadores únicos a los bloques de memoria.
    std::vector<BloquesMemoria> bloques_memoria; //Estructura para almacenar los bloques de memoria.
    std::thread garbage_collector_thread; // hilo usado para revisar en paralelo cada cierto tiempo las referencias y usar el garbage collector
    std::atomic<bool> stop_garbage_collector{false}; // boolean que activa o desactiva el garbage collector

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

        //Creacion de un hilo para que continuamente se revisen los contadores
        garbage_collector_thread = std::thread(&MemoryServiceImpl::runGarbageCollector, this);
    }

    ~MemoryServiceImpl() { // Destructor, encargado de liberar la memoria reservada.
        //En resumidas cuentas esa "~" indica al programa que eso es el destructor, y es llamado al parar el programa.
        cout << "Ejecutando destructor de MemoryServiceImpl..." << std::endl;

        //Activacion del garbage collector
        stop_garbage_collector = true;
        if (garbage_collector_thread.joinable()) {
            garbage_collector_thread.join();
        }

        free(memory_block);
        cout << "Memoria liberada" << std::endl << std::endl << std::flush; //forzar la salida por la consola.
    }

    //Primer método, creación:
    grpc::Status Create(grpc::ServerContext* context,
                        const memory_manager::CreateRequest* request,
                        memory_manager::CreateResponse* response) override {
        cout << "Create llamado - Tamaño en bytes: " << request->size() << ", Tipo: " << request->type() << endl;

        size_t size_needed = request->size(); //Espacio solicitado por el cliente
        if (size_needed > memory_size) { // verificación para ver si hay espacio suficiente en la reserva total.
            response->set_success(false);
            return grpc::Status::OK;
        }

        //Primera optimización: Buscar un bloque(si está creado y libre) que se ajuste al tamaño del objeto entrante(Por lo general nunca
        // se usa en la primera vez que se ejecuta el programa  si no cuando ya se han hecho varios bloques en la segunda optimización y
        // además varios liberaciones por medio del garbage colector).
        BloquesMemoria* best_block = nullptr; //Creación de un puntero del tipo de estructura que crea bloques, esto para poder almacenar el mejor bloque de todos(el que mejor se ajuste al tamaño solicitado) entre la lista de bloques_memoria.
        for (auto& block : bloques_memoria) {
            if (block.is_free && block.size >= size_needed) { //vemos si el boque actual está libre y si el tamaño disponible es suficiente.
                if (!best_block || block.size < best_block->size) { //Si anteriormente no había un mejor bloque o si hay uno mejor al anterior, entonces lo definimos
                    best_block = &block; //asignamos la dirección de memoria de ese bloque en la lista al puntero.
                }
            }
        }

        //Posteriormente lo que se hace es definir ese bloque como ocupado y luego devolvemos el id.
        if (best_block) {
            best_block->is_free = false;
            best_block->type = request->type();
            ref_counts[best_block->id] = 1; // ✅ Inicializamos refCount
            response->set_id(best_block->id);
            response->set_success(true);
            generarDumpsMemoria();
            return grpc::Status::OK;
        }


        //Segunda optimización: Calcular el espacio libre y crear un nuevo bloque si el espacio es suficiente(Normalmente se usa de primero antes que la PrimeraOptimización)
        size_t free_space = memory_size - next_id;
        if (free_space >= size_needed) {
            void* block_start = static_cast<char*>(memory_block) + next_id;
            BloquesMemoria new_block = {next_id, size_needed, false, block_start, request->type()};
            
            bloques_memoria.push_back(new_block);
            ref_counts[new_block.id] = 1; // ✅ Inicializamos refCount
            next_id += size_needed;

            response->set_id(new_block.id);
            response->set_success(true);
            generarDumpsMemoria();
            return grpc::Status::OK;
        }

        //Tercera optimización, esta se usa si las dos ateriores optimizaciones no se cumplieron(por ejemplo que el vector esté vacio,
        //  que los que tenga adentro no tenga el espacio suficiente, que el espacio total en memoria no sea suficiente). Esto fuciona
        // los espacios libres de los bloques contiguos, haciendo que se pueda reutilizar ese espacio.
        for (auto it = bloques_memoria.begin(); it != bloques_memoria.end(); ++it) {
            if (it->is_free) {
                auto next_it = it + 1; //Siguiente bloque en memoria el "+1" pues con solo sumar 1 ya se llega a donde comienza el otro.
                if (next_it != bloques_memoria.end() && next_it->is_free) {
                    it->size += next_it->size; // Fusionar bloques contiguos
                    bloques_memoria.erase(next_it); // borramos el segundo bloque, pues este ya está fucionado, esto para evitar duplicados

                    if (it->size >= size_needed) {// posteriormente intentamos asignar lo solicitado en el nuevo espacio.
                        it->is_free = false;
                        it->type = request->type();
                        ref_counts[it->id] = 1; // Inicializamos refCount
                        response->set_id(it->id);
                        response->set_success(true);
                        generarDumpsMemoria();
                        return grpc::Status::OK;
                    }
                }
            }
        }

        response->set_success(false);
        return grpc::Status::OK;

    }

    //Segundo método para poder establecer un valor a un bloque de memoria
    grpc::Status Set(grpc::ServerContext* context,
                    const memory_manager::SetRequest* request,
                    memory_manager::SetResponse* response) override {
        cout << "Set - ID: " << request->id() << ", Valor: " << request->value() << endl;

        uint64_t id = request->id();
        const std::string& value = request->value();

        for (auto& block : bloques_memoria) {
            if (block.id == id && !block.is_free) {
            std::istringstream iss(value);

            if (block.type == "int") {
                int val;
                iss >> val;
                memcpy(block.start, &val, sizeof(int));
            } else if (block.type == "float") {
                float val;
                iss >> val;
                memcpy(block.start, &val, sizeof(float));
            } else if (block.type == "bool") {
                bool val;
                iss >> std::boolalpha >> val;
                memcpy(block.start, &val, sizeof(bool));
            } else if (block.type == "char") {
                char val;
                iss >> val;
                memcpy(block.start, &val, sizeof(char));
            } else if (block.type == "double") {
                double val;
                iss >> val;
                memcpy(block.start, &val, sizeof(double));
            } else if (block.type == "long") {
                long val;
                iss >> val;
                memcpy(block.start, &val, sizeof(long));
            } else if (block.type == "uint") {
                uint64_t val;
                iss >> val;
                memcpy(block.start, &val, sizeof(uint64_t));
            } else {
                std::cerr << "Tipo no soportado: " << block.type << std::endl;
                response->set_success(false);
                return grpc::Status::OK;
            }

            block.valueMemory = value;
            response->set_success(true);
            generarDumpsMemoria();
            return grpc::Status::OK;
            }
        }

        response->set_success(false);
        return grpc::Status::OK;
    }

    //Tercer método, get: Permite obtener el valor almacenado en un bloque de memoria.
    grpc::Status Get(grpc::ServerContext* context,
                    const memory_manager::GetRequest* request,
                    memory_manager::GetResponse* response) override {
        cout << "Get - ID: " << request->id() << endl;

        uint64_t id = request->id();

        for (auto& block : bloques_memoria) {
            if (block.id == id && !block.is_free) {
                response->set_value(std::string(static_cast<char*>(block.start)));
                response->set_success(true);
                return grpc::Status::OK;
            }
        }

        response->set_success(false);
        return grpc::Status::OK;
    }


    //cuarto método, incrementar las referencias: Permite incrementar las referencias de un bloque.
    grpc::Status IncreaseRefCount(grpc::ServerContext* context,
                                    const memory_manager::RefCountRequest* request,
                                    memory_manager::RefCountResponse* response) override {
        uint64_t id = request->id();

        for (const auto& block : bloques_memoria) {
            if (block.id == id && !block.is_free) {
                ref_counts[id]++;
                response->set_count(ref_counts[id]);
                response->set_success(true);
                return grpc::Status::OK;
            }
        }

        response->set_success(false);
        return grpc::Status::OK;
    }


    //Quinto método, disminuir la cantidad de referencias de un bloque.
    grpc::Status DecreaseRefCount(grpc::ServerContext* context,
                                const memory_manager::RefCountRequest* request,
                                memory_manager::RefCountResponse* response) override {
        uint64_t id = request->id();

        for (auto& block : bloques_memoria) {
            if (block.id == id && !block.is_free) {
                ref_counts[id]--;
                if (ref_counts[id] <= 0) {
                    block.is_free = true;
                    ref_counts.erase(id); // eliminar el ID del mapa
                }
                response->set_count(ref_counts[id]);
                response->set_success(true);
                generarDumpsMemoria();
                return grpc::Status::OK;
            }
        }

        response->set_success(false);
        return grpc::Status::OK;
    }


    //función para generar los dumps de memoria;
    void generarDumpsMemoria() {
        //Primero creamos el título de cada archivo txt, esto se hace con timestamp
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
        dump_file << "ID\tStart Address\tSize\tStatus\t  Type\tValue\tRefCount\n";
        // Escribir información de cada bloque
    for (const auto& block : bloques_memoria) {
        std::ostringstream valor_actual_stream;

        if (!block.is_free && block.start != nullptr) {
            if (block.type == "int") {
                valor_actual_stream << *static_cast<int*>(block.start);
            } else if (block.type == "float") {
                valor_actual_stream << *static_cast<float*>(block.start);
            } else if (block.type == "bool") {
                valor_actual_stream << (*static_cast<bool*>(block.start) ? "true" : "false");
            } else if (block.type == "double") {
                valor_actual_stream << *static_cast<double*>(block.start);
            } else if (block.type == "long") {
                valor_actual_stream << *static_cast<long*>(block.start);
            } else if (block.type == "uint") {
                valor_actual_stream << *static_cast<uint64_t*>(block.start);
            } else {
                valor_actual_stream << "[tipo no compatible]";
            }
        }

        int ref_count = 0;
        if (ref_counts.find(block.id) != ref_counts.end()) {
            ref_count = ref_counts.at(block.id);
        }

        dump_file << block.id << "\t"
                  << block.start << "\t"
                  << block.size << " bytes\t"
                  << (block.is_free ? "FREE" : "OCCUPIED") << "   "
                  << block.type << "\t"
                  << valor_actual_stream.str() << "\t"
                  << ref_count << "\n";
    }        

        dump_file.close();
        std::cout << "Dump generado: " << filename << std::endl;
    }

    //funcion del garbage collector
    void runGarbageCollector() {
        while (!stop_garbage_collector) {
            std::this_thread::sleep_for(std::chrono::seconds(1)); // cada 5s
    
            for (auto& block : bloques_memoria) {
                if (!block.is_free && ref_counts[block.id] <= 0) { 
                    cout << "[GC] Liberando bloque ID " << block.id << endl;
                    block.is_free = true;
                    ref_counts.erase(block.id);
                    generarDumpsMemoria();
                }
            }
        }
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