#include "rt64asm.h"

.section .user.text
.globl spawnuserret
spawnuserret:
  mov %rax, %rdi
  # do not call, since we wont return and have no stack space left
  jmp usysexit
