#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "op.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint16_t u32;
typedef uint16_t u64;

typedef struct {
  union {
    struct {
      u8 C; u8 B;
      u8 E; u8 D;
      u8 L; u8 H;
      union {
        struct { u8 unused:4; u8 FC:1; u8 FH:1; u8 FN:1; u8 FZ:1;};
        u8 F;
      };
      u8 A;
      u16 SP;
      u16 PC;
    };
    u16 w[6];
  };
} regs;

typedef struct {
  regs r;
  u8 op;
} cpu;

#define A  (c->r.A)
#define B  (c->r.B)
#define C  (c->r.C)
#define D  (c->r.D)
#define E  (c->r.E)
#define F  (c->r.F)
#define H  (c->r.H)
#define L  (c->r.L)
#define fZ (c->r.FZ)
#define fH (c->r.FH)
#define fC (c->r.FC)
#define fN (c->r.FN)

#define BC (c->r.w[0])
#define DE (c->r.w[1])
#define HL (c->r.w[2])
#define AF (c->r.w[3])
#define SP (c->r.w[4])
#define PC (c->r.w[5])

u8* ptrs(cpu* c, u8 idx) {
    u8* _ptrs[8] = { &B, &C, &D, &E, &H, &L, 0, &A};
    return _ptrs[idx];
}

#define PTR_REG(x) ptrs(c, (x))

int fail=0;

#define MEMSIZE 16384

u8 mem[MEMSIZE];

/*
0000-1FFF 8K ROM
2000-23FF 1K RAM
// [2000] - [2100] -> ram
// [2300-2400] - Stack
2400-3FFF 7K Video RAM
4000- RAM mirror
*/

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


u8 r8(u16 a) { return mem[a]; }
u16 r16(u16 a) { return ((u16)(r8(a+1)) << 8) | (u16)(r8(a)); }
u8 f8(cpu *c) { u8 v = r8(PC++); return v; }
u16 f16(cpu *c) { u16 v = r16(PC); PC+=2; return v; }
void w8(u16 a, u8 v) { mem[a] = v; printf("W [%04x] <- %02x\n", a, v); }
void w16(u16 a, u16 v) { w8(a,v&0xff); w8(a+1,v>>8); }
void push16(cpu *c, u16 v) { SP -= 2; w16(SP, v); }
u16 pop16(cpu *c) { u16 v = r16(SP); SP+=2; return v; }

u8 inc8(cpu* c, u8 v) {
   u8 r = v + 1;
   fZ = (r == 0); fH = ((v & 0x0f) + 1 > 0x0f); fN = 0;
   return r;
}

u8 dec8(cpu* c, u8 v) {
    u8 r = v - 1;
    fZ = (r == 0); fH = ((v & 0x0f) == 0); fN = 1;
    return r;
}

// register - register load
void incdec(cpu *c) {
  u8 n = c->op & 0x3; //inc/dec
  u8 dst_idx = (c->op >> 3) & 0x7; //last 3 bits are reg#
  u8 src = dst_idx == 6 ? r8(HL) : (*PTR_REG(dst_idx));
  src = n ? dec8(c, src) : inc8(c, src);
  if (dst_idx != 6) *PTR_REG(dst_idx) = src; else w8(HL, src);
}

void ldrr(cpu *c) {
    u8 src_idx = c->op & 0x7; //last 3 bits are reg#
    u8 dst_idx = (c->op >> 3) & 0x7; //last 3 bits are reg#
    u8 src = (((c->op >> 6) & 0x3) == 0) ?
      f8(c) : src_idx == 6 ? r8(HL) : *(PTR_REG(src_idx));
    if (dst_idx != 6) *PTR_REG(dst_idx) = src; else w8(HL, src);
}

void _sub8(cpu *c, u8 v, u8 carry) {
  // use carry?
  u8 _c = carry ? fC : 0;
  u8 r = A;
  r = A - v - _c;
  // update flags
  fZ = (r == 0); fH = (((A & 0xf) < ((v & 0xf) + _c))) ? 1 : 0; fN = 1;
  fC = (((u16)(A) < (u16)(v) + (u16)(_c))) ? 1 : 0;
  A = r;
}

void sub8(cpu *c, u8 v) { _sub8(c, v, 0); }
void cp8 (cpu *c, u8 v)  { u8 r = A; _sub8(c, v, 0); A = r; }

// 8-bit alu
void alu(cpu *c) {
  u8 src_idx = c->op & 0x7; //last 3 bits are reg#
  u8 src = (((c->op >> 6) & 0x3) == 3) ? f8(c) : src_idx == 6 ? r8(HL) : *(PTR_REG(src_idx));
  u8 n = (c->op >> 3) & 0x07;

  switch (n) { // subgroup, bits xxNNNyyy
    case 0: /*add8(g, src);*/ break; // 00000yyy
    case 1: /*adc8(g, src);*/ break; // 00001yyy
    case 2: /*sub8(g, src);*/ break; // 00010yyy
    case 3: /*sbc8(g, src);*/ break; // 00011yyy
    case 4: /*and8(g, src);*/ break; // 00100yyy
    case 5: /*xor8(g, src);*/ break; // 00101yyy
    case 6: /*or8(g, src); */ break; // 00110yyy
    case 7: cp8(c, src);  break; // 00111yyy
  };
}

void unk(cpu *c)  { fail=1; puts("UNK"); };
void nop(cpu *c)  { };
void call(cpu *c) { push16(c, PC+2); PC=r16(PC); }
void ret(cpu *c)  { PC = pop16(c); }
void jmp(cpu *c)  { PC = f16(c);}
void jnz(cpu *c)  { u16 a = f16(c); PC = !fZ ? a : PC; } //JNZ a16
void ldsp(cpu *c) { SP = f16(c); }
void x11(cpu *c)  { DE = f16(c); }
void x21(cpu *c)  { HL = f16(c); }
void x1a(cpu *c)  { A = r16(DE); }
void x13(cpu *c)  { DE++; }
void x23(cpu *c)  { HL++; }

void *ops[256];

void ops_init() {
  for (u16 i=0; i<256; i++)
    ops[i] = &unk;
  ops[0x00]=&nop;
  ops[0x05]=&incdec;
  ops[0x06]=&ldrr;
  ops[0x0e]=&ldrr;
  ops[0x11]=&x11; /* BC := n16 */
  ops[0x1a]=&x1a; /* A := [DE] */
  ops[0x21]=&x21; /* HL := n16 */
  ops[0x13]=&x13; /* DE++ */
  ops[0x23]=&x23; /* HL++ */
  ops[0x16]=&ldrr;
  ops[0x1e]=&ldrr;
  ops[0x26]=&ldrr;
  ops[0x2e]=&ldrr;
  ops[0x31]=&ldsp;
  ops[0x36]=&ldrr;
  ops[0x3e]=&ldrr;
  for (u8 i=0x40; i<0x80; i++) ops[i]=&ldrr;
  ops[0xc2]=&jnz;
  ops[0xc3]=&jmp;
  ops[0xc9]=&ret;

  for (u8 i=0x80; i<0xc0; i++) ops[i] = &alu;
  ops[0xc6]=&alu; ops[0xce]=&alu;
  ops[0xd6]=&alu; ops[0xde]=&alu;
  ops[0xe6]=&alu; ops[0xee]=&alu;
  ops[0xf6]=&alu; ops[0xfe]=&alu;

  ops[0xcd]=&call;
}

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
