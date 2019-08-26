#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct Flags {
    uint8_t z:1;
    uint8_t s:1;
    uint8_t p:1;
    uint8_t cy:1;
    uint8_t ac:1;
    uint8_t pad:3;
} Flags;

typedef struct State8080 {
    union {
        struct {
            Flags cc;
            uint8_t a;
        } s;
        uint16_t af;
    } af;
    union {
        struct {
            uint8_t c;
            uint8_t b;
        } s;
        uint16_t bc;
    } bc;
    union {
        struct {
            uint8_t e;
            uint8_t d;
        } s;
        uint16_t de;
    } de;
    union {
        struct {
            uint8_t l;
            uint8_t h;
        } s;
        uint16_t hl;
    } hl;
    uint16_t sp;
    uint16_t pc;
    uint8_t int_enable;
    bool running;
    uint8_t memory[65536];
} State8080;

uint8_t parity(uint16_t x)
{
	uint8_t p = 0;
	x = (x & ((1 << 8) - 1));
	for (uint8_t i = 0; i < 8; i++)
	{
		if (x & 1) p++;
		x = x >> 1;
	}
	return (p & 1) == 0;
}

void update_flags_nc(Flags *flags, uint16_t n)
{
    flags->z = ((n & 0xFF) == 0);
    flags->s = ((n & 0x80) != 0);
    flags->p = parity(n & 0xFF);
    flags->ac = 0;
}

void update_flags(Flags *flags, uint16_t n)
{
    update_flags_nc(flags, n);
    flags->cy = (n > 0xFF);
}

/* INSTRUCTIONS */

void nop(State8080 *state, uint8_t *opcode)
{
    return;
}


void lxi_bc(State8080 *state, uint8_t *opcode)
{
    state->bc.s.b = opcode[2];
    state->bc.s.c = opcode[1];
    state->pc += 2;
}

void stax_b(State8080 *state, uint8_t *opcode)
{
    state->memory[state->bc.bc] = state->af.s.a;
}

void inx_b(State8080 *state, uint8_t *opcode)
{
    ++state->bc.bc;
}

void inr_b(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, ++state->bc.s.b);
}

void dcr_b(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, --state->bc.s.b);
}

void mvi_b(State8080 *state, uint8_t *opcode)
{
    state->bc.s.b = opcode[1];
    state->pc += 1;
}

void rlc(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = ((state->af.s.a >> 7) & 1) | (state->af.s.a << 1);
}

void dad_b(State8080 *state, uint8_t *opcode)
{
    uint16_t org = state->hl.hl;
    state->hl.hl += state->bc.bc;
    state->af.s.cc.cy = (org >> 15 & 1) && (!(state->hl.hl >> 15 & 1));
}

void ldax_b(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = state->memory[state->bc.bc];
}

void dcx_b(State8080 *state, uint8_t *opcode)
{
    --state->bc.bc;
}

void inr_c(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, ++state->bc.s.c);
}

void dcr_c(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, --state->bc.s.c);
}

void mvi_c(State8080 *state, uint8_t *opcode)
{
    state->bc.s.c = opcode[1];
    state->pc += 1;
}

void rrc(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = ((state->af.s.a & 1) << 7) | (state->af.s.a >> 1);
}

void lxi_de(State8080 *state, uint8_t *opcode)
{
    state->de.s.d = opcode[2];
    state->de.s.e = opcode[1];
    state->pc += 2;
}

void stax_d(State8080 *state, uint8_t *opcode)
{
    state->memory[state->de.de] = state->af.s.a;
}

void inx_d(State8080 *state, uint8_t *opcode)
{
    ++state->de.de;
}

void inr_d(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, ++state->de.s.d);
}

void dcr_d(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, --state->de.s.d);
}

void mvi_d(State8080 *state, uint8_t *opcode)
{
    state->de.s.d = opcode[1];
    state->pc += 1;
}

void ral(State8080 *state, uint8_t *opcode)
{
    uint8_t cy = ((state->af.s.a >> 7) & 1);
    state->af.s.a = (state->af.s.cc.cy) | (state->af.s.a << 1);
    state->af.s.cc.cy = cy;
}

void dad_d(State8080 *state, uint8_t *opcode)
{
    state->hl.hl += state->de.de;
}

void ldax_d(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = state->memory[state->de.de];
}

void dcx_d(State8080 *state, uint8_t *opcode)
{
    --state->de.de;
}

void inr_e(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, ++state->de.s.e);
}

void dcr_e(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, --state->de.s.e);
}

void mvi_e(State8080 *state, uint8_t *opcode)
{
    state->de.s.e = opcode[1];
    state->pc += 1;
}

void rar(State8080 *state, uint8_t *opcode)
{
    uint8_t cy = (state->af.s.a & 1);
    state->af.s.a = (state->af.s.cc.cy << 7) | (state->af.s.a >> 1);
    state->af.s.cc.cy = cy;
}

