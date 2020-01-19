#include "defs.h"

void ldrr(cpu *c) {
    u8 src_idx = c->op & 0x7; //last 3 bits are reg#
    u8 dst_idx = (c->op >> 3) & 0x7; //last 3 bits are reg#
    u8 src = (((c->op >> 6) & 0x3) == 0) ?
      f8(c) : src_idx == 6 ? r8(HL) : *(PTR_REG(src_idx));
    if (dst_idx != 6) *PTR_REG(dst_idx) = src; else w8(HL, src);
}
