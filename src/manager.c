#include "manager.h"

#include <stdlib.h>

#include "assert.h"
#include "managers/bnb_manager.h"
#include "managers/pag_manager.h"
#include "managers/seg_manager.h"
#include "memory.h"
#include "stdio.h"
#include "string.h"
#include "utils.h"

#define BNB_MANAGER 1
#define SEG_MANAGER 2
#define PAG_MANAGER 3

static int g_manager;

void m_init(int argc, char **argv) {
  if (strcmp(argv[1], "bnb") == 0) {
    g_manager = BNB_MANAGER;
    return m_bnb_init(argc, argv);
  } else if (strcmp(argv[1], "seg") == 0) {
    g_manager = SEG_MANAGER;
    m_seg_init(argc, argv);
  } else if (strcmp(argv[1], "pag") == 0) {
    g_manager = PAG_MANAGER;
    m_pag_init(argc, argv);
  } else {
    fprintf(stderr, "Invalid manager %s\n", argv[1]);
    exit(1);
  }
}

int m_malloc(size_t size, ptr_t *out) {
  if (g_manager == BNB_MANAGER) {
    return m_bnb_malloc(size, out);
  }
  if (g_manager == SEG_MANAGER) {
    return m_seg_malloc(size, out);
  }
  if (g_manager == PAG_MANAGER) {
    return m_pag_malloc(size, out);
  }
  return 1;
}

int m_free(ptr_t ptr) {
  if (g_manager == BNB_MANAGER) {
    return m_bnb_free(ptr);
  }
  if (g_manager == SEG_MANAGER) {
    return m_seg_free(ptr);
  }
  if (g_manager == PAG_MANAGER) {
    return m_pag_free(ptr);
  }
  return 1;
}

int m_push(byte val, ptr_t *out) {
  if (g_manager == BNB_MANAGER) {
    return m_bnb_push(val, out);
  }
  if (g_manager == SEG_MANAGER) {
    return m_seg_push(val, out);
  }
  if (g_manager == PAG_MANAGER) {
    return m_pag_push(val, out);
  }
  return 1;
}

int m_pop(byte *out) {
  if (g_manager == BNB_MANAGER) {
    return m_bnb_pop(out);
  }
  if (g_manager == SEG_MANAGER) {
    return m_seg_pop(out);
  }
  if (g_manager == PAG_MANAGER) {
    return m_pag_pop(out);
  }
  return 1;
}

int m_load(addr_t addr, byte *out) {
  if (g_manager == BNB_MANAGER) {
    return m_bnb_load(addr, out);
  }
  if (g_manager == SEG_MANAGER) {
    return m_seg_load(addr, out);
  }
  if (g_manager == PAG_MANAGER) {
    return m_pag_load(addr, out);
  }
  return 1;
}

int m_store(addr_t addr, byte val) {
  if (g_manager == BNB_MANAGER) {
    return m_bnb_store(addr, val);
  }
  if (g_manager == SEG_MANAGER) {
    return m_seg_store(addr, val);
  }
  if (g_manager == PAG_MANAGER) {
    return m_pag_store(addr, val);
  }
  return 1;
}

void m_on_ctx_switch(process_t process) {
  if (g_manager == BNB_MANAGER) {
    return m_bnb_on_ctx_switch(process);
  } else if (g_manager == SEG_MANAGER) {
    m_seg_on_ctx_switch(process);
  } else if (g_manager == PAG_MANAGER) {
    m_pag_on_ctx_switch(process);
  }
}

void m_on_end_process(process_t process) {
  if (g_manager == BNB_MANAGER) {
    return m_bnb_on_end_process(process);
  } else if (g_manager == SEG_MANAGER) {
    m_seg_on_end_process(process);
  } else if (g_manager == PAG_MANAGER) {
    m_pag_on_end_process(process);
  }
}
