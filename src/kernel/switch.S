# Context switch between kernel threads
#   void swtch(struct context **old, struct context *new)
#	push *current_context
#	*old = current_stack_top
#	*current_context = *new
#
# Clobbers caller-save registers

.globl swtch
swtch:

# save to old
  mov %r15, (%rdi)
  mov %r14, 8(%rdi)
  mov %r13, 16(%rdi)
  mov %r12, 24(%rdi)
  mov %rbx, 32(%rdi)
  mov %rbp, 40(%rdi)

  # save stack
  mov %rsp, 56(%rdi)

  # copy %rip from stack
  mov (%rsp), %rax
  mov %rax, 48(%rdi)

# restore from new
  mov (%rsi), %r15
  mov 8(%rsi), %r14
  mov 16(%rsi), %r13
  mov 24(%rsi), %r12
  mov 32(%rsi), %rbx
  mov 40(%rsi), %rbp

  # restore stack
  mov 56(%rsi), %rsp

  # save %rip to stack
  mov 48(%rsi), %rax
  mov %rax, (%rsp)

  ret
