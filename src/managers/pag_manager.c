#include "pag_manager.h"
#include "utils_managers/list_pag.h"
#include "../utils.h"
#include "stdio.h"
#include <math.h>

static int frames_amount;//size de used_frames
static int *used_frames;
static size_t pages_size_pow = 8;
static size_t pages_size;
static list *procs_list;
static process_pag *curr_process;

// Esta función se llama cuando se inicializa un caso de prueba
void m_pag_init(int argc, char **argv)
{
    pages_size = pow(2,pages_size_pow);
    size_t mem_size = m_size();
    procs_list = Init_list_of_pages();
    frames_amount = mem_size / pages_size;
    used_frames = (size_t *)malloc(frames_amount * sizeof(size_t));
}
// Libera un espacio de memoria dado un puntero.
int m_pag_free(ptr_t ptr)
{
  int a = 1;
   size_t pag = ptr.addr >> pages_size_pow;//
    int pag_last = pages_size * pag;
   int proc_page_pos = ptr.addr - pag_last;
  if (ptr.size > pages_size)
  {
    size_t space_to_free = ptr.size;
    while (a)
    {
      used_frames[curr_process->process_pages[pag]] = 0;//libero la pagina cuya direccion corresponde
      m_unset_owner(curr_process->process_pages[pag] * pages_size, (curr_process->process_pages[pag] + 1) * pages_size);
      curr_process->process_pages[pag] = 0;
      
      if (space_to_free >pages_size)
      {
        Reserve_or_free_space_in_page(pages_size, pag, 0);//liberamos toda la pagina
        space_to_free -= pages_size;
      }
      else
      {
        Reserve_or_free_space_in_page(space_to_free, pag, 0);//liberamos el espacio correspondiente en la pagina
        space_to_free = 0;
        return 0;
      }
      pag ++;
    }
  }
  else
  {
    for (size_t i = 0; i < ptr.size; i++)
    {
      if (proc_page_pos >= pages_size)
      {
        proc_page_pos = 0;
        pag++;
      }
      size_t mem_addr = (curr_process->process_pages[pag] * pages_size) + proc_page_pos + i;
      curr_process->pages_free_space[pag][proc_page_pos + i] = 0;
      m_write(mem_addr, 0);
    }return 0;
  }
}

// Reserva un espacio en el heap de tamaño 'size' y establece un puntero al
// inicio del espacio reservado.
int m_pag_malloc(size_t size, ptr_t *out)
{
  if (size <= pages_size)
  {
    int result = 1;
    for (size_t i = 0; i < curr_process->proc_pages_amount; i++)
    {
      for (size_t j = 0; j < pages_size; j++)
      {
        if (curr_process->pages_free_space[i][j] != 1)//itero por cada espacio de las paginas del proceso y si esta vacio reviso si hay espacio consecutivo igual a size se lo asigno y retorno 0
        {
          size_t k = 0;
          for (; k < size && ((j + k) < pages_size); k++)
          {
            if (curr_process->pages_free_space[i][j + k] == 1)
              break;
          }
          if (k == size)//quiere decir que hay espacio en pag i desde j hasta j+size
          {
            out->addr = (i * pages_size) + j;
            out->size = size;
             
            for (size_t k = 0; k < size; k++)//marco como reservado el espacio entre j y j+size
              curr_process->pages_free_space[i][j + k] = 1;
              printf("Successfully allocated");
            return 0;
          }
          else
            j += k;
        }
      }
    }
    result = Assign_New_Pages(size);
    printf("%d", result);
    return result;
  }else
  {
    size_t space_needed = size;
    out->addr = (curr_process->proc_pages_amount) * pages_size;
    out->size = size;
    return Assign_New_Pages(space_needed);
  }
}

int Assign_New_Pages(int space_needed)//retorna 0 si pudo reservar el space needed y 1 en caso contrario
{
  for (size_t i = 0; i < frames_amount; i++)//iterando por frames
    {
      if (used_frames[i] != 1)//si ese frame no pertenece a ningun proceso,agrego esa pag a este proceso
      {
        used_frames[i] = 1;
        //curr_process->proc_pages_amount += 1;
        curr_process->process_pages[curr_process->proc_pages_amount++] = i;
        m_set_owner(i * pages_size, (i + 1) * pages_size);//pagina i empieza en de i*pages_size y ocupa pages_size, se le asigna owner = curr_proc
        if (space_needed > pages_size)//si el espacio que me piden es mayor que el de una pagina hay que asignar al menos otra
        {
          space_needed -= pages_size;
          Reserve_or_free_space_in_page(pages_size, curr_process->proc_pages_amount - 1, 1);
          curr_process->proc_pages_amount++;
        }
        else
        {
          Reserve_or_free_space_in_page(pages_size, curr_process->proc_pages_amount - 1, 1);
          space_needed = 0;
          return 0;
        }
      }
    }
    fprintf(stderr, "La memoria está llena.");
    return 1;
}

