#include "ClaseMPointer.h"
#include <iostream>

// ======= Lista enlazada con MPointers (sin structs anidados) =======

class ListaEnlazada {
private:
    MPointer<int> valores[100];         // Lista de nodos (máx 100 nodos)
    MPointer<uint64_t> siguientes[100]; // Enlaces a los IDs del siguiente nodo
    MPointer<uint64_t> cabeza;          // ID del nodo cabeza
    int contador;                       // Cantidad de nodos insertados

public:
    ListaEnlazada() {
        cabeza = MPointer<uint64_t>::New();
        *cabeza = 0; // 0 representa null
        contador = 0;
    }

    void insertarAlInicio(int valor) {
        // Crear y asignar valor al nodo actual
        valores[contador] = MPointer<int>::New();
        *valores[contador] = valor;

        // Crear y asignar enlace al nodo siguiente
        siguientes[contador] = MPointer<uint64_t>::New();
        uint64_t idCabeza = *cabeza; // Obtener ID del nodo actual cabeza
        *siguientes[contador] = idCabeza; // El nuevo nodo apunta al anterior cabeza

        // Actualizar cabeza al nuevo nodo
        *cabeza = &valores[contador]; // Guardar ID del nuevo nodo como cabeza

        contador++;
    }

    void imprimir() {
        uint64_t actual = *cabeza;

        while (actual != 0) {
            bool encontrado = false;
            for (int i = 0; i < contador; i++) {
                if (&valores[i] == actual) {
                    std::cout << *valores[i] << " -> ";
                    actual = *siguientes[i];
                    encontrado = true;
                    break;
                }
            }
            if (!encontrado) break; // Enlace roto o ID inválido
        }
        std::cout << "NULL" << std::endl;
    }
};

// ======= MAIN PRINCIPAL (conservando tus pruebas originales) =======

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <dirección_del_servidor>" << std::endl;
        return 1;
    }

    std::string server_address = argv[1];
    MPointer<int>::Init(server_address);
    MPointer<uint64_t>::Init(server_address); // Necesario para punteros a IDs

    // === Pruebas individuales ===
    MPointer<int> myPtr = MPointer<int>::New();
    *myPtr = 1234;
    int value = *myPtr;
    std::cout << "Valor almacenado: " << value << std::endl;

    MPointer<int> myPtr2 = MPointer<int>::New();
    *myPtr2 = 12345;
    int value2 = *myPtr2;
    std::cout << "Valor almacenado: " << value2 << std::endl;

    // === Lista enlazada con MPointers ===
    std::cout << "\n== Lista enlazada usando MPointers ==" << std::endl;
    ListaEnlazada lista;
    lista.insertarAlInicio(30);
    lista.insertarAlInicio(20);
    lista.insertarAlInicio(10);

    std::cout << "Lista: ";
    lista.imprimir();

    return 0;
}
