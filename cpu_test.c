#include "defs.h"
#include <string.h>
#include <unistd.h>

/*
0000-1FFF 8K ROM
2000-23FF 1K RAM
// [2000] - [2100] -> ram
// [2300-2400] - Stack
2400-3FFF 7K Video RAM
4000- RAM mirror
*/

extern void emu(cpu *c, u8 *m);

void cpu_step(cpu *c) {

  c->op = mem[PC];
  PC++;
  ((void(*)(cpu*))ops[c->op])(c);
  c->cycl++;

}

void port_out_func(cpu *c, u8 v, u8 a) {
  //printf("port_out_func a=%u, v=%u\n", a, v);
  c->halt = 1;
}

u8 port_in_func(cpu *c, u8 port) {
  u8 op=C;
  u16 addr;
  switch (op) {
    case 2:
      printf("%c", E);
      break;
    case 9:
      addr = (D << 8) | E;
      do {
        printf("%c", mem[addr++]);
      } while ( mem[addr] != '$');
      break;
    default:
      break;
  }
  //printf("port_in_func op=%u, port=%u\n", C, port);
  return 0xff;
}

int main(int argc, char **argv) {

  char *romname =
    (argc > 1) ? argv[1] :
      "./cpu_tests/TST8080.COM";

  u32 cycles = (argc > 2) ?
    strtoul(argv[2], NULL, 10) : 0UL;

  u8 verbose = (argc > 3) ?
    strtoul(argv[3], NULL, 10) : 0;

  u8 slowmo = (argc > 4) ?
    strtoul(argv[4], NULL, 10) : 0;

  ops_init();

  u16 load_at = 0x100;

  cpu c = {0};
  c.r.w[5] = 0x100;

  memset(mem, 0, MEMSIZE);

  mem[0x0000] = 0xd3; /* out 1, a */
  mem[0x0001] = 0x00;
  mem[0x0005] = 0xdb; /* in a, 0 */
  mem[0x0006] = 0x00;
  mem[0x0007] = 0xc9;

  c.port_in = &port_in_func;
  c.port_out = &port_out_func;

  if (0 != mem_load(&mem[load_at], romname)) return -1;

  printf("reset done, running %s\n"
         "cycle limit = %u\n"
         "slowmo = %u, verbose = %u\n",
         romname, cycles, slowmo, verbose);

  u32 i=0;
  mem_print(&mem[0x100], 0x10);
  while (!(c.fail) && !(c.halt)) {
    if (slowmo) sleep(1);
    if (verbose) reg_print(&c);

    cpu_step(&c);

    if (cycles > 0 && c.cycl > cycles) {
      printf("max cycles reached = %u\n", cycles);
      return 1;
    }
  }

  //printf("terminated normally\n");
  return 0;
}
