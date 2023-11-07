#include "tests.h"

#include "mem_sim.h"
#include "memory.h"
#include "utils.h"
#include <stdio.h>

static int run_argc;
static char **run_argv;

static size_t curr_pid = 0;

#define MAX_PROGRAM_COUNT 20
#define PROCESS_FROM(idx) new_process(curr_pid++, &g_programs[idx])

static program_t g_programs[MAX_PROGRAM_COUNT];

static void init_programs()
{
  g_programs[0] = new_program("p_0", 235);
  g_programs[1] = new_program("p_1", 235);
  g_programs[2] = new_program("p_2", 237);
  g_programs[3] = new_program("p_3", 238);
}

static void setup_test_case(const size_t mem_size, const char *log_name)
{
  curr_pid = 0;
  char log_path[50];
  if (snprintf(log_path, 50, "./mem_logs/%s_%s.log", run_argv[1], log_name) < 0)
  {
    fprintf(stderr, "Error building log file name\n");
    exit(1);
  }
  setup_sim(mem_size, log_path, run_argc, run_argv);
}

// ======================================================================
// TEST CASE 1
// ======================================================================

void test_case_001()
{
  setup_test_case(KB_SIZE(8), "case_001");
  process_t processes[] = {
      PROCESS_FROM(0),
      PROCESS_FROM(1),
  };

  ctx_switch(processes[0]);    // Cambia de contexto al proceso 0
  ptr_t p0_x = mem_malloc(4);  // Reserva memoria para 4 bytes
  mem_store(at(p0_x), 10);     // Guarda el valor 10 en la posición 0
  mem_store(at(p0_x) + 1, 20); // Guarda el valor 20 en la posición 1
  mem_push(70);                // Añade al stack el valor 70

  ctx_switch(processes[1]);    // Cambia de contexto al proceso 1
  ptr_t p1_x = mem_malloc(2);  // Reserva 2 bytes de memoria
  mem_store(at(p1_x), 30);     // Guarda el valor 30 en la posición 0
  mem_store(at(p1_x) + 1, 40); // Guarda el valor 40 en la posición 1
  mem_push(80);                // Añade al stack el valor 80

  ctx_switch(processes[0]);          // Cambia de contexto al proceso 0
  mem_load_assert(at(p0_x), 10);     // Comprueba que el valor en p0_x == 10
  mem_load_assert(at(p0_x) + 1, 20); // Comprueba que el valor p0_x + 1 == 20
  mem_pop_assert(70);                // Comprueba que el ultimo valor de la pila es 70

  ctx_switch(processes[1]);          // Cambia de contexto al proceso 1
  mem_load_assert(at(p1_x), 30);     // Comprueba que el valor en p1_x == 30
  mem_load_assert(at(p1_x) + 1, 40); // Comprueba que el valor en p1_x == 40
  mem_pop_assert(80);                // Comprueba que el ultimo valor de la pila es 80

  end_process(processes[0]);
  end_process(processes[1]);
  end_sim(); // Termina la simulación
}

// ======================================================================
// TEST CASE 2
// ======================================================================

void test_case_002()
{
  setup_test_case(KB_SIZE(8), "case_002");
  process_t processes[] = {
      PROCESS_FROM(0),
      PROCESS_FROM(1),
      PROCESS_FROM(2),
  };

  ctx_switch(processes[0]);
  ptr_t p0_x = mem_malloc(6);
  mem_store(at(p0_x), 10);
  mem_store(at(p0_x) + 1, 20);
  mem_store(at(p0_x) + 2, 30);
  mem_push(70);

  ctx_switch(processes[1]);
  ptr_t p1_x = mem_malloc(4);
  mem_store(at(p1_x), 40);
  mem_store(at(p1_x) + 1, 50);
  mem_store(at(p1_x) + 2, 60);
  mem_push(80);

  ctx_switch(processes[2]);
  ptr_t p2_x = mem_malloc(8);
  mem_store(at(p2_x), 70);
  mem_store(at(p2_x) + 1, 80);
  mem_store(at(p2_x) + 2, 90);
  mem_push(90);

  ctx_switch(processes[0]);
  mem_load_assert(at(p0_x), 10);
  mem_load_assert(at(p0_x) + 1, 20);
  mem_load_assert(at(p0_x) + 2, 30);
  mem_pop_assert(70);

  ctx_switch(processes[1]);
  mem_load_assert(at(p1_x), 40);
  mem_load_assert(at(p1_x) + 1, 50);
  mem_load_assert(at(p1_x) + 2, 60);
  mem_pop_assert(80);

  ctx_switch(processes[2]);
  mem_load_assert(at(p2_x), 70);
  mem_load_assert(at(p2_x) + 1, 80);
  mem_load_assert(at(p2_x) + 2, 90);
  mem_pop_assert(90);

  end_process(processes[0]);
  end_process(processes[1]);
  end_process(processes[2]);
  end_sim();
}

