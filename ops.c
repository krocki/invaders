#include "defs.h"

u8* ptrs(cpu* c, u8 idx) {
    u8* _ptrs[8] = { &B, &C, &D, &E, &H, &L, 0, &A};
    return _ptrs[idx];
}

void *ops[256];

u8 r8(u16 a) { return mem[a]; }
u16 r16(u16 a) { return ((u16)(r8(a+1)) << 8) | (u16)(r8(a)); }
u8 f8(cpu *c) { u8 v = r8(PC++); return v; }
u16 f16(cpu *c) { u16 v = r16(PC); PC+=2; return v; }
void w8(u16 a, u8 v) { mem[a] = v; }
void w16(u16 a, u16 v) { w8(a,v&0xff); w8(a+1,v>>8); }

void push16(cpu *c, u16 v) { SP -= 2; w16(SP, v); }
u16 pop16(cpu *c) { u16 v = r16(SP); SP+=2; return v; }
void unk(cpu *c)  { c->fail=1; printf("UNK 0x%04x\n", c->op); };

void port_in(cpu *c) {
  if (c->port_in) {
    A = ((u8 (*)(cpu *c, u8))c->port_in)( c, f8(c) );
  } else {
    puts("port_in is 0");
    c->fail = 1;
  }
}

void port_out(cpu *c) {
  if (c->port_out) {
     ((void (*)(cpu *, u8, u8))c->port_out)( c, f8(c), A );
  } else {
    puts("port_out is 0");
    c->fail = 1;
  }
}

void nop(cpu *c)  { };

void dad(cpu *c, u16 v) {
  u32 res = HL + v;
  fC = (res >> 16) & 1;
  HL = res & 0xffff;
}

//void add16hl(u16 v) {
//  fH = ((HL & 0x07ff) + (v & 0x07ff) > 0x07ff);
//  fN = 0; fC = (HL > (0xffff - v));
//  HL += v;
//}
//
//u16 add16sp(s8 v) {
//  fN = 0; fZ = 0;
//  fH = ((SP & 0x000f) + (v & 0x000f) > 0x000f);
//  fC = ((SP & 0x00ff) + (((u16)((s16)v)) & 0x00ff) > 0x00ff); //?
//  return SP + v;
//}
void dad_bc(cpu *c) { dad(c, BC); } /* 09 */
void dad_de(cpu *c) { dad(c, DE); } /* 19 */
void dad_hl(cpu *c) { dad(c, HL); } /* 29 */
void dad_sp(cpu *c) { dad(c, SP); } /* 39 */

void lxi_bc(cpu *c) { BC = f16(c); } /* 01 */
void lxi_de(cpu *c) { DE = f16(c); } /* 11 */
void lxi_hl(cpu *c) { HL = f16(c); } /* 21 */
void lxi_sp(cpu *c) { SP = f16(c); } /* 31 */

void pop_bc(cpu *c){ BC = pop16(c); } /* c1 */
void pop_de(cpu *c){ DE = pop16(c); } /* d1 */
void pop_hl(cpu *c){ HL = pop16(c); } /* e1 */
void pop_af(cpu *c){ AF = pop16(c); } /* f1 */

void push_bc(cpu *c){ push16(c, BC); } /* c5 */
void push_de(cpu *c){ push16(c, DE); } /* d5 */
void push_hl(cpu *c){ push16(c, HL); } /* e5 */
void push_af(cpu *c){ push16(c, AF); } /* f5 */

void sta_i16(cpu *c) { w8(f16(c), A); }
void lda_i16(cpu *c) { A = r8(f16(c)); }

void xchg(cpu *c) {
  u16 t0 = DE, t1 = HL;
  DE = t1;
  HL = t0;
}

void out(cpu *c) {
  u8 v = f8(c);
  if (v == 0x02) { printf("out shift amt = 0x%02x\n", v); }
  if (v == 0x04) { printf("out shift data = 0x%02x\n", v); }
}

