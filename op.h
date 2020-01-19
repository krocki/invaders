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
