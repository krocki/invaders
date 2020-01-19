#include "defs.h"

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

void reg_print(cpu *c) {
  printf("[BC] [%02x%02x] [DE] [%02x%02x] "
         "[HL] [%02x%02x] [AF] [%02x%02x %c%c%c%c] "
         "[SP] [%04x] [PC] [%04x] OP=%02x : %-8s\n",
           B,  C,  D,  E,
           H,  L,  A,  F,
           fZ ? 'Z' : '-',
           fN ? 'N' : '-',
           fH ? 'H' : '-',
           fC ? 'C' : '-',
           SP, PC, c->op, optxt[c->op]);
}
