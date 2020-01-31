#include "defs.h"

u8 mem[MEMSIZE];
u8 *mem_ptr = mem;

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

void mem_clear(u8 *a, u32 len) {

  for (u32 i=0; i<len; i++)
    a[i] = 0x65;

}

void mem_print(u8 *a, u32 len) {

  for (u16 i=0; i<len; i++) {
    if (i % 8 == 0) printf("[0x%04x] ", i);
    printf("0x%02x%s", a[i],
      i % 8 != 7 ? " " : "\n");
  }
}

void mem_1bpp(u8 *a, u16 w, u16 h) {

  u16 lines=w;
  u16 cols=h/8;

  for (u16 l=0; l<lines; l++) {
  for (u16 c=0; c<cols; c++) {
    u8 v = a[cols*l+c];
    //printf("%04x, ", cols*l+c + 0x2400);
    //printf("%c%c%c%c%c%c%c%c",
    //(v>>0 & 0x1) ?'*':'.',
    //(v>>1 & 0x1) ?'*':'.',
    //(v>>2 & 0x1) ?'*':'.',
    //(v>>3 & 0x1) ?'*':'.',
    //(v>>4 & 0x1) ?'*':'.',
    //(v>>5 & 0x1) ?'*':'.',
    //(v>>6 & 0x1) ?'*':'.',
    //(v>>7 & 0x1) ?'*':'.');
  }
  //printf("\n");
  }
}

void reg_print(cpu *c) {
  printf("%04x: [BC] [%02x%02x] [DE] [%02x%02x] "
         "[HL] [%02x%02x] [AF] [%02x%02x %c%c%c%c%c%c%c%c] "
         "[SP] [%04x] #[%08llu] OP=%02x : %-8s\n",
           PC, B,  C,  D,  E,
           H,  L,  A,  F,
           fS ? 'S' : '-',
           fZ ? 'Z' : '-',
           '0',
           fH ? 'H' : '-',
           '0',
           fP ? 'P' : '-',
           '1',
           fC ? 'C' : '-',
           SP, c->cycl, c->op, optxt[c->op]);
}