void lxi_hl(State8080 *state, uint8_t *opcode)
{
    state->hl.s.h = opcode[2];
    state->hl.s.l = opcode[1];
    state->pc += 2;
}

void shld_addr(State8080 *state, uint8_t *opcode)
{
    uint16_t addr = (opcode[1]) | (opcode[2] << 8);
    state->memory[addr] = state->hl.s.l;
    state->memory[addr + 1] = state->hl.s.h;
    state->pc += 2;
}

void inx_h(State8080 *state, uint8_t *opcode)
{
    ++state->hl.hl;
}

void inr_h(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, ++state->hl.s.h);
}

void dcr_h(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, --state->hl.s.h);
}

void mvi_h(State8080 *state, uint8_t *opcode)
{
    state->hl.s.h = opcode[1];
    state->pc += 1;
}

void daa(State8080 *state, uint8_t *opcode)
{
}

void dad_h(State8080 *state, uint8_t *opcode)
{
    state->hl.hl += state->hl.s.h;
}

void lhld_addr(State8080 *state, uint8_t *opcode)
{
    uint16_t addr = (opcode[1]) | (opcode[2] << 8);
    state->hl.s.l = state->memory[addr];
    state->hl.s.h = state->memory[addr + 1];
    state->pc += 2;
}

void dcx_h(State8080 *state, uint8_t *opcode)
{
    --state->hl.hl;
}

void inr_l(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, ++state->hl.s.l);
}

void dcr_l(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, --state->hl.s.l);
}

void mvi_l(State8080 *state, uint8_t *opcode)
{
    state->hl.s.l = opcode[1];
    state->pc += 1;
}

void cma(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = ~state->af.s.a;
}

void lxi_sp(State8080 *state, uint8_t *opcode)
{
    state->sp = (opcode[2] << 8) | (opcode[1]);
}

void sta_addr(State8080 *state, uint8_t *opcode)
{
    uint16_t addr = (opcode[1]) | (opcode[2] << 8);
    state->memory[addr] = state->af.s.a;
    state->pc += 2;
}

void inx_sp(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, ++state->sp);
}

void inr_m(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, ++state->memory[state->hl.hl]);
}

void dcr_m(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, --state->memory[state->hl.hl]);
}

void mvi_m(State8080 *state, uint8_t *opcode)
{
    state->memory[state->hl.hl] = opcode[1];
    state->pc += 1;
}

void stc(State8080 *state, uint8_t *opcode)
{
    state->af.s.cc.cy = 1;
}

void dad_sp(State8080 *state, uint8_t *opcode)
{
    state->hl.hl += state->sp;
}

void lda_addr(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = state->memory[(opcode[1]) | (opcode[2] << 8)];
    state->pc += 2;
}

void dcx_sp(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, --state->sp);
}

void inr_a(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, ++state->af.s.a);
}

void dcr_a(State8080 *state, uint8_t *opcode)
{
    update_flags_nc(&state->af.s.cc, --state->af.s.a);
}

void mvi_a(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = opcode[1];
    state->pc += 1;
}

void cmc(State8080 *state, uint8_t *opcode)
{
    state->af.s.cc.cy = ~state->af.s.cc.cy;
}

void mov_bb(State8080 *state, uint8_t *opcode)
{
    state->bc.s.b = state->bc.s.b;
}

void mov_bc(State8080 *state, uint8_t *opcode)
{
    state->bc.s.b = state->bc.s.c;
}

void mov_bd(State8080 *state, uint8_t *opcode)
{
    state->bc.s.b = state->de.s.d;
}

void mov_be(State8080 *state, uint8_t *opcode)
{
    state->bc.s.b = state->de.s.e;
}

void mov_bh(State8080 *state, uint8_t *opcode)
{
    state->bc.s.b = state->hl.s.h;
}

void mov_bl(State8080 *state, uint8_t *opcode)
{
    state->bc.s.b = state->hl.s.l;
}

void mov_bm(State8080 *state, uint8_t *opcode)
{
    state->bc.s.b = state->memory[state->hl.hl];
}

void mov_ba(State8080 *state, uint8_t *opcode)
{
    state->bc.s.b = state->af.s.a;
}

void mov_cb(State8080 *state, uint8_t *opcode)
{
    state->bc.s.c = state->bc.s.b;
}

void mov_cc(State8080 *state, uint8_t *opcode)
{
    state->bc.s.c = state->bc.s.c;
}

void mov_cd(State8080 *state, uint8_t *opcode)
{
    state->bc.s.c = state->de.s.d;
}

void mov_ce(State8080 *state, uint8_t *opcode)
{
    state->bc.s.c = state->bc.s.b;
}

void mov_ch(State8080 *state, uint8_t *opcode)
{
    state->bc.s.c = state->hl.s.h;
}

void mov_cl(State8080 *state, uint8_t *opcode)
{
    state->bc.s.c = state->hl.s.l;
}