void Reserve_or_free_space_in_page(int space, int page, int reserved_or_not)
{ 
  for (size_t k = 0; k < space; k++)//declaro toda la pag que acabo de add como reservada y aumento en 1 la cant de pags para continuar buscando otra vacia
   {
    curr_process->pages_free_space[page][k] = reserved_or_not;
   }
}

int m_pag_push(byte val, ptr_t *out)
{
  for (size_t i = 0; i < curr_process->proc_pages_amount; i++)
  {
    for (size_t j = 0; j < pages_size; j++)
    {
      if (curr_process->pages_free_space[i][j] != 1)
      {
        curr_process->pages_free_space[i][j] = 1;
        size_t mem_addr = (curr_process->process_pages[i] * pages_size) + j;
        out->addr = (i * pages_size) + j;
        out->size = 1;
        m_write(mem_addr, val);
        Push_stack(curr_process->my_stack, out->addr);
        return 0;
      }
    }
  }
  curr_process->proc_pages_amount += 1;
  if (curr_process->proc_pages_amount > frames_amount)
  {
    fprintf(stderr, "No queda espacio en memoria.");
    return 1;
  }
  curr_process->pages_free_space[curr_process->proc_pages_amount - 1][0] = 1;
  size_t mem_addr = (curr_process->process_pages[curr_process->proc_pages_amount - 1] * pages_size);
  out->addr = (curr_process->proc_pages_amount - 1) * pages_size;
  out->size = 1;
  m_write(out->addr, val);
  Push_stack(curr_process->my_stack, out->addr);

  return 0;
}
// Quita un elemento del stack
int m_pag_pop(byte *out)
{
  size_t addr = Pop_stack(curr_process->my_stack);
  size_t pag = (addr >> pages_size_pow);
  *out = m_read((curr_process->process_pages[pag]) * pages_size + (addr - (pages_size * pag)));
  curr_process->pages_free_space[pag]--;
  return 0;
}

// Carga el valor en una dirección determinada
int m_pag_load(addr_t addr, byte *out)
{
  // if(addr > (curr_process->proc_pages_amount*pages_size)-1)
  // {
  //   return 1;
  // }
  size_t pag = addr >> pages_size_pow;
  size_t proc_page_pos = addr - (pages_size * pag);
  size_t mem_addr = (curr_process->process_pages[pag] * pages_size) + proc_page_pos;
  *out = m_read(mem_addr);
  return 0;
}

int m_pag_store(addr_t addr, byte val)
{
  printf("%d\n", curr_process->proc_pages_amount);
  printf("%d\n", addr);
  printf("%d\n", curr_process->code_size -1);
  fflush(NULL);
  if(addr > ((curr_process->proc_pages_amount*pages_size)-1))
  {
    return 1;
  }
  size_t pag = addr >> pages_size_pow;
  size_t proc_page_pos = addr - (pages_size * pag);
  size_t mem_addr = (curr_process->process_pages[pag] * pages_size) + proc_page_pos;
  m_write(mem_addr, val);
  return 0;
}

void m_pag_on_ctx_switch(process_t process)
{
  int pos = pag_Contains(procs_list, process.pid);//reviso si el proceso es nuevo
  if (pos != -1)//ya tiene paginas asignadas
  {
    *curr_process = procs_list->data[pos];
    set_curr_owner(process.pid);
  }
  else //hay que inicializarlo
  {
    size_t code_size = process.program->size;
    size_t code_pages = code_size / pages_size;
    if (code_size % pages_size != 0 || code_pages == 0)
      code_pages++;    //importante, un poceso al parecer puede tener code_size = 0
    curr_process = Init_process_pag(process.pid, pages_size, frames_amount, code_size);
    set_curr_owner(process.pid);
    size_t page_counter = 0;
    
    for (size_t i = 0; i < frames_amount; i++)
    {
      if (used_frames[i] != 1)
      {
        m_set_owner(i * pages_size, (i + 1) * pages_size);
        used_frames[i] = 1;
        if (code_size >= pages_size)
        {
          code_size -= pages_size;
          Reserve_or_free_space_in_page(pages_size,page_counter,1);
        }
        else
        {
          Reserve_or_free_space_in_page(code_size,page_counter,1);
        }
        curr_process->process_pages[page_counter] = i;
        curr_process->proc_pages_amount++;
        page_counter++;
        if (page_counter == code_pages)
          break;
      }
    }
    if (page_counter != code_pages)
    {
      fprintf(stderr, "No queda espacio en mem");
      exit(1);
    }
    pag_Push(procs_list, *curr_process);
  }
}

// Notifica que un proceso ya terminó su ejecución
void m_pag_on_end_process(process_t process)
{
  set_curr_owner(process.pid);
  int pos = pag_Contains(procs_list, process.pid);
  if (pos != -1)
  {
    process_pag *p = &procs_list->data[pos];
    for (size_t i = 0; i < p->proc_pages_amount; i++)
    {
      used_frames[p->process_pages[i]] = 0;
      m_unset_owner(p->process_pages[i] * pages_size, (p->process_pages[i] + 1) * pages_size);
    }
    Free_p_pag(p);
    pag_RemovePos(procs_list, pos);
  }
}
