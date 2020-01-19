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

#define PTR_REG(x) ptrs(c, (x))
#define MEMSIZE 16384

extern void *ops[256];
extern void ops_init();
extern void alu(cpu *c);
extern void incdec(cpu *c);
extern void ldrr(cpu *c);
extern void reg_print(cpu *c);
extern void mem_load(u8 *at, char *fname);
extern void mem_clear(u8 *a, u16 len);
extern void mem_print(u8 *a, u16 len);
extern u8 r8(u16);
extern void w8(u16, u8 v);
extern u8 f8(cpu *c);
extern u8 mem[MEMSIZE];
extern u8 *ptrs(cpu* c, u8 idx);
extern const char *optxt[256];
extern int fail;
