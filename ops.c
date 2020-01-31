#include "defs.h"

int fail=0;

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
void unk(cpu *c)  { fail=1; printf("UNK 0x%04x\n", c->op); };
void nop(cpu *c)  { };

void dad(cpu *c, u16 v) {
  u32 res = HL + v;
  fC = (res >> 16) & 1;
  HL = res & 0xffff;
}

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
  if (v != 0x06) printf("out = 0x%02x\n", v);
}

void di(cpu *c) {
  printf("disable interrupts\n");
}

void ei(cpu *c) {
  printf("enable interrupts\n");
}

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
  ops[0xc2]=&jnz;
  ops[0xc3]=&jmp;
  ops[0xc9]=&ret;

  ops[0x04]=&incdec;
  ops[0x05]=&incdec;
  ops[0x0c]=&incdec;
  ops[0x0d]=&incdec;
  ops[0x14]=&incdec;
  ops[0x15]=&incdec;
  ops[0x1c]=&incdec;
  ops[0x1d]=&incdec;
  ops[0x24]=&incdec;
  ops[0x25]=&incdec;
  ops[0x2c]=&incdec;
  ops[0x2d]=&incdec;
  ops[0x34]=&incdec;
  ops[0x35]=&incdec;
  ops[0x3c]=&incdec;
  ops[0x3d]=&incdec;

  ops[0x0f]=&alu; /* rrc */

  for (u8 i=0x80; i<0xc0; i++) ops[i] = &alu;
  ops[0xc6]=&alu; ops[0xce]=&alu;
  ops[0xd6]=&alu; ops[0xde]=&alu;
  ops[0xe6]=&alu; ops[0xee]=&alu;
  ops[0xf6]=&alu; ops[0xfe]=&alu;

  ops[0xf3]=&di;
  ops[0xfb]=&ei;

  ops[0xcd]=&call;

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
  "RNC", "POP DE",   "JNC", "OUT", "CNC", "PUSH DE", "SUI", "RST 2", "RC", "RET", "JC", "---", "CC", "CALL", "SBI", "RST 3",
  "RPO", "POP HL",   "JPO", "XTHL", "CPO", "PUSH HL", "AND D8", "RST 4", "RPE", "PCHL", "JPE", "XCHG", "CPE", "CALL", "XRI", "RST 5",
  "RP", "POP AF",   "JP",  "DI", "CP", "PUSH AF", "ORI", "RST 6", "RM", "SPHL", "JM", "---", "CM", "CALL", "CPI", "RST 7"
};
