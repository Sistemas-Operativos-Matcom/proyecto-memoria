#include "structs.h"

//-----------------------------------------------------NODOS----------------------------------------------------

// Conectar nodos en una lista
static void Connect(Node *node, Node *next, Node *prev, List *list)
{
    // Conectando nodo a su siguiente
    node->next = next;
    // Si el nodo siguiente es nulo, el nodo insertado es el último de la lista. De lo contrario, conectar ambos nodos.
    if (next != NULL)
    {
        next->prev = node;
    }
    else
    {
        list->tail = node;
    }
    // Conectando nodo a su anterior
    node->prev = prev;
    // Si el nodo anterior es nulo, el nodo insertado es el primero de la lista. De lo contrario, conectar ambos nodos.
    if (prev != NULL)
    {
        prev->next = node;
    }
    else
    {
        list->head = node;
    }
}

//----------------------------------------------------LISTAS-----------------------------------------------------

// Inicializar una lista
void List_Init(List *list, size_t total_memory)
{
    // Reservando espacio en memoria para los elementos de la lista
    list->size = 1;
    list->head = list->tail = (Node *)malloc(sizeof(Node));
    // Asignando valores iniciales de la lista
    list->head->next = list->head->prev = NULL;
    list->head->pos = 0;
    list->head->size = total_memory;
    list->max_pos = total_memory;
}

// Limpiar una lista (Reset)
void List_RS(List *list, size_t total_memory)
{
    if (list->size > 0)
    {
        // Vaciando la lista y liberando espacio
        Node *node = list->head;
        while (node != NULL)
        {
            Node *next_node = node->next;
            free(node);
            node = next_node;
        }
    }
    // Creando una nueva lista del mismo tamaño
    List_Init(list, total_memory);
}

// Obtener el primer fragmento de memoria con suficiente espacio disponible
int Get_Mem(List *list, size_t size, size_t *addr)
{
    // Si la lista está vacía, devuelve error
    if (list->size <= 0)
    {
        return Error;
    }
    // De lo contrario, declara un estado inicial activo y analiza los nodos
    int status = Error;
    // Mientras status==Error, Get_Mem no habrá terminado con éxito

    // Iterando por todos los nodos de la lista
    Node *node = list->head;
    while (node != NULL)
    {
        // Verificando si hay suficiente espacio en el nodo
        if (node->size < size)
        {
            // Si el nodo no tiene espacio, saltar al siguiente
            node = node->next;
        }
        else
        {
            // Guardando dirección del nodo en memoria
            *addr = node->pos;
            // Guardando nodos conectados
            Node *previous = node->prev;
            Node *next = node->next;
            // Declarando nuevo tamaño del nodo
            size_t new_size = node->size - size;
            int new_pos = node->pos + size;
            // Si el nodo aún tiene espacio vacío, crea dos nodos a partir de él, uno lleno y otro vacío
            if (new_size != 0)
            {
                // Reservando el espacio en memoria para el nuevo nodo
                Node *new = (Node *)malloc(sizeof(Node));
                // Conectando el nuevo nodo
                Connect(new, next, previous, list);
                // Asignando valores
                new->pos = new_pos;
                new->size = new_size;
                // Actualizando el tamaño de la lista
                list->size++;
            }
            // Si el nodo no tiene más espacio libre, conectar sus nodos antecesor y sucesor entre ellos
            else
            {
                if (previous != NULL)
                {
                    previous->next = next;
                }
                if (next != NULL)
                {
                    next->prev = previous;
                }
                // Actualizar el tamaño de la lista
                list->size--;
            }
            // Terminando ciclo forzadamente
            free(node);
            status = Ok;
            break;
        }
    }
    // Caso en que la lista se queda vacía
    if (list->size == 0)
    {
        list->head = list->tail = NULL;
    }
    return status;
}

int Free_Mem(List *list, size_t size, size_t addr)
{
    // La lista es muy pequeña en comparación con el tamaño solicitado
    if (size > list->size)
    {
        return Error;
    }
    // Si la lista esta vacía significa que se reservo toda la
    // memoria, por tanto libera el fragmento de memoria en addr
    if (list->size == 0)
    {
        Node *new = (Node *)malloc(sizeof(Node));
        new->next = new->prev = NULL;
        new->pos = addr;
        new->size = size;
        return Ok;
    }

    int status = Error;
    Node *node = list->head;

    while (node != NULL)
    {
        size_t next_pos;
        if (node->next != NULL)
        {
            next_pos = (size_t)node->next->pos;
        }
        else
        {
            next_pos = list->max_pos;
        }

        // Buscar el espacio a liberar
        if (node->pos <= addr && addr <= next_pos)
        {
            // Creando nuevo nodo
            Node *new = (Node *)malloc(sizeof(Node));
            new->pos = addr;
            new->size = size;
            Node *previous = node->prev;
            Node *next = node->next;
            // Actualizando el tamaño de la lista
            list->size++;

            // En caso que el nodo anterior colisione con el actual, combinarlos a ambos
            if (previous != NULL && (size_t)previous->pos + previous->size == new->pos)
            {
                Node *previous_prev = previous->prev;

                // Actualizar el nodo
                new->pos = previous->pos;
                new->size += previous->size;
                free(previous);
                previous = previous_prev;

                // Actualizar el tamaño de la lista
                list->size--;
            }

            // En caso que el nodo siguiente colisione con el actual, combinarlos a ambos
            if (next != NULL && (size_t) new->pos + new->size == next->pos)
            {
                Node *next_next = next->next;

                // Actualizar el nodo actual (no es necesario actualizarlo ya que la pos del actual es menor que la del next)
                new->size += next->size;
                free(next);
                next = next_next;

                // Actualizar el tamaño de la lista
                list->size--;
            }

            // Conectar el nuevo nodo
            Connect(new, next, previous, list);
            status = Ok;
            break;
        }
        // Si no hay espacio a liberar, saltar de nodo
        node = node->next;
    }
    // Caso en que la lista se queda vacía
    if (list->size == 0)
    {
        list->head = list->tail = NULL;
    }
    return status;
}