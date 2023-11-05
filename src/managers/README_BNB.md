# Base and Bounds BNB

## Puntualizaciones

A la hora de ejecutar los test no se puede reservar una cantidad de memoria mayor que el _bound_ por lo que el valor de dicha variable se puede modificar de ser necesario en la línea 13:

`static size_t bb_value = 512;`

## Implementación

### Estructuras utilizadas

- `typedef struct Process
{
    int pid;
    size_t base;
    size_t bound;
    size_t *memory;
    stack *s;
} process_bb;`

  _process_bb_ representa un proceso de base and bound, está compuesto por su pid para identificarlo, su base, su bound, un array _memory_ donde su longitud es la misma del bound y representa la memoria virtual del proceso. También cada proceso tiene un stack que almacena las posiciones de memoria donde se introducen los valores que van al stack.

- `typedef struct List
  {
  size_t len;
  size_t size;
  process_bb *data;
} pList;`

  _pList_ representa la lista de procesos activos ( _process_bb_ ) de la simulación.

### Variables globales

- `static addr_t puntero;`

  Representa donde se debe empezar a buscar la memoria libre de cada proceso, su valor es el siguiente espacio de memoria libre luego de reservar el _size_ del código del programa en cuestión.

- `static process_bb *process_act;`

  Representa el proceso actual en el que se realizan todas las operaciones. Su valor cambia solo cuando se realiza un cambio de contexto de proceso( _m_bnb_on_ctx_switch_ ).

- `static pList *l;`

  Lista de procesos activos.

- `static size_t *virtual_mem;`

  Representa la memoria física convertida a virtual de la simulación, cada posición significa una cantidad igual al valor de _bound_ pero en memoria física, obteniéndose el valor de esta última multiplicando el bound por la posición. Es un array de bool donde cada posición ocupada es un 1 y cada vez que se crea un proceso nuevo este busca una posicion libre de dicho array y la marca como ocupada.

- `static size_t virtual_mem_c;`

  Representa la longitud del array anterior, se obtiene dividiendo la cantidad de memoria física de la simulación entre el valor de _bound_.

- `static size_t bb_value = 512;`

  El valor de bound, es el mismo para cada proceso.

### Funciones

- `int m_bnb_malloc(size_t size, ptr_t *out)`

  Reserva un espacio en el heap de tamaño _size_ y establece un puntero al inicio del espacio reservado, el heap por cada programa inicia luego de reservado el código del programa (el valor del _puntero_).
  Se busca en el array de memoria virtual del proceso si hay un espacio contiguo libre igual al _size_ que se desea reservar y de ser así se reserva y se devuelve el valor del puntero correspondiente.
  Si no se dispone de tamaño suficiente se lanza un error.

- `int m_bnb_free(ptr_t ptr)`

  Dado el puntero libera de forma contigua una cantidad igual a su _size_ .

- `int m_bnb_push(byte val, ptr_t *out)`

  Se comienza a buscar un espacio libre desde la última posición de la memoria virtual del proceso hacia arriba, al encontrarse se hace push al _stack_ del proceso la posición donde se guardó el valor y también se escribe en la memoria física correspondiente.

- `int m_bnb_pop(byte *out)`

  Al hacer pop al stack se obtiene la posición de la memoria virtual donde se guardó el último valor que se hizo push al stack, por lo tanto se libera esa posición de la memoria virtual y se lee y devuelve de la memoria física.

- `int m_bnb_load(addr_t addr, byte *out)`

  Convierte la dirección virtual recibida en física. Se lee de la memoria física y luego se devuelve ese valor.

- `int m_bnb_store(addr_t addr, byte val)`

  Convierte la dirección virtual recibida en física y escribe el valor correspondiente en dicha posición.

- `void m_bnb_on_ctx_switch(process_t process)`

  Verifica si el proceso ya está creado, si lo está entonces se pone como actual ( _process_act_ ). Si no está creado se crea, se pone como actual y al inicio de su memoria virtual reserva el size del código del programa, luego se guarda en la lista de programas.

- `void m_bnb_on_end_process(process_t process)`

  Elimina un proceso de la lista de procesos y también libera toda su memoria correspondiente.
