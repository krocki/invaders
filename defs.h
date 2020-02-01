#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef struct {
  union {
    struct {
      u8 C; u8 B;
      u8 E; u8 D;
      u8 L; u8 H;
      union {
        struct { u8 FC:1; u8 F1:1;
                 u8 FP:1; u8 F3:1;
                 u8 FH:1; u8 F5:1;
                 u8 FZ:1; u8 FS:1;};
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
  u8 fail; /* trap */
  u8 halt;
  u8 ei; /* enable interrupts */
  regs r;
  u8 op;
  u64 cycl;
  void *port_in;
  void *port_out;
} cpu;

#define A  (c->r.A)
#define B  (c->r.B)
#define C  (c->r.C)
#define D  (c->r.D)
#define E  (c->r.E)
#define F  (c->r.F)
#define H  (c->r.H)
#define L  (c->r.L)
#define fS (c->r.FS)
#define fZ (c->r.FZ)
#define fH (c->r.FH)
#define fP (c->r.FP)
#define fC (c->r.FC)

#define BC (c->r.w[0])
#define DE (c->r.w[1])
#define HL (c->r.w[2])
#define AF (c->r.w[3])
#define SP (c->r.w[4])
#define PC (c->r.w[5])

#define PTR_REG(x) ptrs(c, (x))
#define MEMSIZE 0x40000

extern void *ops[256];
extern void ops_init();
extern void alu(cpu *c);
extern void incdec(cpu *c);
extern void ldrr(cpu *c);
extern void reg_print(cpu *c);
extern int  mem_load(u8 *at, char *fname);
extern void mem_clear(u8 *a, u32 len);
extern void mem_print(u8 *a, u32 len);
extern void mem_1bpp(u8 *dst, u8 *src, u16 w, u16 h);
extern u8 r8(u16);
extern void w8(u16, u8 v);
extern u8 f8(cpu *c);
extern u8 mem[MEMSIZE];
extern u8 *mem_ptr;
extern u8 *ptrs(cpu* c, u8 idx);
extern const char *optxt[256];
extern int fail;
