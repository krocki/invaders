#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint16_t u32;
typedef uint16_t u64;

typedef struct {
  u8 C; u8 B;
  u8 E; u8 D;
  u8 L; u8 H;
  u8 F; u8 A;
  u16 SP;
  u16 PC;
} regs;

#define RAM_S 2048
#define SCR_W 224
#define SCR_H 256

u8 ram[RAM_S];
u8 vram[SCR_H * SCR_W/8];

void mem_load(u8 *at, char *fname) {

  FILE *f = fopen(fname, "rb");
  if (!f) {
    fprintf(stderr,
      "couldn't open %s\n", fname);
    return;
  }

  fseek(f, 0, SEEK_END);
  u16 len = ftell(f);
  rewind(f);

  u16 n = fread(at, 1, len, f);

  printf("%s : %u/%u bytes read\n",
    fname, n, len);

  fclose(f);
}

void mem_clear(u8 *a, u16 len) {

  for (u16 i=0; i<len; i++)
    a[i] = 0;

}

void mem_print(u8 *a, u16 len) {

  for (u16 i=0; i<len; i++) {
    if (i % 8 == 0) printf("[0x%04x] ", i);
    printf("0x%02x%s", a[i],
      i % 8 != 7 ? " " : "\n");
  }
}

void reg_print(regs *r) {
  printf("[BC] [%02x %02x] [DE] [%02x %02x]\n"
         "[HL] [%02x %02x] [AF] [%02x %02x]\n"
         "[SP] [ %04x] [PC] [ %04x]\n",
           r->B,  r->C,  r->D,  r->E,
           r->H,  r->L,  r->A,  r->F,
           r->SP, r->PC);
}

void cpu_step(regs *c) {

  reg_print(c);
  u8 byte = ram[c->PC];
  printf("fetched byte at [%04x] -> [%02x]\n",
    c->PC, byte);
  c->PC++;
}

int main(int argc, char **argv) {

  char *romname =
    (argc > 1) ? argv[1] :
      "invaders.rom";

  u32 cycles = 3;
  //u32 cycles = (argc > 2) ?
  //  strtoul(argv[2], NULL, 10) : 0ULL;

  u16 load_at = 0;

  mem_clear(ram, RAM_S);
  mem_clear(vram, SCR_H * SCR_W/8);

  mem_load(&ram[load_at], romname);
  mem_print(ram, 32);

  regs cpu;
  mem_clear((u8*)&cpu, sizeof(cpu));

  u32 i=0;
  while (i < cycles || cycles == 0) {
    printf("cycle %u\n", i++);
    cpu_step(&cpu);
  }

  return 0;
}
