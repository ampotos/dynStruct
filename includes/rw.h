#ifndef RW_H_
#define RW_H_

typedef struct
{
  void *addr;
  void *dr_addr;
} ctx_t;

void memory_read(void *pc, void *next_pc);
void memory_write(void *pc, void *prev_pc);

#endif
