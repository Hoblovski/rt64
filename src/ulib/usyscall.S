#include "asm.h"

#define SYSCALL(name)                                                          \
.globl usys ## name ;                                                          \
usys ## name:                                                                  \
  movl $SYS_ ## name, %eax ;                                                   \
  int $T_SYSCALL ;                                                             \
  ret

SYSCALL(print)
SYSCALL(exit)

