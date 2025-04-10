#ifndef MPOINTER_H
#define MPOINTER_H

#include <string>
#include <memory>
#include <type_traits>
#include <sstream>
#include "memory_manager_client.cpp"  // Incluye la clase MemoryManagerClient para poder accerder a los métodos del cliente.
using namespace std;


// Función auxiliar para traducir T a string
template<typename T>
std::string tipoComoTexto() {
    if (std::is_same<T, int>::value) return "int";
    if (std::is_same<T, float>::value) return "float";
    if (std::is_same<T, bool>::value) return "bool";
    if (std::is_same<T, double>::value) return "double";
    if (std::is_same<T, long>::value) return "long";
    if (std::is_same<T, char>::value) return "char";
    if (std::is_same<T, uint64_t>::value) return "uint";
    return "unknown";
}

template <typename T>
class MPointer {
  private:
    uint64_t id; //ID del bloque de memoria en el servidor
    static std::unique_ptr<MemoryManagerClient> client; //Instancia compartida de MemoryManagerClient

  public:
    //Método estático para inicializar conexiones con el server en base a la IP
    static void Init(const std::string& server_address) {
        client = std::make_unique<MemoryManagerClient>(server_address);
    }

    //Método para crear un nuevo bloque de memoria
    static MPointer<T> New() {
      MPointer<T> ptr;
      ptr.id = client->Create(sizeof(T), tipoComoTexto<T>()); // typeid(t).name lo que indica es que tipo de dato es el que se está ingresando
      return ptr;
    }

    // Sobrecarga del operador * para acceder y modificar el valor del bloque de memoria
    class Reference {
      private:
          uint64_t id; // ID del bloque de memoria
          std::unique_ptr<MemoryManagerClient>& client; // Referencia al cliente

      public:
          Reference(uint64_t id, std::unique_ptr<MemoryManagerClient>& client)
              : id(id), client(client) {}

          // Operador de conversión para obtener el valor (lectura)
          operator T() const {
              std::string value = client->Get(id);
              T result;
              std::istringstream iss(value);
              iss >> result;
              return result;
          }

          // Operador de asignación para modificar el valor (escritura)
          Reference& operator=(const T& value) {
              std::ostringstream oss;
              oss << value; // Convertir el valor a string
              client->Set(id, oss.str()); // Enviar el valor al servidor
              return *this;
          }
      };

      // Sobrecarga del operador *
      Reference operator*() {
          return Reference(id, client);
      }

    // Constructor por defecto
    MPointer() = default;

    // Constructor de copia
    MPointer(const MPointer<T>& other) {
        id = other.id;
        if (client) {
            client->IncreaseRefCount(id);
        }
    }


    //sobrecarga del operador = para copiar el ID y aumentar el conteo de referencias.
    MPointer<T>& operator=(const MPointer<T>& other) {
      if (this->id != other.id) { // Evitar auto-asignación
          // Incrementar el conteo de referencias del nuevo ID
          client->IncreaseRefCount(other.id);

          // Decrementar el conteo de referencias del ID actual (si es diferente)
          if (id != other.id) {
              client->DecreaseRefCount(id);
          }

          // Asignar el nuevo ID
          id = other.id;
      }
      return *this;  // Devolver una referencia a este objeto
    }

    //Sobrecarga del operador & para obtener el ID de un bloque de memoria en especifico.
    uint64_t operator&() const {
      return id;
    }

    //Destructor de la clase MPointer, disminuye el coteo de referencias de memoria de un bloque creado que se destruye.
    ~MPointer() {
      if (client) {
        //cout << "Llamando a DecreaseRefCount para ID: " << id << endl;
        client->DecreaseRefCount(id);
      } else {
        cout << "Client es nullptr, no se llama a DecreaseRefCount" << endl;
      }
    }
};

//Inicialización de la instancia compartida de MemoryManagerClient
template <typename T>
std::unique_ptr<MemoryManagerClient> MPointer<T>::client = nullptr;
#endif // MPOINTER_H