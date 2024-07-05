/*
 *  ARM translation: M-profile Beco instructions
 *
 *  Copyright (c) 2024 BES Technic, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"

#if defined CONFIG_TARGET_ARM_M_BECO && CONFIG_TARGET_ARM_M_BECO != 0

#include "qemu/log.h"
#include "translate.h"
#include "translate-a32.h"

#include "decode-m-beco.c.inc"

#define HELPER_H "helper-m-beco.h"
#include "exec/helper-proto.h.inc"
#include "exec/helper-gen.h.inc"
#include "exec/helper-info.c.inc"
#undef  HELPER_H

#include "beco/include/beco-sim.h"

void HELPER(beco_cdp)(CPUARMState *env, uint32_t cp_cmd)
{
    arg_beco cmd;
    disas_m_beco_extract_cdp(NULL, &cmd, cp_cmd);
    //qemu_log_mask(CPU_LOG_INT, "   cr%d = f(cr%d, cr%d)\n", cmd.crd, cmd.crm, cmd.crn);

    CDP(0, cmd.opc1, cmd.opc2, cmd.crd, cmd.crm,  cmd.crn);
}

void HELPER(beco_mcr)(CPUARMState *env, uint32_t cp_cmd, uint32_t rt)
{
    arg_beco cmd;
    disas_m_beco_extract_mcr_mrc(NULL, &cmd, cp_cmd);
    //qemu_log_mask(CPU_LOG_INT, "   cr%d = f(cr%d, r%d(=%08x))\n", cmd.crm, cmd.crn, cmd.rt, rt);

    MCR(0, cmd.opc1, cmd.opc2, rt, cmd.crm,  cmd.crn);
}


uint32_t HELPER(beco_mrc)(CPUARMState *env, uint32_t cp_cmd)
{
    arg_beco cmd;
    disas_m_beco_extract_mcr_mrc(NULL, &cmd, cp_cmd);

    uint32_t result = MRC(0, cmd.opc1, cmd.opc2, cmd.crm,  cmd.crn);
    //qemu_log_mask(CPU_LOG_INT, "   r%d = f(cr%d, cr%d) (=%08x)\n", cmd.rt, cmd.crm, cmd.crn, result);

    return result;
}

void HELPER(beco_mcrr)(CPUARMState *env, uint32_t cp_cmd, uint32_t rt, uint32_t rt2)
{
    arg_beco cmd;
    disas_m_beco_extract_mcrr_mrrc(NULL, &cmd, cp_cmd);
    //qemu_log_mask(CPU_LOG_INT, "   cr%d = f(r%d(=%08x), r%d(=%08x))\n", cmd.crm, cmd.rt, rt, cmd.rt2,rt2);

    MCRR(0, cmd.opc1, rt, rt2, cmd.crm);
}

uint64_t HELPER(beco_mrrc)(CPUARMState *env, uint32_t cp_cmd)
{
    arg_beco cmd;
    disas_m_beco_extract_mcrr_mrrc(NULL, &cmd, cp_cmd);

    uint64_t result = MRRC_R(0, cmd.opc1, cmd.crm);
    //qemu_log_mask(CPU_LOG_INT, "   {r%d,r%d} = f(cr%d) (=%016lx)\n", cmd.rt, cmd.rt2, cmd.crm, result);

    return result;
}



// CDP   void fn{opc1, opc2}(crd, crn, crm)
//   DEF_HELPER_2(beco_cdp, void, env, cptr)
static bool trans_BECO_CDP(DisasContext *ctx, arg_BECO_CDP *cmd)
{
    TCGv_i32 cp_cmd;
    if (!arm_dc_feature(ctx, ARM_FEATURE_M)) {
        return false;
    }
    qemu_log_mask(CPU_LOG_INT, "Translate CDP opc1:%01x opc2:%01x\n", cmd->opc1, cmd->opc2);

    cp_cmd = tcg_constant_i32(ctx->insn);
    gen_helper_beco_cdp(tcg_env, cp_cmd);

    return true;
}


// MCR   void fn{opc1, opc2}(crn, crm), rt
//   DEF_HELPER_3(beco_mcr, void, env, cptr, i32)
static bool trans_BECO_MCR(DisasContext *ctx, arg_BECO_MCR *cmd)
{
    TCGv_i32 t0;
    TCGv_i32 cp_cmd;

    if (!arm_dc_feature(ctx, ARM_FEATURE_M)) {
        return false;
    }
    qemu_log_mask(CPU_LOG_INT, "Translate MCR f(opc1:%01x opc2:%01x) <- rt:%d\n", cmd->opc1, cmd->opc2, cmd->rt);

    cp_cmd = tcg_constant_i32(ctx->insn);
    t0 = load_reg(ctx, cmd->rt);
    gen_helper_beco_mcr(tcg_env, cp_cmd, t0);

    return true;

}


// MRC   rt  fn{opc1, opc2}(crn, crm), rt
//   DEF_HELPER_2(beco_mrc, i32, env, cptr)
static bool trans_BECO_MRC(DisasContext *ctx, arg_BECO_MRC *cmd)
{
    TCGv_i32 tmp;
    TCGv_i32 cp_cmd;

    if (!arm_dc_feature(ctx, ARM_FEATURE_M)) {
        return false;
    }
    qemu_log_mask(CPU_LOG_INT, "Translate MRC  rt:%d = f(opc1:%01x opc2:%01x)\n", cmd->rt, cmd->opc1, cmd->opc2);

    cp_cmd = tcg_constant_i32(ctx->insn);
    tmp = tcg_temp_new_i32();
    gen_helper_beco_mrc(tmp, tcg_env, cp_cmd);
    store_reg(ctx, cmd->rt, tmp);

    return true;
}



// MCRR   void  fn{opc1}(crm), rt, rt2
//   DEF_HELPER_4(beco_mcrr, void, env, cptr, i32, i32)
static bool trans_BECO_MCRR(DisasContext *ctx, arg_BECO_MCRR *cmd)
{
    TCGv_i32 t0, t1;
    TCGv_i32 cp_cmd;

    if (!arm_dc_feature(ctx, ARM_FEATURE_M)) {
        return false;
    }
    qemu_log_mask(CPU_LOG_INT, "Translate MCRR {opc1:%1x, crm:%1x}\n", cmd->opc1, cmd->crm);

    cp_cmd = tcg_constant_i32(ctx->insn);
    t0 = load_reg(ctx, cmd->rt);
    t1 = load_reg(ctx, cmd->rt2);
    gen_helper_beco_mcrr(tcg_env, cp_cmd, t0, t1);

    return true;
}



// MRRC   {rt, rt2}  fn{t2, opc1}(crm)
//   DEF_HELPER_4(beco_mrrc, env, cptr, ptr, ptr)
static bool trans_BECO_MRRC(DisasContext *ctx, arg_BECO_MRRC *cmd)
{
    TCGv_i64 t64;
    TCGv_i32 tl, th;
    TCGv_i32 cp_cmd;

    if (!arm_dc_feature(ctx, ARM_FEATURE_M)) {
        return false;
    }
    qemu_log_mask(CPU_LOG_INT, "Translate MRRC {opc1:%1x, crm:%1x}\n", cmd->opc1, cmd->crm);

    cp_cmd = tcg_constant_i32(ctx->insn);

    t64 = tcg_temp_new_i64();
    gen_helper_beco_mrrc(t64, tcg_env, cp_cmd);

    tl = tcg_temp_new_i32();
    th = tcg_temp_new_i32();
    tcg_gen_extr_i64_i32(tl, th, t64);

    store_reg(ctx, cmd->rt, tl);
    store_reg(ctx, cmd->rt2, th);

    return true;
}

#endif

