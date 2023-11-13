#include "pag_manager.h"
#include "stdio.h"
#include <string.h>

#define stackSz 1024
#define pagSz 1024
#define maxProcs 1024
#define maxMem 1000000
#define maxPags 128

int fromPID[maxMem];
int memAlloc[maxMem];
int stackPoint[maxProcs];
int curPID = 0;
int lastp = -1;
unsigned char pageMem[maxMem];
size_t stackPos[maxProcs][stackSz];

struct pageTable{
    int pageFrame[maxPags];
    int used[maxPags];
    int mk[maxPags][pagSz];
} table[maxProcs];


// Devuelve la direccion virtual correspondiente a un par de VNP y offset dados
size_t to_addr(int vpn, int offset){ 
  return offset | (vpn << 10);
}


// Devuelve el VNP y offset correspondientes a una direccion dada 
void decode(size_t address, int *vpn, int *offset){
  *offset = address & 0x3FF;          
  *vpn = (address >> 10) & 0x3FFFFF;  
}


// Devuelve la posicion en memoria correspondiente a la direccion dada
int mem_pos(size_t addr){
  int vpn = 0, offset = 0;
  decode(addr, &vpn, &offset);
  int pt = table[curPID].pageFrame[vpn];
  if(fromPID[pt * pagSz + offset] != curPID)return -1;
  return pt * pagSz + offset;
}


// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv){
  // Inicializa todas las estructuras utilizadas
  memset(memAlloc, 0, sizeof(memAlloc[0]) * maxMem);
  memset(fromPID, -1, sizeof(fromPID[0]) * maxMem);
  memset(pageMem, 0, sizeof(pageMem[0]) * maxMem);
  memset(stackPoint, 0, sizeof(stackPoint[0]) * maxProcs);

  for (int i = 0; i < maxProcs; i++){
    memset(stackPos[i], 0, sizeof(stackPos[i][0]) * stackSz);
    memset(table[i].pageFrame, 0, sizeof(table[i].pageFrame[0]) * maxPags);
    memset(table[i].used, 0, sizeof(table[i].used[0]) * maxPags);
    memset(table[i].mk, 0, sizeof(table[i].mk[0][0]) * maxPags * pagSz);
  }
}


// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out){
  int vpn = -1, offset = 0;

  // Itera sobre las páginas del heap para encontrar un espacio continuo lo suficientemente grande
  for(int i = 0; i < maxPags; i++){
    if(pagSz - table[curPID].used[i] >= (int)size){
      vpn = i; 
      offset = table[curPID].used[i];

      // Si la página no ha sido usada antes, se asigna un marco de página y se marca como usado
      if(!table[curPID].used[i]){
        lastp++;
        table[curPID].pageFrame[i] = lastp;
      }
      table[curPID].used[i] += size;
      break;
    }
  }
  // Si no se encuentra un espacio suficientemente grande, se retorna un error
  if(vpn < 0) return 1;
  
  // Marca las posiciones de memoria correspondientes como usadas y asigna el PID del proceso a esas posiciones
  for(int i = offset; i < (int)(offset + size); i++){
    table[curPID].mk[vpn][i] = 1;
    fromPID[table[curPID].pageFrame[vpn] * pagSz + i] = curPID;
  }
  // Establece la dirección y el tamaño del espacio reservado en el puntero de salida
  out->addr = to_addr(vpn, offset);
  out->size = size;

  // Calcula la dirección física real a partir del marco de página y el desplazamiento
  int pt = table[curPID].pageFrame[vpn];
  int realp = pt * pagSz + offset;

  // Retorna 0 para indicar que la reserva de memoria fue exitosa
  return 0;
}


// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr){
  m_pag_store(ptr.addr, 0);             // Almacena 0 en la dirección de memoria apuntada por ptr
  int vpn = 0, offset = 0;              // Declara las variables vpn y offset e inicialízalas con 0
  decode(ptr.addr, &vpn, &offset);      // Decodifica la dirección de memoria almacenada en ptr.addr para obtener vpn y offset
  
  // Recorre la tabla de páginas y libera las entradas correspondientes al puntero
  for(int i = offset; i < (int)(offset + ptr.size); i++){
    table[curPID].mk[vpn][i] = 0;       // Almacena 0 en la entrada correspondiente a vpn e i en la tabla de páginas del proceso actual
  }
  
  // Recorre la tabla de páginas para actualizar el valor de used[vpn]
  for(int i = table[curPID].used[vpn]; i >= 0; i--){
    if(table[curPID].mk[vpn][i]) break; // Si encuentra una entrada distinta de 0, termina el loop
    else table[curPID].used[vpn] = i;   // Actualiza el valor de used[vpn] con el valor de i actual
  }
  // Retorna 0 al finalizar la función
  return 0;
}


