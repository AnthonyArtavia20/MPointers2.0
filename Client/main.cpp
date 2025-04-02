#include "ClaseMPointer.h"

int main(int argc, char** argv) {
    // Verificar que se recibió dirección IP desde el servidor.
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <dirección_del_servidor>" << std::endl;
        return 1;
    }

    // Inicializar la conexión con el servidor
    std::string server_address = argv[1]; // Ejemplo: "localhost:50051"
    MPointer<int>::Init(server_address);

    // Crear un nuevo MPointer
    MPointer<int> myPtr = MPointer<int>::New();

    // Asignar un valor al bloque de memoria
    *myPtr = 1234;  // Esto ahora funciona correctamente

    // Obtener el valor almacenado:
    int value = *myPtr;
    std::cout << "Valor almacenado: " << value << std::endl;

    // Crear un nuevo MPointer
    MPointer<int> myPtr2 = MPointer<int>::New();

    // Asignar un valor al bloque de memoria
    *myPtr2 = 12345;  // Esto ahora funciona correctamente

    // Obtener el valor almacenado:
    int value2 = *myPtr2;
    std::cout << "Valor almacenado: " << value << std::endl;

    return 0;
}