void mov_cm(State8080 *state, uint8_t *opcode)
{
    state->bc.s.c = state->memory[state->hl.hl];
}

void mov_ca(State8080 *state, uint8_t *opcode)
{
    state->bc.s.c = state->af.s.a;
}

void mov_db(State8080 *state, uint8_t *opcode)
{
    state->de.s.d = state->bc.s.b;
}

void mov_dc(State8080 *state, uint8_t *opcode)
{
    state->de.s.d = state->bc.s.c;
}

void mov_dd(State8080 *state, uint8_t *opcode)
{
    state->de.s.d = state->de.s.d;
}

void mov_de(State8080 *state, uint8_t *opcode)
{
    state->de.s.d = state->de.s.e;
}

void mov_dh(State8080 *state, uint8_t *opcode)
{
    state->de.s.d = state->hl.s.h;
}

void mov_dl(State8080 *state, uint8_t *opcode)
{
    state->de.s.d = state->hl.s.l;
}

void mov_dm(State8080 *state, uint8_t *opcode)
{
    state->de.s.d = state->memory[state->hl.hl];
}

void mov_da(State8080 *state, uint8_t *opcode)
{
    state->de.s.d = state->af.s.a;
}

void mov_eb(State8080 *state, uint8_t *opcode)
{
    state->de.s.e = state->bc.s.b;
}

void mov_ec(State8080 *state, uint8_t *opcode)
{
    state->de.s.e = state->bc.s.c;
}

void mov_ed(State8080 *state, uint8_t *opcode)
{
    state->de.s.e = state->de.s.d;
}

void mov_ee(State8080 *state, uint8_t *opcode)
{
    state->de.s.e = state->de.s.e;
}

void mov_eh(State8080 *state, uint8_t *opcode)
{
    state->de.s.e = state->hl.s.h;
}

void mov_el(State8080 *state, uint8_t *opcode)
{
    state->de.s.e = state->hl.s.l;
}

void mov_em(State8080 *state, uint8_t *opcode)
{
    state->de.s.e = state->memory[state->hl.hl];
}

void mov_ea(State8080 *state, uint8_t *opcode)
{
    state->de.s.e = state->af.s.a;
}

void mov_hb(State8080 *state, uint8_t *opcode)
{
    state->hl.s.h = state->bc.s.b;
}

void mov_hc(State8080 *state, uint8_t *opcode)
{
    state->hl.s.h = state->bc.s.c;
}

void mov_hd(State8080 *state, uint8_t *opcode)
{
    state->hl.s.h = state->de.s.d;
}

void mov_he(State8080 *state, uint8_t *opcode)
{
    state->hl.s.h = state->de.s.e;
}

void mov_hh(State8080 *state, uint8_t *opcode)
{
    state->hl.s.h = state->hl.s.h;
}

void mov_hl(State8080 *state, uint8_t *opcode)
{
    state->hl.s.h = state->hl.s.l;
}

void mov_hm(State8080 *state, uint8_t *opcode)
{
    state->hl.s.h = state->memory[state->hl.hl];
}

void mov_ha(State8080 *state, uint8_t *opcode)
{
    state->hl.s.h = state->af.s.a;
}

void mov_lb(State8080 *state, uint8_t *opcode)
{
    state->hl.s.l = state->bc.s.b;
}

void mov_lc(State8080 *state, uint8_t *opcode)
{
    state->hl.s.l = state->bc.s.c;
}

void mov_ld(State8080 *state, uint8_t *opcode)
{
    state->hl.s.l = state->de.s.d;
}

void mov_le(State8080 *state, uint8_t *opcode)
{
    state->hl.s.l = state->de.s.e;
}

void mov_lh(State8080 *state, uint8_t *opcode)
{
    state->hl.s.l = state->hl.s.h;
}

void mov_ll(State8080 *state, uint8_t *opcode)
{
    state->hl.s.l = state->hl.s.l;
}

void mov_lm(State8080 *state, uint8_t *opcode)
{
    state->hl.s.l = state->memory[state->hl.hl];
}

void mov_la(State8080 *state, uint8_t *opcode)
{
    state->hl.s.l = state->af.s.a;
}

void mov_mb(State8080 *state, uint8_t *opcode)
{
    state->memory[state->hl.hl] = state->bc.s.b;
}

void mov_mc(State8080 *state, uint8_t *opcode)
{
    state->memory[state->hl.hl] = state->bc.s.c;
}

void mov_md(State8080 *state, uint8_t *opcode)
{
    state->memory[state->hl.hl] = state->de.s.d;
}

void mov_me(State8080 *state, uint8_t *opcode)
{
    state->memory[state->hl.hl] = state->de.s.e;
}

void mov_mh(State8080 *state, uint8_t *opcode)
{
    state->memory[state->hl.hl] = state->hl.s.h;
}