// ======================================================================

void test_case_003()
{
  setup_test_case(KB_SIZE(4), "case_003");
  process_t processes[] = {
      PROCESS_FROM(0),
      PROCESS_FROM(1),
      PROCESS_FROM(2),
      PROCESS_FROM(3),
  };

  ctx_switch(processes[0]);
  ptr_t p0_x = mem_malloc(6);
  mem_store(at(p0_x), 10);
  mem_store(at(p0_x) + 1, 20);
  mem_store(at(p0_x) + 2, 30);
  mem_push(70);
  mem_push(90);

  ctx_switch(processes[1]);
  ptr_t p1_x = mem_malloc(8);
  mem_store(at(p1_x), 40);
  mem_store(at(p1_x) + 1, 50);
  mem_store(at(p1_x) + 2, 60);
  mem_push(80);
  mem_push(100);

  ctx_switch(processes[2]);
  ptr_t p2_x = mem_malloc(4);
  mem_store(at(p2_x), 70);
  mem_store(at(p2_x) + 1, 80);
  mem_push(110);
  mem_push(130);

  ctx_switch(processes[3]);
  ptr_t p3_x = mem_malloc(10);
  mem_store(at(p3_x), 90);
  mem_store(at(p3_x) + 1, 100);
  mem_store(at(p3_x) + 2, 110);
  mem_push(120);
  mem_push(140);

  ctx_switch(processes[0]);
  mem_load_assert(at(p0_x), 10);
  mem_load_assert(at(p0_x) + 1, 20);
  mem_load_assert(at(p0_x) + 2, 30);
  mem_pop_assert(90);
  mem_pop_assert(70);

  ctx_switch(processes[1]);
  mem_load_assert(at(p1_x), 40);
  mem_load_assert(at(p1_x) + 1, 50);
  mem_load_assert(at(p1_x) + 2, 60);
  mem_pop_assert(100);
  mem_pop_assert(80);

  ctx_switch(processes[2]);
  mem_load_assert(at(p2_x), 70);
  mem_load_assert(at(p2_x) + 1, 80);
  mem_pop_assert(130);
  mem_pop_assert(110);

  ctx_switch(processes[3]);
  mem_load_assert(at(p3_x), 90);
  mem_load_assert(at(p3_x) + 1, 100);
  mem_load_assert(at(p3_x) + 2, 110);
  mem_pop_assert(140);
  mem_pop_assert(120);

  end_process(processes[0]);
  end_process(processes[1]);
  end_process(processes[2]);
  end_process(processes[3]);
  end_sim();
}

///
///   CUSTOM CASES
///
void test_case_p_stress_mem()
{
  setup_test_case(KB_SIZE(8), "case_p_stress");
  process_t processes[] = {
      PROCESS_FROM(0),
  };

  ctx_switch(processes[0]);
  ptr_t p0_x = mem_malloc(50);
  mem_push(100);
  ptr_t p1_x = mem_malloc(500);

  end_process(processes[0]);
  end_sim();
}

