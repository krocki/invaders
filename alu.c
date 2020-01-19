#include "defs.h"

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