void mov_ml(State8080 *state, uint8_t *opcode)
{
    state->memory[state->hl.hl] = state->hl.s.l;
}

void hlt(State8080 *state, uint8_t *opcode)
{
    state->running = false;
}

void mov_ma(State8080 *state, uint8_t *opcode)
{
    state->memory[state->hl.hl] = state->af.s.a;
}

void mov_ab(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = state->bc.s.b;
}

void mov_ac(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = state->bc.s.c;
}

void mov_ad(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = state->de.s.d;
}

void mov_ae(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = state->de.s.e;
}

void mov_ah(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = state->hl.s.h;
}

void mov_al(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = state->hl.s.l;
}

void mov_am(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = state->memory[state->hl.hl];
}

void mov_aa(State8080 *state, uint8_t *opcode)
{
    state->af.s.a = state->af.s.a;
}

void add_b(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a += state->bc.s.b);
}

void add_c(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a += state->bc.s.c);
}

void add_d(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a += state->de.s.d);
}

void add_e(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a += state->de.s.e);
}

void add_h(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a += state->hl.s.h);
}

void add_l(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a += state->hl.s.l);
}

void add_m(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a += state->memory[state->hl.hl]);
}

void add_a(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a += state->af.s.a);
}

void adc_b(State8080 *state, uint8_t *opcode)
{
    state->af.s.a += state->bc.s.b + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void adc_c(State8080 *state, uint8_t *opcode)
{
    state->af.s.a += state->bc.s.c + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void adc_d(State8080 *state, uint8_t *opcode)
{
    state->af.s.a += state->de.s.d + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void adc_e(State8080 *state, uint8_t *opcode)
{
    state->af.s.a += state->de.s.e + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void adc_h(State8080 *state, uint8_t *opcode)
{
    state->af.s.a += state->hl.s.h + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void adc_l(State8080 *state, uint8_t *opcode)
{
    state->af.s.a += state->hl.s.l + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void adc_m(State8080 *state, uint8_t *opcode)
{
    state->af.s.a += state->memory[state->hl.hl] + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void adc_a(State8080 *state, uint8_t *opcode)
{
    state->af.s.a += state->af.s.a + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void sub_b(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a -= state->bc.s.b);
}

void sub_c(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a -= state->bc.s.c);
}

void sub_d(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a -= state->de.s.d);
}

void sub_e(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a -= state->de.s.e);
}

void sub_h(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a -= state->hl.s.h);
}

void sub_l(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a -= state->hl.s.l);
}

void sub_m(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a -= state->memory[state->hl.hl]);
}

void sub_a(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a -= state->af.s.a);
}

void sbb_b(State8080 *state, uint8_t *opcode)
{
    state->af.s.a -= state->bc.s.b + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void sbb_c(State8080 *state, uint8_t *opcode)
{
    state->af.s.a -= state->bc.s.c + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void sbb_d(State8080 *state, uint8_t *opcode)
{
    state->af.s.a -= state->de.s.d + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void sbb_e(State8080 *state, uint8_t *opcode)
{
    state->af.s.a -= state->de.s.e + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void sbb_h(State8080 *state, uint8_t *opcode)
{
    state->af.s.a -= state->hl.s.h + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void sbb_l(State8080 *state, uint8_t *opcode)
{
    state->af.s.a -= state->hl.s.l + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void sbb_m(State8080 *state, uint8_t *opcode)
{
    state->af.s.a -= state->memory[state->hl.hl] + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void sbb_a(State8080 *state, uint8_t *opcode)
{
    state->af.s.a -= state->af.s.a + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
}

void ana_b(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a &= state->bc.s.b);
}

void ana_c(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a &= state->bc.s.c);
}

void ana_d(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a &= state->de.s.d);
}

void ana_e(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a &= state->de.s.e);
}

void ana_h(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a &= state->hl.s.h);
}

void ana_l(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a &= state->hl.s.l);
}

void ana_m(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a &= state->memory[state->hl.hl]);
}

void ana_a(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a &= state->af.s.a);
}

void xra_b(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a ^= state->bc.s.b);
}

void xra_c(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a ^= state->bc.s.c);
}

void xra_d(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a ^= state->de.s.d);
}

void xra_e(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a ^= state->de.s.e);
}

void xra_h(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a ^= state->hl.s.h);
}

void xra_l(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a ^= state->hl.s.l);
}

void xra_m(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a ^= state->memory[state->hl.hl]);
}

void xra_a(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a ^= state->af.s.a);
}

void ora_b(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a |= state->bc.s.b);
}

void ora_c(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a |= state->bc.s.c);
}

void ora_d(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a |= state->de.s.d);
}

void ora_e(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a |= state->de.s.e);
}

void ora_h(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a |= state->hl.s.h);
}

void ora_l(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a |= state->hl.s.l);
}

void ora_m(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a |= state->memory[state->hl.hl]);
}

