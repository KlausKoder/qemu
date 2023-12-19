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
#include <inttypes.h>
#include <assert.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <glib.h>

#include <qemu-plugin.h>

QEMU_PLUGIN_EXPORT int qemu_plugin_version = QEMU_PLUGIN_VERSION;

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))


static bool do_inline;
static bool verbose;


typedef struct {
    const char *name;
    const char *rev;
    uint32_t mask;
    uint32_t pattern;
} BecoInsn;



/*
 * Matchers for instructions, order is important.
 *
 * Your most precise match must be before looser matches. If no match
 * is found in the table we can create an individual entry.
 */
static BecoInsn beco_insn[] = {
    // Note Instructions are always {uint16[, uint16]} so first uint16 is low 16 bit
    // in 32 bit word below.

    // 1110 1110, 000 1 0000 .... 0000 001, 1 0000  status
    { "beco_status",       "v1",     0x0fffffff, 0x0030ee10},
    // 1110 1110, 000 1 0000 .... 0000 000, 1 0000  cpid
    { "beco_cpid",         "v1",     0x0fffffff, 0x0010ee10},
};


static const BecoInsn *find_instruction(struct qemu_plugin_insn *insn)
{
    int i;
    uint32_t opcode;
    BecoInsn *class = NULL;

    opcode = *((uint32_t *)qemu_plugin_insn_data(insn));
    for (i = 0; i < ARRAY_SIZE(beco_insn); i++) {
        class = &beco_insn[i];
        uint32_t masked_bits = opcode & class->mask;
        if (masked_bits == class->pattern) {
            printf("Found Beco: %s\n", class->name);
            return class;
        }
    }
    return NULL;
}


static void vcpu_udata_cb(unsigned int cpuid, void *userdata)
{
    const BecoInsn *beco_inst = (const BecoInsn *)userdata;
    printf("Executing: %s\n", beco_inst->name);
}



static void vcpu_tb_trans(qemu_plugin_id_t id, struct qemu_plugin_tb *tb)
{
    size_t n = qemu_plugin_tb_n_insns(tb);
    size_t i;
    void *userdata = NULL;

    for (i = 0; i < n; i++) {
        const BecoInsn *beco_inst;
        struct qemu_plugin_insn *insn = qemu_plugin_tb_get_insn(tb, i);
        beco_inst = find_instruction(insn);
        if (beco_inst) {
            qemu_plugin_register_vcpu_tb_exec_cb(tb,
                                          vcpu_udata_cb,
                                          0,
                                          (void*)beco_inst);
        }
    }
}

QEMU_PLUGIN_EXPORT int qemu_plugin_install(qemu_plugin_id_t id,
                                           const qemu_info_t *info,
                                           int argc, char **argv)
{
    const char beco_target[] = "aarch32";
    int i;

    if (strcmp(beco_target, info->target_name) == 0) {
                fprintf(stderr, "Beco plugin need %s arch, got: %s\n", beco_target, info->target_name);
                return -1;
    }

    for (i = 0; i < argc; i++) {
        char *p = argv[i];
        g_auto(GStrv) tokens = g_strsplit(p, "=", -1);
        if (g_strcmp0(tokens[0], "inline") == 0) {
            if (!qemu_plugin_bool_parse(tokens[0], tokens[1], &do_inline)) {
                fprintf(stderr, "boolean argument parsing failed: %s\n", p);
                return -1;
            }
        } else if (g_strcmp0(tokens[0], "verbose") == 0) {
            if (!qemu_plugin_bool_parse(tokens[0], tokens[1], &verbose)) {
                fprintf(stderr, "boolean argument parsing failed: %s\n", p);
                return -1;
            }
        } else {
            fprintf(stderr, "option parsing failed: %s\n", p);
            return -1;
        }
    }

    qemu_plugin_register_vcpu_tb_trans_cb(id, vcpu_tb_trans);

    return 0;
}
