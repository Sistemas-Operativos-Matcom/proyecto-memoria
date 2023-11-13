# proyecto-memoria

Implementa las estrategias estudiadas en clase sobre virtualización de la
memoria.

Para compilar y probar las estrategias ejecuta:

```bash
./test.sh
```

## Implementación

En la carpeta `src/managers` se enceuntran los módulos que deben modificar para
la implementación de cada estrategia: `bnb`, `seg` y `pag`.

Las funciones a implementar en cada estrategia sonlas mismas (solo cambia el
nombre seguún la estrategia). A continuación se explica la funcionalidad que
debe realizar cada una de ellas tomando como referencia la estrategia `bnb`:

- `void m_bnb_init(int argc, char **argv)`

    Se llama cada vez que se inicia un caso de prueba. Recibe como parámetros la
    cantidad de argumentos y los argumentos con el que se ejecutó el simulador
    Debes tener en cuenta reinicializar aquellas estructuras globales extras que
    utilices en caso de ser necesario.

- `int m_bnb_malloc(size_t size, ptr_t *out)`

    Reserva un espacio de tamaño `size` en la memoria y modificar el puntero
    `out` con la dirección al espacio reservado. Esta función debe devolver 0 si
    la operación se realizó correctamente, 1 en caso contrario.

- `int m_bnb_free(ptr_t ptr)`

    Libera el espacio de memoria al que apunta `ptr`. Esta función debe devolver 0 si
    la operación se realizó correctamente, 1 en caso contrario.

- `int m_bnb_push(byte val, ptr_t *out)`

    Agrega un valor `val` al stack y modifica el puntero `out` con la dirección
    donde se almacenó el valor.  Esta función debe devolver 0 si la operación se
    realizó correctamente, 1 en caso contrario.

- `int m_bnb_pop(byte *out)`

    Saca un elemento del stack y lo almacena en la variable `out` Esta función
    debe devolver 0 si la operación se realizó correctamente, 1 en caso
    contrario.

- `int m_bnb_load(addr_t addr, byte *out)`

    Lee lo que hay en la dirección `addr` y lo almacena el valor en la variable
    `out` Esta función debe devolver 0 si la operación se realizó correctamente,
    1 en caso contrario.

- `int m_bnb_store(addr_t addr, byte val)`

    Agrega un valor `val` al heap en la dirección `addr`.  Esta función debe
    devolver 0 si la operación se realizó correctamente, 1 en caso contrario.

- `void m_bnb_on_ctx_switch(process_t process)`

    Se ejecuta cada vez que ocurre un cambio de contexto. El parámetro `process`
    representa el nuevo proceso.

- `void m_bnb_on_end_process(process_t process)`

    Se ejecuta cada vez que termina la ejecución del proceso `process`.

Si es necesario modificar la estructura `ptr_t` pueden hacerlo, solo no deben
cambiar su nombre ni eliminar el campo `addr`.

## Logs

La ejecución del proyecto genera unos logs que se almacenan en una carpeta
`mem_logs`. Estos logs contienen información sobre las operaciones que se han
ido ejecutando en la simulación.

Junto con el proyecto hay un script de python con el que pueden plotear una animación del uso de la memoria dado un archivo de log. Por ejemplo:

```python
python ./mem_logs/bnb_case_002.log
```
