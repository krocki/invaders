.text
.p2align 4
.globl _emu


# rdi - struct cpu
# rsi - mem ptr
# rcx - PC
# al - OP

_emu:
  pushq %rbp
  movq %rsp, %rbp

  xor %rax, %rax         # RAX = 0
  xor %rcx, %rcx         # RCX = 0

  movw 10(%rdi), %cx     # cpu.PC -> CX
  movb (%rsi, %rcx), %al # mem[PC] -> AL

  # do something depending on %al value


  #

  inc %cx                # CX++

  movw %cx, 10(%rdi)     # CX -> cpu.PC
  movb %al, 12(%rdi)     # AL -> cpu.OP

  popq %rbp
  retq
