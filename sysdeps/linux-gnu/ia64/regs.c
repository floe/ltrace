#include "config.h"

#include <sys/types.h>
#include <sys/ptrace.h>
#include <errno.h>

#include <asm/ptrace_offsets.h>
#include <asm/rse.h>

#include <stddef.h>
#include "common.h"

void *
get_instruction_pointer(Process *proc) {
	unsigned long ip = ptrace(PTRACE_PEEKUSER, proc->pid, PT_CR_IIP, 0);
	unsigned long slot =
	    (ptrace(PTRACE_PEEKUSER, proc->pid, PT_CR_IPSR, 0) >> 41) & 3;

	return (void *)(ip | slot);
}

void
set_instruction_pointer(Process *proc, void *addr) {

	unsigned long newip = (unsigned long)addr;
	unsigned long slot = (unsigned long)addr & 0xf;
	unsigned long psr = ptrace(PTRACE_PEEKUSER, proc->pid, PT_CR_IPSR, 0);

	psr &= ~(3UL << 41);
	psr |= (slot & 0x3) << 41;

	newip &= ~0xfUL;

	ptrace(PTRACE_POKEUSER, proc->pid, PT_CR_IIP, (long)newip);
	ptrace(PTRACE_POKEUSER, proc->pid, PT_CR_IPSR, psr);
}

void *
get_stack_pointer(Process *proc) {
	long l = ptrace(PTRACE_PEEKUSER, proc->pid, PT_R12, 0);
	if (l == -1 && errno)
		return NULL;
	return (void *)l;
}

void *
get_return_addr(Process *proc, void *stack_pointer) {
	long l = ptrace(PTRACE_PEEKUSER, proc->pid, PT_B0, 0);
	if (l == -1 && errno)
		return NULL;
	return (void *)l;
}

void
set_return_addr(Process *proc, void *addr) {
	ptrace(PTRACE_POKEUSER, proc->pid, PT_B0, addr);
}
