#include "defs.h"

int fail=0;

u8 mem[MEMSIZE];

/*
0000-1FFF 8K ROM
2000-23FF 1K RAM
// [2000] - [2100] -> ram
// [2300-2400] - Stack
2400-3FFF 7K Video RAM
4000- RAM mirror
*/

void cpu_step(cpu *c) {

  c->op = mem[PC];
  reg_print(c);
  PC++;
  ((void(*)(cpu*))ops[c->op])(c);

}

int main(int argc, char **argv) {

  char *romname =
    (argc > 1) ? argv[1] :
      "invaders.rom";

  u32 cycles = (argc > 2) ?
    strtoul(argv[2], NULL, 10) : 0ULL;

  u16 load_at = 0;

  mem_clear(mem, MEMSIZE);

  mem_load(&mem[load_at], romname);
  mem_print(mem, 32);

  cpu c;
  mem_clear((u8*)&c, sizeof(cpu));

  ops_init();

  u32 i=0;
  while ((i < cycles || cycles==0) && (fail==0)) {
    printf("[%5u], ", i++);
    cpu_step(&c);
  }

  return 0;
}
