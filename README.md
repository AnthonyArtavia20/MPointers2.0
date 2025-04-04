## MPointers 2.0


### ¿Cómo usar?

##### Se  descargan los archivos aquí disponibles y posterior se abre la carpeta en VisualStudioCode, luego desde la terminal ejecutar la siguiente serie de comandos:

##### 1) Iniciar el servidor:
 Se realiza la navegación hasta la carpeta "Build" situada en la raíz principal de proyecto por medio del comando:

`C:\Users\ruta> cd build`

Posterior se debe ejecutar el siguiente comando desde la terminal estando dentro de dicho directorio:
`C:\Users\ruta\build> ./mem-mgr --port 50051 --memsize 100 --dumpFolder ../dumpFolderRegistros`
+ Aquí --port indica por cual puerto TCP/IP de los 65535 se iniciará el servicio.
+ --memsize indica la cantidad de memoria que el servidor reservará en el heap con malloc
+ --dumpFolder indica el directorio donde se almacenarán los registros y estado de memoria, este no se deberá alterar pues ya hay un folder para esto.


Luego por la consola se indicará el inicio exitoso, mostrando la cantidad de memoria reservada y la carpeta donde se guardarán los registros.

##### 2) Iniciar el Cliente:
Similar a iniciar el servidor, se deberá navegar a la carpeta build nuevamente, pero esta vez desde otra terminal, esta será dedicada exclusivamente para cliente.
Estando en dicha carpeta en otra terminal, se deberá ejecutar el siguiente comando:

`C:\Users\ruta\build> ./mem-client localhost:50051`
+ localhost indica la dirección IP por defecto, siendo 0.0.0.0
+ la parte ":" es un separador
+ 50051 es no de los puertos entre los 65535, el del ejemplo es recomendable, pues no se utiliza normalmente para un servicio importante.

## ¿Cómo funciona este programa?
##### Este programa está hecho en el lenguaje de programación c++ utilizando a su vez la integración del framework gRPC para el modelo Cliente - Servidor, esto para lograr servicios comunicables. El proyecto consiste en dos componentes principales: el administrador de memoria y la biblioteca MPointers. El administrador de memoria reserva un bloque de memoria de cierto tamaño y lo administra. La biblioteca MPointers permite a las aplicaciones que lo usen, interactuar con el administrador de memoria y el bloque de memoria reservado por este.

Este programa aplica los conceptos fundamentales de la programación orientada a objetos (POO) y aplicación de patrones de diseño en la solución de un problema

##### Operaciones soportadas:
+ **Create (size, type):** Crea un bloque en la memoria reservada en el server para el tamaño y tipo de datos indicado en la petición. Retorna un id que pertenece al espacio generado.
+ **Set(id, value):** guarda un valor determinado en la posición de memoria indicado por Id.
+ **Get(id):**  retorna el valor guardado en el bloque de memoria identificado por el Id devuelto al hacer el create.

Las siguiente operaciones se ejecutan de forma automáticas en MPointers.
+ **IncreaseRefCount(id):** incrementa el conteo de referencias para el bloque indicado por Id
+ **DecreaseRefCount(Id):** decrementa el conteo de referencias para el bloque indicado por
Id.
+ **GarbageCollector**: Este es un hilo que cada cierto tiempo está buscando bloques con referencias en 0, liberando la memoria en ellos para lograr optimizarla posteriormente con la función Create al momento de hacer un nuevo bloque de memoria.