void di(cpu *c) { c->ei = 0; }
void ei(cpu *c) { c->ei = 1; }

void call(cpu *c) { push16(c, PC+2); PC=r16(PC); }
void ret(cpu *c)  { PC = pop16(c); }
void jmp(cpu *c)  { PC = f16(c);}

/* conditional jumps */
void jz(cpu *c)   { u16 a = f16(c); PC =  fZ ? a : PC; } //JZ  a16
void jnz(cpu *c)  { u16 a = f16(c); PC = !fZ ? a : PC; } //JNZ a16
void jc(cpu *c)   { u16 a = f16(c); PC =  fC ? a : PC; } //JC  a16
void jnc(cpu *c)  { u16 a = f16(c); PC = !fC ? a : PC; } //JNC a16
void jpe(cpu *c)  { u16 a = f16(c); PC =  fP ? a : PC; } //JPE a16
void jpo(cpu *c)  { u16 a = f16(c); PC = !fP ? a : PC; } //JPO a16
void jm(cpu *c)   { u16 a = f16(c); PC =  fS ? a : PC; } //JM  a16
void jp(cpu *c)   { u16 a = f16(c); PC = !fS ? a : PC; } //JP  a16

/* conditional calls */
void cz(cpu *c)   { u16 a = f16(c); if ( fZ) { push16(c, PC); PC=a; } } //CZ  a16
void cnz(cpu *c)  { u16 a = f16(c); if (!fZ) { push16(c, PC); PC=a; } } //CNZ a16
void cc(cpu *c)   { u16 a = f16(c); if ( fC) { push16(c, PC); PC=a; } } //CC  a16
void cnc(cpu *c)  { u16 a = f16(c); if (!fC) { push16(c, PC); PC=a; } } //CNC a16
void cpe(cpu *c)  { u16 a = f16(c); if ( fP) { push16(c, PC); PC=a; } } //CPE a16
void cpo(cpu *c)  { u16 a = f16(c); if (!fP) { push16(c, PC); PC=a; } } //CPO a16
void cm(cpu *c)   { u16 a = f16(c); if ( fS) { push16(c, PC); PC=a; } } //CM  a16
void cp(cpu *c)   { u16 a = f16(c); if (!fS) { push16(c, PC); PC=a; } } //CP  a16

/* conditional returns */
void rz(cpu *c)   { PC =  fZ ? pop16(c) : PC; } //RZ  a16
void rnz(cpu *c)  { PC = !fZ ? pop16(c) : PC; } //RNZ a16
void rc(cpu *c)   { PC =  fC ? pop16(c) : PC; } //RC  a16
void rnc(cpu *c)  { PC = !fC ? pop16(c) : PC; } //RNC a16
void rpe(cpu *c)  { PC =  fP ? pop16(c) : PC; } //RPE a16
void rpo(cpu *c)  { PC = !fP ? pop16(c) : PC; } //RPO a16
void rm(cpu *c)   { PC =  fS ? pop16(c) : PC; } //RM  a16
void rp(cpu *c)   { PC = !fS ? pop16(c) : PC; } //RP  a16

void rst_0(cpu *c) { /*call(c, 0x00);*/ printf("rst 00\n"); }
void rst_1(cpu *c) { /*call(c, 0x00);*/ printf("rst 08\n"); }
void rst_2(cpu *c) { /*call(c, 0x00);*/ printf("rst 10\n"); }
void rst_3(cpu *c) { /*call(c, 0x00);*/ printf("rst 18\n"); }
void rst_4(cpu *c) { /*call(c, 0x00);*/ printf("rst 20\n"); }
void rst_5(cpu *c) { /*call(c, 0x00);*/ printf("rst 28\n"); }
void rst_6(cpu *c) { /*call(c, 0x00);*/ printf("rst 30\n"); }
void rst_7(cpu *c) { /*call(c, 0x00);*/ printf("rst 38\n"); }