// Agrega un elemento al stack
int m_pag_push(unsigned char val, ptr_t *out){
  // Llama a la función m_pag_malloc para reservar espacio en memoria para el nuevo elemento
  int ret = m_pag_malloc(1, out);
  // Verifica si hubo un error al reservar memoria y devuelve 1 en ese caso
  if(ret) return 1;
  // Guarda el valor del nuevo elemento en la memoria reservada
  m_pag_store(out->addr, val);
  // Almacena la dirección del nuevo elemento en el stackPos del proceso actual
  stackPos[curPID][stackPoint[curPID]] = out->addr;
  // Incrementa el puntero de stackPoint del proceso actual
  stackPoint[curPID]++;
  // Devuelve 0 para indicar que la operación fue exitosa
  return 0;
}

  
// Quita un elemento del stack
int m_pag_pop(unsigned char *out){
  if(stackPoint[curPID] == 0) {
    return 1;  // Retorna 1 si la operacion falla
  }
  stackPoint[curPID]--;
  size_t addr = stackPos[curPID][stackPoint[curPID]];  // Obtiene la dirección del elemento que se sacará del stack
  return m_pag_load(addr, out);  // Llama a la función para cargar el valor en la dirección obtenida
}


// Función para cargar el valor de una dirección determinada
int m_pag_load(size_t addr, unsigned char *out){
  int pos = mem_pos(addr);  // Obtiene la posición de memoria correspondiente a la dirección dada
  if(pos < 0) {
    return 1;  // Retorna 1 si la posición de memoria no es válida
  }
  *out = pageMem[pos];  // Asigna el valor de la posición de memoria al puntero de salida
  return 0;  // Retorna 0 para indicar que se cargó el valor correctamente
}


// Función para almacenar un valor en una dirección determinada
int m_pag_store(size_t addr, unsigned char val){
  int pos = mem_pos(addr);  // Obtiene la posición de memoria correspondiente a la dirección dada
  if(pos < 0) {
    return 1;  // Retorna 1 si la posición de memoria no es válida
  }
  pageMem[pos] = val;  // Almacena el valor en la posición de memoria obtenida
  return 0;  // Retorna 0 para indicar que se almacenó el valor correctamente
}


// Función para notificar un cambio de contexto al proceso 'next_pid'
void m_pag_on_ctx_switch(process_t process){
  curPID = process.pid;  // Actualiza el ID del proceso actual con el ID del proceso dado en el cambio de contexto
}


// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process){
  int pid = process.pid;  // Obtiene el ID del proceso
  memAlloc[pid] = 0;  // Libera la asignación de memoria para el proceso
  pageMem[pid] = 0;  // Libera el espacio de la página para el proceso
  stackPoint[pid] = 0;  // Resetea el puntero de la pila para el proceso

  // Itera sobre las páginas usadas por el proceso
  for(int i = 0; i < maxPags; i++){
    if(table[pid].used[i]){  // Si la página está en uso
      // Itera sobre cada posición de la página y la libera
      for(int j = 0; j < pagSz; j++){
        fromPID[table[pid].pageFrame[i] * pagSz + j] = -1;
      }     
    }
  }

  // Reinicia el estado de la pila del proceso
  memset(stackPos[pid], 0, sizeof(stackPos[pid][0]) * stackSz);
  // Reinicia los pageFrames usados por el proceso
  memset(table[pid].pageFrame, 0, sizeof(table[pid].pageFrame[0]) * maxPags);
  // Reinicia el estado de uso de las páginas del proceso
  memset(table[pid].used, 0, sizeof(table[pid].used[0]) * maxPags);
  // Reinicia las marcas de página asociadas al proceso
  memset(table[pid].mk, 0, sizeof(table[pid].mk[0][0]) * maxPags * pagSz);
}
