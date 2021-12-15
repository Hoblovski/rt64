#include "rt64asm.h"

#define SYSCALL(name)                                                          \
.globl usys ## name ;                                                          \
usys ## name:                                                                  \
  movq $SYS_ ## name, %rax ;                                                   \
  int $T_SYSCALL ;                                                             \
  ret

.section .user.text
SYSCALL(print)
SYSCALL(exit)