void ldsp(cpu *c) { SP = f16(c); }
void x11(cpu *c)  { DE = f16(c); }
void x21(cpu *c)  { HL = f16(c); }
void x1a(cpu *c)  { A = r16(DE); }

void x03(cpu *c) { BC++; }
void x13(cpu *c) { DE++; }
void x23(cpu *c) { HL++; }
void x33(cpu *c) { SP++; }
// dec 16cpu *c
void x0b(cpu *c) { BC--; }
void x1b(cpu *c) { DE--; }
void x2b(cpu *c) { HL--; }
void x3b(cpu *c) { SP--; }
// add 16cpu *c to hl
//void x09(cpu *c) { add16hl(BC); }
//void x19(cpu *c) { add16hl(DE); }
//void x29(cpu *c) { add16hl(HL); }
//void x39(cpu *c) { add16hl(SP); }
// add tocpu *c sp
//void xe8(cpu *c) { SP = add16sp((s8)(f8()));   } // SP = SP + s8
//void xf8(cpu *c) { HL = add16sp((s8)(f8()));   } // LD = SP + s8

void ops_init() {

  for (u16 i=0; i<256; i++)
    ops[i] = &unk;

  ops[0x00]=&nop;
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

  /* unconditional jump */
  ops[0xc3]=&jmp;
  /* conditional jumps */
  ops[0xc2]=&jnz; ops[0xca]=&jz;
  ops[0xd2]=&jnc; ops[0xda]=&jc;
  ops[0xe2]=&jpo; ops[0xea]=&jpe;
  ops[0xf2]=&jp;  ops[0xfa]=&jm;

  /* unconditional call */
  ops[0xcd]=&call;
  /* conditional calls */
  ops[0xc4]=&cnz; ops[0xcc]=&cz;
  ops[0xd4]=&cnc; ops[0xdc]=&cc;
  ops[0xe4]=&cpo; ops[0xec]=&cpe;
  ops[0xf4]=&cp;  ops[0xfc]=&cm;

  /* unconditional return */
  ops[0xc9]=&ret;
  /* conditional returns */
  ops[0xc0]=&rnz; ops[0xc8]=&rz;
  ops[0xd0]=&rnc; ops[0xd8]=&rc;
  ops[0xe0]=&rpo; ops[0xe8]=&rpe;
  ops[0xf0]=&rp;  ops[0xf8]=&rm;

  ops[0x04]=&incdec; ops[0x05]=&incdec;
  ops[0x0c]=&incdec; ops[0x0d]=&incdec;
  ops[0x14]=&incdec; ops[0x15]=&incdec;
  ops[0x1c]=&incdec; ops[0x1d]=&incdec;
  ops[0x24]=&incdec; ops[0x25]=&incdec;
  ops[0x2c]=&incdec; ops[0x2d]=&incdec;
  ops[0x34]=&incdec; ops[0x35]=&incdec;
  ops[0x3c]=&incdec; ops[0x3d]=&incdec;

  ops[0xc7]=&rst_0;
  ops[0xcf]=&rst_1;
  ops[0xd7]=&rst_2;
  ops[0xdf]=&rst_3;
  ops[0xe7]=&rst_4;
  ops[0xef]=&rst_5;
  ops[0xf7]=&rst_6;
  ops[0xff]=&rst_7;

  ops[0x0f]=&alu; /* rrc */

  for (u8 i=0x80; i<0xc0; i++) ops[i] = &alu;
  ops[0xc6]=&alu; ops[0xce]=&alu;
  ops[0xd6]=&alu; ops[0xde]=&alu;
  ops[0xe6]=&alu; ops[0xee]=&alu;
  ops[0xf6]=&alu; ops[0xfe]=&alu;

  ops[0xf3]=&di;
  ops[0xfb]=&ei;


  /* push16 group */
  ops[0xc5]=&push_bc;
  ops[0xd5]=&push_de;
  ops[0xe5]=&push_hl;
  ops[0xf5]=&push_af;

  /* pop16 group */
  ops[0xc1]=&pop_bc;
  ops[0xd1]=&pop_de;
  ops[0xe1]=&pop_hl;
  ops[0xf1]=&pop_af;

  /* dad */
  ops[0x09]=&dad_bc;
  ops[0x19]=&dad_de;
  ops[0x29]=&dad_hl;
  ops[0x39]=&dad_sp;

  ops[0xeb]=&xchg;
  ops[0xd3]=&out;

  ops[0x01]=&lxi_bc;
  ops[0x11]=&lxi_de;
  ops[0x21]=&lxi_hl;
  ops[0x31]=&lxi_sp;

  ops[0x32]=&sta_i16;
  ops[0x3a]=&lda_i16;

  ops[0xdb]=&port_in;
  ops[0xd3]=&port_out;

  // 16-bit ALU
  // inc
    ops[0x03] = &x03; ops[0x13] = &x13;
    ops[0x23] = &x23; ops[0x33] = &x33;
  // dec
    ops[0x0b] = &x0b; ops[0x1b] = &x1b;
    ops[0x2b] = &x2b; ops[0x3b] = &x3b;
  // add 16
    //ops[0x09] = &x09; ops[0x19] = &x19;
    //ops[0x29] = &x29; ops[0x39] = &x39;
    //ops[0xe8] = &xe8; ops[0xf8] = &xf8;
  // end 16-bit ALU
}