void ora_a(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a |= state->af.s.a);
}

void cmp_b(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a - state->bc.s.b);
}

void cmp_c(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a - state->bc.s.c);
}

void cmp_d(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a - state->de.s.d);
}

void cmp_e(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a - state->de.s.e);
}

void cmp_h(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a - state->hl.s.h);
}

void cmp_l(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a - state->hl.s.l);
}

void cmp_m(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a - state->memory[state->hl.hl]);
}

void cmp_a(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a - state->af.s.a);
}

void rnz(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.z) {
        state->pc = (state->memory[state->sp++]) | (state->memory[state->sp++] << 8);
    } else {
        state->pc += 2;
    }
}

void pop_b(State8080 *state, uint8_t *opcode)
{
    state->bc.s.c = state->memory[state->sp++];
    state->bc.s.b = state->memory[state->sp++];
}

void jnz_addr(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.z) {
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void jmp_addr(State8080 *state, uint8_t *opcode)
{
    state->pc = (opcode[1]) | (opcode[2] << 8);
}

void cnz_addr(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.z) {
        uint16_t ret = state->pc + 2;
        state->memory[--state->sp] = ((ret >> 8) & 0xFF);
        state->memory[--state->sp] = (ret & 0xFF);
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void push_b(State8080 *state, uint8_t *opcode)
{
    state->memory[--state->sp] = state->bc.s.b;
    state->memory[--state->sp] = state->bc.s.c;
}

void adi(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a += opcode[1]);
    state->pc += 1;
}

void rst_0(State8080 *state, uint8_t *opcode)
{
}

void rz(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.z) {
        state->pc = (state->memory[state->sp++]) | (state->memory[state->sp++] << 8);
    } else {
        state->pc += 2;
    }
}

void ret(State8080 *state, uint8_t *opcode)
{
    state->pc = (state->memory[state->sp++]) | (state->memory[state->sp++] << 8);
}

