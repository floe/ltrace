#include "config.h"

#include <sys/ptrace.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "common.h"
#include "sysdep.h"
#include "breakpoint.h"
#include "proc.h"
#include "library.h"

#ifdef ARCH_HAVE_ENABLE_BREAKPOINT
extern void arch_enable_breakpoint(pid_t, struct breakpoint *);
#else				/* ARCH_HAVE_ENABLE_BREAKPOINT */
void
arch_enable_breakpoint(pid_t pid, struct breakpoint *sbp)
{
	static unsigned char break_insn[] = BREAKPOINT_VALUE;
	unsigned int i, j;

	debug(DEBUG_PROCESS,
	      "arch_enable_breakpoint: pid=%d, addr=%p, symbol=%s",
	      pid, sbp->addr, breakpoint_name(sbp));

	for (i = 0; i < 1 + ((BREAKPOINT_LENGTH - 1) / sizeof(long)); i++) {
		long a = ptrace(PTRACE_PEEKTEXT, pid,
				sbp->addr + i * sizeof(long), 0);
		if (a == -1 && errno) {
			fprintf(stderr, "enable_breakpoint"
				" pid=%d, addr=%p, symbol=%s: %s\n",
				pid, sbp->addr, breakpoint_name(sbp),
				strerror(errno));
			return;
		}
		for (j = 0;
		     j < sizeof(long)
		     && i * sizeof(long) + j < BREAKPOINT_LENGTH; j++) {
			unsigned char *bytes = (unsigned char *)&a;

			sbp->orig_value[i * sizeof(long) + j] = bytes[j];
			bytes[j] = break_insn[i * sizeof(long) + j];
		}
		a = ptrace(PTRACE_POKETEXT, pid,
			   sbp->addr + i * sizeof(long), a);
		if (a == -1) {
			fprintf(stderr, "enable_breakpoint"
				" pid=%d, addr=%p, symbol=%s: %s\n",
				pid, sbp->addr, breakpoint_name(sbp),
				strerror(errno));
			return;
		}
	}
}
#endif				/* ARCH_HAVE_ENABLE_BREAKPOINT */

void
enable_breakpoint(Process *proc, struct breakpoint *sbp)
{
	debug(DEBUG_PROCESS, "enable_breakpoint: pid=%d, addr=%p, symbol=%s",
	      proc->pid, sbp->addr, breakpoint_name(sbp));
	arch_enable_breakpoint(proc->pid, sbp);
}

#ifdef ARCH_HAVE_DISABLE_BREAKPOINT
extern void arch_disable_breakpoint(pid_t, const struct breakpoint *sbp);
#else				/* ARCH_HAVE_DISABLE_BREAKPOINT */
void
arch_disable_breakpoint(pid_t pid, const struct breakpoint *sbp)
{
	unsigned int i, j;

	debug(DEBUG_PROCESS,
	      "arch_disable_breakpoint: pid=%d, addr=%p, symbol=%s",
	      pid, sbp->addr, breakpoint_name(sbp));

	for (i = 0; i < 1 + ((BREAKPOINT_LENGTH - 1) / sizeof(long)); i++) {
		long a = ptrace(PTRACE_PEEKTEXT, pid,
				sbp->addr + i * sizeof(long), 0);
		if (a == -1 && errno) {
			fprintf(stderr,
				"disable_breakpoint pid=%d, addr=%p: %s\n",
				pid, sbp->addr, strerror(errno));
			return;
		}
		for (j = 0;
		     j < sizeof(long)
		     && i * sizeof(long) + j < BREAKPOINT_LENGTH; j++) {
			unsigned char *bytes = (unsigned char *)&a;

			bytes[j] = sbp->orig_value[i * sizeof(long) + j];
		}
		a = ptrace(PTRACE_POKETEXT, pid,
			   sbp->addr + i * sizeof(long), a);
		if (a == -1 && errno) {
			fprintf(stderr,
				"disable_breakpoint pid=%d, addr=%p: %s\n",
				pid, sbp->addr, strerror(errno));
			return;
		}
	}
}
#endif				/* ARCH_HAVE_DISABLE_BREAKPOINT */

void
disable_breakpoint(Process *proc, struct breakpoint *sbp)
{
	debug(DEBUG_PROCESS, "disable_breakpoint: pid=%d, addr=%p, symbol=%s",
	      proc->pid, sbp->addr, breakpoint_name(sbp));
	arch_disable_breakpoint(proc->pid, sbp);
}
