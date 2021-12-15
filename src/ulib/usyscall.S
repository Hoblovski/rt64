#include "rt64asm.h"

#define SYSCALL(name)                                                          \
.globl usys ## name ;                                                          \
usys ## name:                                                                  \
  movl $SYS_ ## name, %eax ;                                                   \
  int $T_SYSCALL ;                                                             \
  ret

.section .user.text
SYSCALL(print)
SYSCALL(exit)