void jz_addr(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.z) {
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void cz_addr(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.z) {
        uint16_t ret = state->pc + 2;
        state->memory[--state->sp] = ((ret >> 8) & 0xFF);
        state->memory[--state->sp] = (ret & 0xFF);
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void call_addr(State8080 *state, uint8_t *opcode)
{
    uint16_t ret = state->pc + 2;
    state->memory[--state->sp] = ((ret >> 8) & 0xFF);
    state->memory[--state->sp] = (ret & 0xFF);
    state->pc = (opcode[1]) | (opcode[2] << 8);
}

void aci(State8080 *state, uint8_t *opcode)
{
    state->af.s.a += opcode[1] + state->af.s.cc.cy;
    update_flags(&state->af.s.cc, state->af.s.a);
    state->pc += 1;
}

void rst_1(State8080 *state, uint8_t *opcode)
{
}

void rnc(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.cy) {
        state->pc = (state->memory[state->sp++]) | (state->memory[state->sp++] << 8);
    } else {
        state->pc += 2;
    }
}

void pop_d(State8080 *state, uint8_t *opcode)
{
    state->de.s.e = state->memory[state->sp++];
    state->de.s.d = state->memory[state->sp++];
}

void jnc_addr(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.cy) {
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void out(State8080 *state, uint8_t *opcode)
{
    /// TODO
    state->pc += 1;
}

void cnc_addr(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.cy) {
        uint16_t ret = state->pc + 2;
        state->memory[--state->sp] = ((ret >> 8) & 0xFF);
        state->memory[--state->sp] = (ret & 0xFF);
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void push_d(State8080 *state, uint8_t *opcode)
{
    state->memory[--state->sp] = state->de.s.d;
    state->memory[--state->sp] = state->de.s.e;
}

void sui(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a -= opcode[1]);
    state->pc += 1;
}

void rst_2(State8080 *state, uint8_t *opcode)
{
}

void rc(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.cy) {
        state->pc = (state->memory[state->sp++]) | (state->memory[state->sp++] << 8);
    } else {
        state->pc += 2;
    }
}

void jc_addr(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.cy) {
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void in(State8080 *state, uint8_t *opcode)
{
    /// TODO
    state->pc += 1;
}

void cc_addr(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.cy) {
        uint16_t ret = state->pc + 2;
        state->memory[--state->sp] = ((ret >> 8) & 0xFF);
        state->memory[--state->sp] = (ret & 0xFF);
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void sbi(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a -= opcode[1] + state->af.s.cc.cy);
    state->pc += 1;
}

void rst_3(State8080 *state, uint8_t *opcode)
{
}

void rpo(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.p) {
        state->pc = (state->memory[state->sp++]) | (state->memory[state->sp++] << 8);
    } else {
        state->pc += 2;
    }
}

void pop_h(State8080 *state, uint8_t *opcode)
{
    state->hl.s.l = state->memory[state->sp++];
    state->hl.s.h = state->memory[state->sp++];
}

void jpo_addr(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.p) {
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void xthl(State8080 *state, uint8_t *opcode)
{
    uint16_t stop = (state->memory[state->sp++]) | (state->memory[state->sp] << 8);
    state->memory[state->sp--] = state->hl.s.h;
    state->memory[state->sp] = state->hl.s.l;
    state->hl.hl = stop;
}

void cpo_addr(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.p) {
        uint16_t ret = state->pc + 2;
        state->memory[--state->sp] = ((ret >> 8) & 0xFF);
        state->memory[--state->sp] = (ret & 0xFF);
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void push_h(State8080 *state, uint8_t *opcode)
{
    state->memory[--state->sp] = state->hl.s.h;
    state->memory[--state->sp] = state->hl.s.l;
}

void ani(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a &= opcode[1]);
    state->pc += 1;
}

void rst_4(State8080 *state, uint8_t *opcode)
{
}

void rpe(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.p) {
        state->pc = (state->memory[state->sp++]) | (state->memory[state->sp++] << 8);
    } else {
        state->pc += 2;
    }
}

void pchl(State8080 *state, uint8_t *opcode)
{
    state->pc = state->hl.hl;
}

void jpe_addr(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.p) {
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void xchg(State8080 *state, uint8_t *opcode)
{
    uint16_t tmp = state->de.de;
    state->de.de = state->hl.hl;
    state->hl.hl = tmp;
}

void cpe_addr(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.p) {
        uint16_t ret = state->pc + 2;
        state->memory[--state->sp] = ((ret >> 8) & 0xFF);
        state->memory[--state->sp] = (ret & 0xFF);
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void xri(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a ^= opcode[1]);
    state->pc += 1;
}

void rst_5(State8080 *state, uint8_t *opcode)
{
}

void rp(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.s && !state->af.s.cc.z) {
        state->pc = (state->memory[state->sp++]) | (state->memory[state->sp++] << 8);
    } else {
        state->pc += 2;
    }
}

void pop_psw(State8080 *state, uint8_t *opcode)
{
    state->af.af = (state->memory[state->sp++]) | (state->memory[state->sp++] << 8);
}

void jp_addr(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.s && !state->af.s.cc.z) {
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void di(State8080 *state, uint8_t *opcode)
{
    state->int_enable = false;
}

void cp_addr(State8080 *state, uint8_t *opcode)
{
    if (!state->af.s.cc.s && !state->af.s.cc.z) {
        uint16_t ret = state->pc + 2;
        state->memory[--state->sp] = ((ret >> 8) & 0xFF);
        state->memory[--state->sp] = (ret & 0xFF);
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void push_psw(State8080 *state, uint8_t *opcode)
{
    state->memory[--state->sp] = *((uint8_t *)&state->af.s.cc);
    state->memory[--state->sp] = state->af.s.a;
}

void ori(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a |= opcode[1]);
    state->pc += 1;
}

void rst_6(State8080 *state, uint8_t *opcode)
{
}

void rm(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.s) {
        state->pc = (state->memory[state->sp++]) | (state->memory[state->sp++] << 8);
    } else {
        state->pc += 2;
    }
}

void sphl(State8080 *state, uint8_t *opcode)
{
    state->sp = state->hl.hl;
}

void jm_addr(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.s) {
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void ei(State8080 *state, uint8_t *opcode)
{
    state->int_enable = true;
}

void cm_addr(State8080 *state, uint8_t *opcode)
{
    if (state->af.s.cc.s) {
        uint16_t ret = state->pc + 2;
        state->memory[--state->sp] = ((ret >> 8) & 0xFF);
        state->memory[--state->sp] = (ret & 0xFF);
        state->pc = (opcode[1]) | (opcode[2] << 8);
    } else {
        state->pc += 2;
    }
}

void cpi(State8080 *state, uint8_t *opcode)
{
    update_flags(&state->af.s.cc, state->af.s.a - opcode[1]);
    state->pc += 1;
}

void rst_7(State8080 *state, uint8_t *opcode)
{
}

static void (*instructions[256])(State8080 *state, uint8_t *opcode) = {
    nop, /*00*/

    lxi_bc, /*01*/
    stax_b, /*02*/
    inx_b, /*03*/
    inr_b, /*04*/
    dcr_b, /*05*/
    mvi_b, /*06*/
    rlc, /*07*/

    nop, /*08*/

    dad_b, /*09*/
    ldax_b, /*0A*/
    dcx_b, /*0B*/
    inr_c, /*0C*/
    dcr_c, /*0D*/
    mvi_c, /*0E*/
    rrc, /*0F*/

    nop, /*10*/

    lxi_de, /*11*/
    stax_d, /*12*/
    inx_d, /*13*/
    inr_d, /*14*/
    dcr_d, /*15*/
    mvi_d, /*16*/
    ral, /*17*/

    nop, /*18*/

    dad_d, /*19*/
    ldax_d, /*1A*/
    dcx_d, /*1B*/
    inr_e, /*1C*/
    dcr_e, /*1D*/
    mvi_e, /*1E*/
    rar, /*1F*/

    nop, /*20*/

    lxi_hl, /*21*/
    shld_addr, /*22*/
    inx_h, /*23*/
    inr_h, /*24*/
    dcr_h, /*25*/
    mvi_h, /*26*/
    daa, /*27*/

    nop, /*28*/

    dad_h, /*29*/
    lhld_addr, /*2A*/
    dcx_h, /*2B*/
    inr_l, /*2C*/
    dcr_l, /*2D*/
    mvi_l, /*2E*/
    cma, /*2F*/

    nop, /*30*/

    lxi_sp, /*31*/
    sta_addr, /*32*/
    inx_sp, /*33*/
    inr_m, /*34*/
    dcr_m, /*35*/
    mvi_m, /*36*/
    stc, /*37*/

    nop, /*38*/

    dad_sp, /*39*/
    lda_addr, /*3A*/
    dcx_sp, /*3B*/
    inr_a, /*3C*/
    dcr_a, /*3D*/
    mvi_a, /*3E*/
    cmc, /*3F*/
    mov_bb, /*40*/
    mov_bc, /*41*/
    mov_bd, /*42*/
    mov_be, /*43*/
    mov_bh, /*44*/
    mov_bl, /*45*/
    mov_bm, /*46*/
    mov_ba, /*47*/
    mov_cb, /*48*/
    mov_cc, /*49*/
    mov_cd, /*4A*/
    mov_ce, /*4B*/
    mov_ch, /*4C*/
    mov_cl, /*4D*/
    mov_cm, /*4E*/
    mov_ca, /*4F*/
    mov_db, /*50*/
    mov_dc, /*51*/
    mov_dd, /*52*/
    mov_de, /*53*/
    mov_dh, /*54*/
    mov_dl, /*55*/
    mov_dm, /*56*/
    mov_da, /*57*/
    mov_eb, /*58*/
    mov_ec, /*59*/
    mov_ed, /*5A*/
    mov_ee, /*5B*/
    mov_eh, /*5C*/
    mov_el, /*5D*/
    mov_em, /*5E*/
    mov_ea, /*5F*/
    mov_hb, /*60*/
    mov_hc, /*61*/
    mov_hd, /*62*/
    mov_he, /*63*/
    mov_hh, /*64*/
    mov_hl, /*65*/
    mov_hm, /*66*/
    mov_ha, /*67*/
    mov_lb, /*68*/
    mov_lc, /*69*/
    mov_ld, /*6A*/
    mov_le, /*6B*/
    mov_lh, /*6C*/
    mov_ll, /*6D*/
    mov_lm, /*6E*/
    mov_la, /*6F*/
    mov_mb, /*70*/
    mov_mc, /*71*/
    mov_md, /*72*/
    mov_me, /*73*/
    mov_mh, /*74*/
    mov_ml, /*75*/
    hlt, /*76*/
    mov_ma, /*77*/
    mov_ab, /*78*/
    mov_ac, /*79*/
    mov_ad, /*7A*/
    mov_ae, /*7B*/
    mov_ah, /*7C*/
    mov_al, /*7D*/
    mov_am, /*7E*/
    mov_aa, /*7F*/
    add_b, /*80*/
    add_c, /*81*/
    add_d, /*82*/
    add_e, /*83*/
    add_h, /*84*/
    add_l, /*85*/
    add_m, /*86*/
    add_a, /*87*/
    adc_b, /*88*/
    adc_c, /*89*/
    adc_d, /*8A*/
    adc_e, /*8B*/
    adc_h, /*8C*/
    adc_l, /*8D*/
    adc_m, /*8E*/
    adc_a, /*8F*/
    sub_b, /*90*/
    sub_c, /*91*/
    sub_d, /*92*/
    sub_e, /*93*/
    sub_h, /*94*/
    sub_l, /*95*/
    sub_m, /*96*/
    sub_a, /*97*/
    sbb_b, /*98*/
    sbb_c, /*99*/
    sbb_d, /*9A*/
    sbb_e, /*9B*/
    sbb_h, /*9C*/
    sbb_l, /*9D*/
    sbb_m, /*9E*/
    sbb_a, /*9F*/
    ana_b, /*A0*/
    ana_c, /*A1*/
    ana_d, /*A2*/
    ana_e, /*A3*/
    ana_h, /*A4*/
    ana_l, /*A5*/
    ana_m, /*A6*/
    ana_a, /*A7*/
    xra_b, /*A8*/
    xra_c, /*A9*/
    xra_d, /*AA*/
    xra_e, /*AB*/
    xra_h, /*AC*/
    xra_l, /*AD*/
    xra_m, /*AE*/
    xra_a, /*AF*/
    ora_b, /*B0*/
    ora_c, /*B1*/
    ora_d, /*B2*/
    ora_e, /*B3*/
    ora_h, /*B4*/
    ora_l, /*B5*/
    ora_m, /*B6*/
    ora_a, /*B7*/
    cmp_b, /*B8*/
    cmp_c, /*B9*/
    cmp_d, /*BA*/
    cmp_e, /*BB*/
    cmp_h, /*BC*/
    cmp_l, /*BD*/
    cmp_m, /*BE*/
    cmp_a, /*BF*/
    rnz, /*C0*/
    pop_b, /*C1*/
    jnz_addr, /*C2*/
    jmp_addr, /*C3*/
    cnz_addr, /*C4*/
    push_b, /*C5*/
    adi, /*C6*/
    rst_0, /*C7*/
    rz, /*C8*/
    ret, /*C9*/
    jz_addr, /*CA*/

    nop, /*CB*/

    cz_addr, /*CC*/
    call_addr, /*CD*/
    aci, /*CE*/
    rst_1, /*CF*/
    rnc, /*D0*/
    pop_d, /*D1*/
    jnc_addr, /*D2*/
    out, /*D3*/
    cnc_addr, /*D4*/
    push_d, /*D5*/
    sui, /*D6*/
    rst_2, /*D7*/
    rc, /*D8*/

    nop, /*D9*/

    jc_addr, /*DA*/
    in, /*DB*/
    cc_addr, /*DC*/

    nop, /*DD*/

    sbi, /*DE*/
    rst_3, /*DF*/
    rpo, /*E0*/
    pop_h, /*E1*/
    jpo_addr, /*E2*/
    xthl, /*E3*/
    cpo_addr, /*E4*/
    push_h, /*E5*/
    ani, /*E6*/
    rst_4, /*E7*/
    rpe, /*E8*/
    pchl, /*E9*/
    jpe_addr, /*EA*/
    xchg, /*EB*/
    cpe_addr, /*EC*/

    nop, /*ED*/

    xri, /*EE*/
    rst_5, /*EF*/
    rp, /*F0*/
    pop_psw, /*F1*/
    jp_addr, /*F2*/
    di, /*F3*/
    cp_addr, /*F4*/
    push_psw, /*F5*/
    ori, /*F6*/
    rst_6, /*F7*/
    rm, /*F8*/
    sphl, /*F9*/
    jm_addr, /*FA*/
    ei, /*FB*/
    cm_addr, /*FC*/

    nop, /*FD*/

    cpi, /*FE*/
    rst_7 /*FF*/
};

void load_rom(State8080 *state, FILE *fp)
{
    fseek(fp, 0L, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    long read = fread(&state->pc, sizeof(uint16_t), 1, fp);
    read = fread(state->memory + state->pc, sizeof(uint8_t), fsize - read, fp);
}

void print_regs(State8080 *state)
{
    printf("a = %u ", state->af.s.a);
    printf("b = %u ", state->bc.s.b);
    printf("c = %u ", state->bc.s.c);
    printf("d = %u ", state->de.s.d);
    printf("e = %u ", state->de.s.e);
    printf("sp = %u ", state->sp);
    printf("pc = %u\n", state->pc);
    printf("flags:");
    printf("z = %u ", state->af.s.cc.z);
    printf("s = %u ", state->af.s.cc.s);
    printf("cy = %u ", state->af.s.cc.cy);
    printf("ac = %u ", state->af.s.cc.ac);
    printf("p = %u\n", state->af.s.cc.p);
}

void emulate8080(State8080 *state)
{
    state->running = true;
    while (state->running) {
        uint8_t *opcode = &state->memory[state->pc++];
        printf("opcode = %x\n", *opcode);
        instructions[*opcode](state, opcode);
    }
}

int main(int argc, char *argv[])
{

    /// TEST - unsigned multiplication program
    State8080 *state = calloc(1, sizeof(State8080));

    state->pc = 0;

    state->memory[0] = 0x06;
    state->memory[1] = 0x02;
    state->memory[2] = 0x0E;
    state->memory[3] = 0x03;
    state->memory[4] = 0x3E;
    state->memory[5] = 0x00;
    state->memory[6] = 0x57;
    state->memory[7] = 0x78;
    state->memory[8] = 0xFE;
    state->memory[9] = 0x00;
    state->memory[10] = 0xCA;
    state->memory[11] = 0x15;
    state->memory[12] = 0x00;
    state->memory[13] = 0xDE;
    state->memory[14] = 0x01;
    state->memory[15] = 0x47;
    state->memory[16] = 0x7A;
    state->memory[17] = 0x81;
    state->memory[18] = 0xC3;
    state->memory[19] = 0x06;
    state->memory[20] = 0x00;
    state->memory[21] = 0x7A;
    state->memory[22] = 0x76;

    emulate8080(state);

    print_regs(state);

    free(state);

    return 0;
}
