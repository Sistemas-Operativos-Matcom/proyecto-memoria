# Base and Bounds BNB

## Puntualizaciones

El tamaño de página debe ser una potencia de dos
y debe de haber como mínimo cantidad de páginas igual a cantidad de programas en la simulación. Dicho valor de tamaño de página se puede modificar en la línea 8 y en la línea 9 la potencia de dos correspondiente:

`static size_t pag_size = 128;`

`static size_t pag_size_inb = 7;`

## Implementación

### Estructuras utilizadas

- `typedef struct Process
{
    int pid;
    size_t *pag_process;
    size_t pag_process_c;
    size_t **pag_process_free;
    stack *s;
} process_pag;`

  _process_pag_: es un proceso de _paginación_, está compuesto por su pid para identificarlo, un array _\*pag_process_ que representa la tabla de páginas del proceso actual donde _pag_process_c_ es la cantidad de páginas que ha ocupado el proceso. También tiene un array de arrays de bool _\*\*pag_process_free_ que representa la memoria particular de cada página ocupada por el proceso, cada posición tiene un array del tamaño de página donde este último tiene un 1 si está ocupada esa dirección (de size 1) y diferente de 1 si está libre.

- `typedef struct List
  {
  size_t len;
  size_t size;
  process_pag *data;
} pList;`

  _pList_ representa la lista de procesos activos ( _process_pag_ ) de la simulación.

### Variables globales

- `static process_pag *process_act;`

  Representa el proceso actual en el que se realizan todas las operaciones. Su valor cambia solo cuando se realiza un cambio de contexto de proceso( _m_bnb_on_ctx_switch_ ).

- `static pList *process_l;`

  Lista de procesos activos.

- `static size_t *pag_table_frame;`

  Representa la memoria física dividida en páginas, cada posición es una página que se traduce en memoria física como una cantidad igual al tamaño de página, tiene valor 1 si esa página está ocupada por algún proceso y diferente de 1 si está libre.

- `static size_t pag_table_frame_c;`

  Representa la longitud del array anterior, se obtiene dividiendo la cantidad de memoria física de la simulación entre el valor del tamaño de página ( _pag_size_ ) .

- `static size_t pag_size = 128;`

  El valor del tamaño de página.

- `static size_t pag_size_inb = 7;`

  Representa la potencia de dos tal que 2^n = pag_size. Ejemplo: 2^7 = 128.

### Funciones

- `int m_bnb_malloc(size_t size, ptr_t *out)`

  Se divide en dos casos:

  - El valor _size_ a reservar es mayor que el tamaño de página: se calcula cuantas páginas ocupará dicha operación y se colocan de manera contigua al final de la tabla de páginas del proceso. El puntero devuelto tomaría el valor de la posición inicial de la primera página que se reservó.
  - El valor _size_ a reservar es menor o igual al tamaño de página: se comienza a buscar desde el inicio de la tabla de páginas si hay un espacio de memoria libre de manera contigua igual al _size_ que se quiere reservar, si se encuentra se reserva y devuelve el puntero correspondiente, si no se encuentra entonces se busca una página nueva y se reserva al inicio de dicha página.

- `int m_bnb_free(ptr_t ptr)`

  Similar al malloc, se divide en dos casos:

  - El size es mayor que el tamaño de página: se obtiene del addr la página que corresponde a la primera reservada y a partir de ahí se comienza a liberar la cantidad de páginas que ocupa la operación. Paralelamente se libera la página de memoria física.
  - El size es menor o igual que el tamaño de página: se obtiene del addr la página y posición dentro de la página y se libera el espacio correspondiente.

- `int m_bnb_push(byte val, ptr_t *out)`

  Se comienza a buscar por la tabla de páginas una posición libre la cual ocupar, al obtenerla se le hace push al stack la posición donde se almacena el valor _val_. En caso de no existir posición libre se reserva una página nueva y se almacena al inicio.

- `int m_bnb_pop(byte *out)`

  Al hacer pop al stack se obtiene el _addr_ del último valor al que se le hizo push, a partir de él se obtiene la página y posición dentro de la página de dicho _val_, luego se lee y se devuelve.

- `int m_bnb_load(addr_t addr, byte *out)`

  De la dirección _addr_ recibida se obtiene la página y posición dentro de la página, luego se lee el valor correspondiente y se devuelve.

- `int m_bnb_store(addr_t addr, byte val)`

  De la dirección _addr_ recibida se obtiene la página y posición dentro de la página, luego se escribe el valor correspondiente en dicha posición.

- `void m_bnb_on_ctx_switch(process_t process)`

  Verifica si el proceso ya está creado, si lo está entonces se pone como actual ( _process_act_ ). Si no está creado se crea, se pone como actual y se reserva el size del código del programa, luego se guarda en la lista de programas.

- `void m_bnb_on_end_process(process_t process)`

  Elimina un proceso de la lista de procesos y también libera toda su memoria correspondiente.
