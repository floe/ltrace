#include "config.h"

#include <sys/types.h>
#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <errno.h>
#include <error.h>

#include "proc.h"
#include "common.h"

#if (!defined(PTRACE_PEEKUSER) && defined(PTRACE_PEEKUSR))
# define PTRACE_PEEKUSER PTRACE_PEEKUSR
#endif

#if (!defined(PTRACE_POKEUSER) && defined(PTRACE_POKEUSR))
# define PTRACE_POKEUSER PTRACE_POKEUSR
#endif

void *
get_instruction_pointer(Process *proc) {
	return (void *)ptrace(PTRACE_PEEKUSER, proc->pid, sizeof(long)*PT_NIP, 0);
}

void
set_instruction_pointer(Process *proc, void *addr)
{
	if (ptrace(PTRACE_POKEUSER, proc->pid, sizeof(long)*PT_NIP, addr) != 0)
		error(0, errno, "set_instruction_pointer");
}

void *
get_stack_pointer(Process *proc) {
	return (void *)ptrace(PTRACE_PEEKUSER, proc->pid, sizeof(long)*PT_R1, 0);
}

void *
get_return_addr(Process *proc, void *stack_pointer) {
	return (void *)ptrace(PTRACE_PEEKUSER, proc->pid, sizeof(long)*PT_LNK, 0);
}

void
set_return_addr(Process *proc, void *addr) {
	ptrace(PTRACE_POKEUSER, proc->pid, sizeof(long)*PT_LNK, addr);
}

/* Grab the value of CTR registers.  */
void *
get_count_register (Process *proc) {
	return (void *) ptrace (PTRACE_PEEKUSER, proc->pid,
				sizeof (long) * PT_CTR, 0);
}
