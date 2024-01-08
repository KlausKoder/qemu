#include "qemu/osdep.h"
#include "qapi/error.h"
#include "exec/exec-all.h"
#include "sysemu/cpus.h"
#include "qemu/seqlock.h"
#include "sysemu/cpu-timers.h"


/* icount - Instruction Counter API */

int use_icount = 1;


/* qemu_icount - Number of instructions executed.
   CONFIG_USER_ONLY: Single thread, use a local counter. */

static int64_t qemu_icount;

void icount_configure(QemuOpts *opts, Error **errp)
{
    /* signal error */
    error_setg(errp, "cannot configure icount, TCG support not available");
}

int64_t icount_to_ns(int64_t icount)
{
    abort();
    return 0;
}
int64_t icount_round(int64_t count)
{
    abort();
    return 0;
}
void icount_start_warp_timer(void)
{
    abort();
}
void icount_account_warp_timer(void)
{
    abort();
}

void icount_notify_exit(void)
{
}

static int64_t icount_get_executed(CPUState *cpu)
{
    return (cpu->icount_budget -
            (cpu_neg(cpu)->icount_decr.u16.low + cpu->icount_extra));
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
    cpu->tcg_cflags = icount_enabled() ? CF_USE_ICOUNT : 0;;

    cpu->icount_budget -= executed;
    qemu_icount = qemu_icount + executed;
}

int64_t icount_get_raw(void)
{
    CPUState *cpu = current_cpu;
    icount_update(cpu);

    return qemu_icount;
}

int64_t icount_get(void)
{
    return icount_get_raw();
}