const char *optxt[256] =
{
  "NOP", "LXI B",  "STAX B", "INC BC",  "INC B", "DEC BC", "MVI B", "RLC", "NOP", "DAD B",  "LDAX B", "DCX B",  "INR C", "DCR C", "MVI C", "RRC",
  "NOP", "LXI D",  "STAX D", "INC DE",  "INC D", "DEC DE", "MVI D", "RAL", "NOP", "DAD D",  "LDAX D", "DCX D",  "INR E", "DCR E", "MVI E", "RAR",
  "NOP", "LXI H",  "SHLD",   "INC HL",  "INC H", "DEC HL", "MVI H", "DAA", "NOP", "DAD H",  "LHLD",   "DCX H",  "INR L", "DCR L", "MVI L", "CMA",
  "NOP", "LXI SP", "STA",    "INC SP", "INC M", "DEC M", "MVI M", "STC", "NOP", "DAD SP", "LDA",    "DCX SP", "INR A", "DCR A", "MVI A", "CMC",

  "MOV", "MOV",   "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV",
  "MOV", "MOV",   "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV",
  "MOV", "MOV",   "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV",
  "MOV", "MOV",   "MOV", "MOV", "MOV", "MOV", "HLT", "MOV [HL], A", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV",

  "---", "---",   "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---",
  "---", "---",   "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---",
  "---", "---",   "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---",
  "---", "---",   "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---",

  "RNZ", "POP BC",   "JNZ", "JMP", "CNZ", "PUSH BC", "ADI", "RST 0", "RZ", "RET", "JZ", "---", "CZ", "CALL", "ACI", "RST 1",
  "RNC", "POP DE",   "JNC", "OUT", "CNC", "PUSH DE", "SUI", "RST 2", "RC", "RET", "JC", "INP", "CC", "CALL", "SBI", "RST 3",
  "RPO", "POP HL",   "JPO", "XTHL", "CPO", "PUSH HL", "AND D8", "RST 4", "RPE", "PCHL", "JPE", "XCHG", "CPE", "CALL", "XRI", "RST 5",
  "RP", "POP AF",   "JP",  "DI", "CP", "PUSH AF", "ORI", "RST 6", "RM", "SPHL", "JM", "---", "CM", "CALL", "CPI", "RST 7"
};
