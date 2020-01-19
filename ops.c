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
void w8(u16 a, u8 v) { mem[a] = v; printf("W [%04x] <- %02x\n", a, v); }
void w16(u16 a, u16 v) { w8(a,v&0xff); w8(a+1,v>>8); }

void push16(cpu *c, u16 v) { SP -= 2; w16(SP, v); }
u16 pop16(cpu *c) { u16 v = r16(SP); SP+=2; return v; }
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

const char *optxt[256] =
{
  "NOP", "LXI B",  "STAX B", "INX B",  "INR B", "DCR B", "MVI B", "RLC", "NOP", "DAD B",  "LDAX B", "DCX B",  "INR C", "DCR C", "MVI C", "RRC",
  "NOP", "LXI D",  "STAX D", "INX D",  "INR D", "DCR D", "MVI D", "RAL", "NOP", "DAD D",  "LDAX D", "DCX D",  "INR E", "DCR E", "MVI E", "RAR",
  "NOP", "LXI H",  "SHLD",   "INX H",  "INR H", "DCR H", "MVI H", "DAA", "NOP", "DAD H",  "LHLD",   "DCX H",  "INR L", "DCR L", "MVI L", "CMA",
  "NOP", "LXI SP", "STA",    "INX SP", "INR M", "DCR M", "MVI M", "STC", "NOP", "DAD SP", "LDA",    "DCX SP", "INR A", "DCR A", "MVI A", "CMC",

  "MOV", "MOV",   "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV",
  "MOV", "MOV",   "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV",
  "MOV", "MOV",   "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV",
  "MOV", "MOV",   "MOV", "MOV", "MOV", "MOV", "HLT", "MOV [HL], A", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV", "MOV",

  "---", "---",   "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---",
  "---", "---",   "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---",
  "---", "---",   "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---",
  "---", "---",   "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---", "---",

  "RNZ", "POP BC",   "JNZ", "JMP", "CNZ", "PUSH B", "ADI", "RST 0", "RZ", "RET", "JZ", "---", "CZ", "CALL", "ACI", "RST 1",
  "RNC", "POP DE",   "JNC", "OUT", "CNC", "PUSH D", "SUI", "RST 2", "RC", "RET", "JC", "---", "CC", "CALL", "SBI", "RST 3",
  "RPO", "POP HL",   "JPO", "XTHL", "CPO", "PUSH H", "ANI", "RST 4", "RPE", "PCHL", "JPE", "XCHG", "CPE", "CALL", "XRI", "RST 5",
  "RP", "POP AF",   "JP",  "DI", "CP", "PUSH F", "ORI", "RST 6", "RM", "SPHL", "JM", "---", "CM", "CALL", "CPI", "RST 7"
};
