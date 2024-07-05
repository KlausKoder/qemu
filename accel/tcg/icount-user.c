#include "qemu/osdep.h"
#include "exec/exec-all.h"
#include "sysemu/cpus.h"
#include "sysemu/cpu-timers.h"

/*
 * ICOUNT: Instruction Counter
 *
 * this module is split off from cpu-timers because the icount part
 * is TCG-specific, and does not need to be built for other accels.
 * This file is seperate from icount-common.c because that can't
 * be build for USER_MODE
 */
ICountMode use_icount = ICOUNT_PRECISE;


/* qemu_icount - Number of instructions executed.
   CONFIG_USER_ONLY: Single thread, use a local counter. */

static int64_t qemu_icount;

/*
 * The current number of executed instructions is based on what we
 * originally budgeted minus the current state of the decrementing
 * icount counters in extra/u16.low.
 */
static int64_t icount_get_executed(CPUState *cpu)
{
    return (cpu->icount_budget -
            (cpu->neg.icount_decr.u16.low + cpu->icount_extra));
}

/*
 * Update the global shared timer_state.qemu_icount to take into
 * account executed instructions. This is done by the TCG vCPU
 * thread so the main-loop can see time has moved forward.
 */
void icount_update(CPUState *cpu)
{
    int64_t executed = icount_get_executed(cpu);

    // Make sure that CF_USE_ICOUNT is set if icount_update()
    // is called in user emulation.
    cpu->tcg_cflags = icount_enabled() ? CF_USE_ICOUNT : 0;

    cpu->icount_budget -= executed;
    qemu_icount = qemu_icount + executed;
}

/* Return the virtual CPU time, based on the instruction counter.  */
int64_t icount_get(void)
{
    CPUState *cpu = current_cpu;
    icount_update(cpu);

    return qemu_icount;
}