void test_case_p_free_mem()
{
  setup_test_case(KB_SIZE(8), "case_p_free");
  process_t processes[] = {
      PROCESS_FROM(0),
  };

  ctx_switch(processes[0]);
  ptr_t p0_x = mem_malloc(50);
  mem_push(100);
  ptr_t p1_x = mem_malloc(500);
  mem_free(p1_x);
  ptr_t p2_x = mem_malloc(500);

  end_process(processes[0]);
  end_sim();
}

void test_case_p_complex_procs()
{
  setup_test_case(KB_SIZE(8), "case_p_complex_procs");
  process_t processes[] = {
      PROCESS_FROM(0),
      PROCESS_FROM(1),
  };

  ctx_switch(processes[0]);
  ptr_t p0_x = mem_malloc(50);
  mem_push(100);
  ptr_t p1_x = mem_malloc(500);
  mem_free(p1_x);
  ptr_t p2_x = mem_malloc(500);

  end_process(processes[0]);

  ctx_switch(processes[1]);
  ptr_t p01_x = mem_malloc(50);
  mem_push(100);
  ptr_t p11_x = mem_malloc(500);
  mem_free(p1_x);
  ptr_t p21_x = mem_malloc(500);

  end_process(processes[1]);
  end_sim();
}

void test_case_p_freelist()
{
  setup_test_case(KB_SIZE(8), "case_p_freelist");
  process_t processes[] = {
      PROCESS_FROM(0),
  };

  // Round 1
  ctx_switch(processes[0]);
  ptr_t p01_x = mem_malloc(50);
  ptr_t p02_x = mem_malloc(30);
  ptr_t p03_x = mem_malloc(60);
  ptr_t p04_x = mem_malloc(30);
  m_free(p02_x);
  m_free(p04_x);
  m_free(p03_x);
  m_free(p01_x);

  // Round 2
  ptr_t p05_x = mem_malloc(1000);
  ptr_t p06_x = mem_malloc(10);
  m_free(p05_x);
  m_free(p06_x);

  end_process(processes[0]);
  end_sim();
}

void test_case_p_zahdehyv()
{
  setup_test_case(KB_SIZE(8), "case_p_zahdehyv");
  process_t processes[] = {
      PROCESS_FROM(0),
      PROCESS_FROM(1),
  };

  ctx_switch(processes[0]);
  ptr_t p0_x = mem_malloc(50);
  mem_push(100);
  ptr_t p1_x = mem_malloc(500);
  mem_free(p1_x);

  ctx_switch(processes[1]);
  ptr_t p01_x = mem_malloc(50);
  mem_push(100);
  ptr_t p11_x = mem_malloc(500);
  mem_free(p1_x);
  ptr_t p21_x = mem_malloc(500);

  ctx_switch(processes[0]);
  ptr_t pt_x = mem_malloc(500);
  mem_free(pt_x);
  ptr_t p5_x = mem_malloc(300);
  ptr_t p6_x = mem_malloc(300);
    ctx_switch(processes[1]);
  ptr_t p7_x = mem_malloc(300);
  ptr_t p8_x = mem_malloc(300);
  mem_free(p7_x);
 ctx_switch(processes[0]);
  ptr_t pst_x = mem_malloc(500);
ptr_t pst_dx = mem_malloc(500);
ptr_t psddt_x = mem_malloc(500);
ptr_t psddtd_x = mem_malloc(500);
ptr_t psddtddd_x = mem_malloc(500);

  end_process(processes[0]);
  end_process(processes[1]);

  end_sim();
}

///
///   RUN
///
void run_tests(int argc, char **argv)
{
  init_programs();
  run_argc = argc;
  run_argv = argv;
  test_case_001();
  test_case_002();
  test_case_003();
  //ESTOS CASOS HACEN MALLOC A UN SIZE MAYOR Q EL TAMANO DE BLOQUE+CODE, POR ESO PARTEN BNB
  //test_case_p_stress_mem();
  //test_case_p_free_mem();
  //test_case_p_complex_procs();
  //test_case_p_freelist(); // Este es para el pagination solamente (que los otros mueran es normal)
  //test_case_p_zahdehyv();
}
