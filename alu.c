#include "defs.h"

u8 parity(u8 v) {
  u8 n = 1;
  for (u8 i=0; i<8; i++)
    n ^= ((v >> i) & 1);
  return n;
}

u8 inc8(cpu* c, u8 v) {
   u8 r = v + 1;
   fZ = (r == 0); fH = ((v & 0x0f) + 1 > 0x0f); fS = r >> 7;
   fP = parity(r);
   return r;
}

u8 dec8(cpu* c, u8 v) {
  u8 r = v - 1;
  fZ = (r == 0); fH = ((v & 0x0f) == 0); fS = r >> 7;
  fP = parity(r);
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

void set_flags(cpu *c, u8 v) {
  fZ = (v == 0); fS = (v >> 7) & 0x1; fP = parity(A);
}

void _add8(cpu *c, u8 v, u8 carry) {
  u8 c0 = carry ? fC : 0;
  u8 r = A;
  r = A + v + c0;
  fH = (((A & 0xf) + (v & 0xf) + c0) > 0xf) ? 1 : 0;
  fC = (((u16)(A) + (u16)(v) + (u16)(c0)) > 0x00ff) ? 1 : 0;
  A = r;
  set_flags(c, A);
}

void _sub8(cpu *c, u8 v, u8 carry) {
  // use carry?
  u8 _c = carry ? fC : 0;
  u8 r = A;
  r = A - v - _c;
  // update flags
  fH = (((A & 0xf) < ((v & 0xf) + _c))) ? 1 : 0;
  fC = (((u16)(A) < (u16)(v) + (u16)(_c))) ? 1 : 0;
  A = r;
  set_flags(c, A);
}


void add8(cpu *c, u8 v) { _add8(c, v, 0); }
void adc8(cpu *c, u8 v) { _add8(c, v, 1); }
void sub8(cpu *c, u8 v) { _sub8(c, v, 0); }
void sbc8(cpu *c, u8 v) { _sub8(c, v, 1); }

void cp8 (cpu *c, u8 v) { u8 r = A; _sub8(c, v, 0); A = r; }
void and8(cpu *c, u8 v) {
  fH = !!((A | v) & 0x08); fC = 0; A &= v; set_flags(c, A);
}
void or8(cpu *c, u8 v)  { A |= v; fH = 0; set_flags(c, A); }
void xor8(cpu *c, u8 v) { A ^= v; fH = 0; set_flags(c, A); }

// 8-bit alu
void alu(cpu *c) {
  u8 src_idx = c->op & 0x7; //last 3 bits are reg#
  u8 src = (((c->op >> 6) & 0x3) == 3) ?
  f8(c) : src_idx == 6 ? r8(HL) : *(PTR_REG(src_idx));
  u8 n = (c->op >> 3) & 0x07;

  switch (n) { // subgroup, bits xxNNNyyy
    case 0: add8(c, src); break; // 00000yyy
    case 1: adc8(c, src); break; // 00001yyy
    case 2: sub8(c, src); break; // 00010yyy
    case 3: sbc8(c, src); break; // 00011yyy
    case 4: and8(c, src); break; // 00100yyy
    case 5: xor8(c, src); break; // 00101yyy
    case 6: or8(c, src);  break; // 00110yyy
    case 7: cp8(c, src);  break; // 00111yyy
  };
}
