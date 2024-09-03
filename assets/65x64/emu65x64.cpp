//==============================================================================
//                                          .ooooo.     .o      .ooo
//                                         d88'   `8. o888    .88'
//  .ooooo.  ooo. .oo.  .oo.   oooo  oooo  Y88..  .8'  888   d88'
// d88' `88b `888P"Y88bP"Y88b  `888  `888   `88888b.   888  d888P"Ybo.
// 888ooo888  888   888   888   888   888  .8'  ``88b  888  Y88[   ]88
// 888    .o  888   888   888   888   888  `8.   .88P  888  `Y88   88P
// `Y8bod8P' o888o o888o o888o  `V88V"V8P'  `boood8'  o888o  `88bod8'
//
// A Portable C++ Nozotech 65x64 Emulator
//------------------------------------------------------------------------------
// 2024 KyokoToreno
//
// Based on work by:
//
// Copyright (C),2016 Andrew John Jacobs
// All rights reserved.
//
// This work is made available under the terms of the Creative Commons
// Attribution-NonCommercial-ShareAlike 4.0 International license. Open the
// following URL to see the details.
//
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//------------------------------------------------------------------------------

#include "emu65x64.hpp"

union emu65x64::FLAGS		emu65x64::p;

union emu65x64::REGS		emu65x64::a;
union emu65x64::REGS		emu65x64::x;
union emu65x64::REGS		emu65x64::y;
union emu65x64::REGS		emu65x64::sp;
union emu65x64::REGS		emu65x64::z[128];

emu65x64::QWord			emu65x64::pc, emu65x64::tpc;

emu65x64::Byte			emu65x64::cop_size;
emu65x64::Byte			emu65x64::cop_op;
emu65x64::Word		   *emu65x64::cop;
bool					emu65x64::stopped;
emu65x64::StopReason	emu65x64::stop_reason;
bool					emu65x64::interrupted;
unsigned long			emu65x64::cycles;
bool					emu65x64::trace;

// Convert a value to a hex string
char *emu65x64::toHex(unsigned long long value, unsigned int digits)
{
	static char buffer[32];
	unsigned int offset = sizeof(buffer);

	buffer[--offset] = 0;
	while (digits-- > 0) {
		buffer[--offset] = "0123456789ABCDEF"[value & 0xf];
		value >>= 4;
	}
	return (&(buffer[offset]));
}

//==============================================================================

// Not used.
emu65x64::emu65x64()
{ }

// Not used
emu65x64::~emu65x64()
{ }

// Reset the state of emulator
void emu65x64::reset(bool trace)
{
	sp.w = 0x100;
	tpc = pc = getWord(0xfffc);
	p.b = 0x34;

	stopped = false;
	stop_reason = StopReason::RUNNING;
	interrupted = false;

	emu65x64::trace = trace;
}

// Execute a single instruction or invoke an interrupt
void emu65x64::step()
{
    QWord tpc = pc;
    pc += 8;

	// Check for NMI/IRQ

	SHOWPC();

	if (interrupted) {
		interrupted = false;
		stop_reason = StopReason::RUNNING;

		if (stop_reason == StopReason::WAIT && p.f_i == 1) {
			//pc++;
		} else if (p.f_i == 0) {
            pushQWord(pc);
            pushQWord(p.q);

            p.f_i = 1;
            p.f_d = 0;

            pc = getQWord(0xffee);
            cycles += 8; //TODO: FIXME
		}
	}

    for (Byte i = 0; i < 4; i++) {
        Word in = getWord(tpc + (i*2));
        Word op = in & 0x3ff;
        Size size = (Size)((in >> 10) & 0x3);

        switch (op) {
        case 0x00:	op_brk(am_immb());	break;
        /*case 0x01:	op_ora(am_dpix());	break;
        case 0x02:	op_cop(am_immb());	break;
        case 0x03:	op_ora(am_srel());	break;
        case 0x04:	op_tsb(am_dpag());	break;
        case 0x05:	op_ora(am_dpag());	break;
        case 0x06:	op_asl(am_dpag());	break;
        case 0x07:	op_ora(am_dpil());	break;
        case 0x08:	op_php(am_impl());	break;
        case 0x09:	op_ora(am_immm());	break;
        case 0x0a:	op_asla(am_acc());	break;
        case 0x0b:	op_phd(am_impl());	break;
        case 0x0c:	op_tsb(am_absl());	break;
        case 0x0d:	op_ora(am_absl());	break;
        case 0x0e:	op_asl(am_absl());	break;
        case 0x0f:	op_ora(am_alng());	break;

        case 0x10:	op_bpl(am_rela());	break;
        case 0x11:	op_ora(am_dpiy());	break;
        case 0x12:	op_ora(am_dpgi());	break;
        case 0x13:	op_ora(am_sriy());	break;
        case 0x14:	op_trb(am_dpag());	break;
        case 0x15:	op_ora(am_dpgx());	break;
        case 0x16:	op_asl(am_dpgx());	break;
        case 0x17:	op_ora(am_dily());	break;
        case 0x18:	op_clc(am_impl());	break;
        case 0x19:	op_ora(am_absy());	break;
        case 0x1a:	op_inca(am_acc());	break;
        case 0x1b:	op_tcs(am_impl());	break;
        case 0x1c:	op_trb(am_absl());	break;
        case 0x1d:	op_ora(am_absx());	break;
        case 0x1e:	op_asl(am_absx());	break;
        case 0x1f:	op_ora(am_alnx());	break;

        case 0x20:	op_jsr(am_absl());	break;
        case 0x21:	op_and(am_dpix());	break;
        case 0x22:	op_jsl(am_alng());	break;
        case 0x23:	op_and(am_srel());	break;
        case 0x24:	op_bit(am_dpag());	break;
        case 0x25:  op_and(am_dpag());	break;
        case 0x26:	op_rol(am_dpag());	break;
        case 0x27:	op_and(am_dpil());	break;
        case 0x28:	op_plp(am_impl());	break;
        case 0x29:	op_and(am_immm());	break;
        case 0x2a:	op_rola(am_acc());	break;
        case 0x2b:	op_pld(am_impl());	break;
        case 0x2c:	op_bit(am_absl());	break;
        case 0x2d:  op_and(am_absl());	break;
        case 0x2e:	op_rol(am_absl());	break;
        case 0x2f:  op_and(am_alng());	break;

        case 0x30:	op_bmi(am_rela());	break;
        case 0x31: 	op_and(am_dpiy());	break;
        case 0x32: 	op_and(am_dpgi());	break;
        case 0x33: 	op_and(am_sriy());	break;
        case 0x34:	op_bit(am_dpgx());	break;
        case 0x35: 	op_and(am_dpgx());	break;
        case 0x36:	op_rol(am_dpgx());	break;
        case 0x37: 	op_and(am_dily());	break;
        case 0x38:	op_sec(am_impl());	break;
        case 0x39: 	op_and(am_absy());	break;
        case 0x3a:	op_deca(am_acc());	break;
        case 0x3b:	op_tsa(am_impl());	break;
        case 0x3c:	op_bit(am_absx());	break;
        case 0x3d: 	op_and(am_absx());	break;
        case 0x3e:	op_rol(am_absx());	break;
        case 0x3f: 	op_and(am_alnx());	break;

        case 0x40:	op_rti(am_impl());	break;
        case 0x41:	op_eor(am_dpix());	break;
        //case 0x42:	op_wdm(am_immb());	break;
        case 0x43:	op_eor(am_srel());	break;
        case 0x44:	op_mvp(am_immw());	break;
        case 0x45:	op_eor(am_dpag());	break;
        case 0x46:	op_lsr(am_dpag());	break;
        case 0x47:	op_eor(am_dpil());	break;
        case 0x48:	op_pha(am_impl());	break;
        case 0x49:	op_eor(am_immm());	break;
        case 0x4a:	op_lsra(am_impl());	break;
        case 0x4b:	op_phk(am_impl());	break;
        case 0x4c:	op_jmp(am_absl());	break;
        case 0x4d:	op_eor(am_absl());	break;
        case 0x4e:	op_lsr(am_absl());	break;
        case 0x4f:	op_eor(am_alng());	break;

        case 0x50:	op_bvc(am_rela());	break;
        case 0x51:	op_eor(am_dpiy());	break;
        case 0x52:	op_eor(am_dpgi());	break;
        case 0x53:	op_eor(am_sriy());	break;
        case 0x54:	op_mvn(am_immw());	break;
        case 0x55:	op_eor(am_dpgx());	break;*/
        //case 0x56:	op_lsr(am_dpgx());	break;
        //case 0x57:	op_eor(am_dpil());	break;
        case 0x58:	op_cli(am_impl());	break;
        case 0x59:	op_eor(am_absy());	break;
        case 0x5a:	op_phy(am_impl());	break;
        case 0x5b:	op_tcd(am_impl());	break;
        //case 0x5c:	op_jmp(am_alng());	break;
        case 0x5d:	op_eor(am_absx());	break;
        case 0x5e:	op_lsr(am_absx());	break;
        //case 0x5f:	op_eor(am_alnx());	break;

        case 0x60:	op_rts(am_impl());	break;
       /* case 0x61:	op_adc(am_dpix());	break;
        case 0x62:	op_per(am_lrel());	break;
        case 0x63:	op_adc(am_srel());	break;
        case 0x64:	op_stz(am_dpag());	break;
        case 0x65:	op_adc(am_dpag());	break;
        case 0x66:	op_ror(am_dpag());	break;
        case 0x67:	op_adc(am_dpil());	break;
        case 0x68:	op_pla(am_impl());	break;
        case 0x69:	op_adc(am_immm());	break;
        case 0x6a:	op_rora(am_impl());	break;
        case 0x6b:	op_rtl(am_impl());	break;
        case 0x6c:	op_jmp(am_absi());	break;
        case 0x6d:	op_adc(am_absl());	break;
        case 0x6e:	op_ror(am_absl());	break;
        case 0x6f:	op_adc(am_alng());	break;

        case 0x70:	op_bvs(am_rela());	break;
        case 0x71:	op_adc(am_dpiy());	break;
        case 0x72:	op_adc(am_dpgi());	break;
        case 0x73:	op_adc(am_sriy());	break;
        case 0x74:	op_stz(am_dpgx());	break;
        case 0x75:	op_adc(am_dpgx());	break;
        case 0x76:	op_ror(am_dpgx());	break;
        case 0x77:	op_adc(am_dily());	break;
        case 0x78:	op_sei(am_impl());	break;
        case 0x79:	op_adc(am_absy());	break;
        case 0x7a:	op_ply(am_impl());	break;
        case 0x7b:	op_tdc(am_impl());	break;
        case 0x7c:	op_jmp(am_abxi());	break;
        case 0x7d:	op_adc(am_absx());	break;
        case 0x7e:	op_ror(am_absx());	break;
        case 0x7f:	op_adc(am_alnx());	break;

        case 0x80:	op_bra(am_rela());	break;
        case 0x81:	op_sta(am_dpix());	break;
        case 0x82:	op_brl(am_lrel());	break;*/
        case 0x83:	op_sta(am_srel());	break;
        //case 0x84:	op_sty(am_dpag());	break;
        //case 0x85:	op_sta(am_dpag());	break;
        //case 0x86:	op_stx(am_dpag());	break;
        //case 0x87:	op_sta(am_dpil());	break;
        case 0x88:	op_dey(am_impl());	break;
        //case 0x89:	op_biti(am_immm());	break;
        case 0x8a:	op_txa(am_impl());	break;
        case 0x8b:	op_phb(am_impl());	break;
        case 0x8c:	op_sty(am_absl());	break;
        case 0x8d:	op_sta(am_absl());	break;
        case 0x8e:	op_stx(am_absl());	break;
        //case 0x8f:	op_sta(am_alng());	break;

        case 0x90:	op_bcc(am_rela());	break;
        //case 0x91:	op_sta(am_dpiy());	break;
        //case 0x92:	op_sta(am_dpgi());	break;
        case 0x93:	op_sta(am_sriy());	break;
        //case 0x94:	op_sty(am_dpgx());	break;
        //case 0x95:	op_sta(am_dpgx());	break;
        //case 0x96:	op_stx(am_dpgy());	break;
        //case 0x97:	op_sta(am_dily());	break;
        case 0x98:	op_tya(am_impl());	break;
        case 0x99:	op_sta(am_absy());	break;
        case 0x9a:	op_txs(am_impl());	break;
        case 0x9b:	op_txy(am_impl());	break;
        case 0x9c:	op_stz(am_absl());	break;
        case 0x9d:	op_sta(am_absx());	break;
        case 0x9e:	op_stz(am_absx());	break;
        //case 0x9f:	op_sta(am_alnx());	break;

        //case 0xa0:	op_ldy(am_immx());	break;
        //case 0xa1:	op_lda(am_dpix());	break;
        //case 0xa2:	op_ldx(am_immx());	break;
        case 0xa3:	op_lda(am_srel());	break;
        //case 0xa4:	op_ldy(am_dpag());	break;
        //case 0xa5:	op_lda(am_dpag());	break;
        //case 0xa6:	op_ldx(am_dpag());	break;
        //case 0xa7:	op_lda(am_dpil());	break;
        case 0xa8:	op_tay(am_impl());	break;
        //case 0xa9:	op_lda(am_immm());	break;
        case 0xaa:	op_tax(am_impl());	break;
        case 0xab:	op_plb(am_impl());	break;
        case 0xac:	op_ldy(am_absl());	break;
        case 0xad:	op_lda(am_absl());	break;
        case 0xae:	op_ldx(am_absl());	break;
        //case 0xaf:	op_lda(am_alng());	break;

        case 0xb0:	op_bcs(am_rela());	break;
        //case 0xb1:	op_lda(am_dpiy());	break;
        //case 0xb2:	op_lda(am_dpgi());	break;
        case 0xb3:	op_lda(am_sriy());	break;
        //case 0xb4:	op_ldy(am_dpgx());	break;
        //case 0xb5:	op_lda(am_dpgx());	break;
        //case 0xb6:	op_ldx(am_dpgy());	break;
        //case 0xb7:	op_lda(am_dily());	break;
        case 0xb8:	op_clv(am_impl());	break;
        case 0xb9:	op_lda(am_absy());	break;
        case 0xba:	op_tsx(am_impl());	break;
        case 0xbb:	op_tyx(am_impl());	break;
        case 0xbc:	op_ldy(am_absx());	break;
        case 0xbd:	op_lda(am_absx());	break;
        case 0xbe:	op_ldx(am_absy());	break;
        //case 0xbf:	op_lda(am_alnx());	break;

        //case 0xc0:	op_cpy(am_immx());	break;
        //case 0xc1:	op_cmp(am_dpix());	break;
        case 0xc2:	op_rep(am_immb());	break;
        case 0xc3:	op_cmp(am_srel());	break;
        //case 0xc4:	op_cpy(am_dpag());	break;
        //case 0xc5:	op_cmp(am_dpag());	break;
        //case 0xc6:	op_dec(am_dpag());	break;
        //case 0xc7:	op_cmp(am_dpil());	break;
        case 0xc8:	op_iny(am_impl());	break;
        //case 0xc9:	op_cmp(am_immm());	break;
        case 0xca:	op_dex(am_impl());	break;
        case 0xcb:	op_wai(am_impl());	break;
        case 0xcc:	op_cpy(am_absl());	break;
        case 0xcd:	op_cmp(am_absl());	break;
        case 0xce:	op_dec(am_absl());	break;
        //case 0xcf:	op_cmp(am_alng());	break;

        case 0xd0:	op_bne(am_rela());	break;
        //case 0xd1:	op_cmp(am_dpiy());	break;
        //case 0xd2:	op_cmp(am_dpgi());	break;
        case 0xd3:	op_cmp(am_sriy());	break;
        //case 0xd4:	op_pei(am_dpag());	break;
        //case 0xd5:	op_cmp(am_dpgx());	break;
        //case 0xd6:	op_dec(am_dpgx());	break;
        //case 0xd7:	op_cmp(am_dily());	break;
        case 0xd8:	op_cld(am_impl());	break;
        case 0xd9:	op_cmp(am_absy());	break;
        case 0xda:	op_phx(am_impl());	break;
        case 0xdb:	op_stp(am_impl());	break;
        //case 0xdc:	op_jmp(am_abil());	break;
        case 0xdd:	op_cmp(am_absx());	break;
        case 0xde:	op_dec(am_absx());	break;
        //case 0xdf:	op_cmp(am_alnx());	break;

        //case 0xe0:	op_cpx(am_immx());	break;
        //case 0xe1:	op_sbc(am_dpix());	break;
        case 0xe2:	op_sep(am_immb());	break;
        case 0xe3:	op_sbc(am_srel());	break;
        //case 0xe4:	op_cpx(am_dpag());	break;
        //case 0xe5:	op_sbc(am_dpag());	break;
        //case 0xe6:	op_inc(am_dpag());	break;
        //case 0xe7:	op_sbc(am_dpil());	break;
        case 0xe8:	op_inx(am_impl());	break;
        //case 0xe9:	op_sbc(am_immm());	break;
        case 0xea:	op_nop(am_impl());	break;
        //case 0xeb:	op_xba(am_impl());	break;
        case 0xec:	op_cpx(am_absl());	break;
        case 0xed:	op_sbc(am_absl());	break;
        case 0xee:	op_inc(am_absl());	break;
        //case 0xef:	op_sbc(am_alng());	break;

        case 0xf0:	op_beq(am_rela());	break;
        //case 0xf1:	op_sbc(am_dpiy());	break;
        //case 0xf2:	op_sbc(am_dpgi());	break;
        case 0xf3:	op_sbc(am_sriy());	break;
        case 0xf4:	op_pea(am_immw());	break;
        //case 0xf5:	op_sbc(am_dpgx());	break;
        //case 0xf6:	op_inc(am_dpgx());	break;
        //case 0xf7:	op_sbc(am_dily());	break;
        case 0xf8:	op_sed(am_impl());	break;
        case 0xf9:	op_sbc(am_absy());	break;
        case 0xfa:	op_plx(am_impl());	break;
        //case 0xfb:	op_xce(am_impl());	break;
        case 0xfc:	op_jsr(am_abxi());	break;
        case 0xfd:	op_sbc(am_absx());	break;
        case 0xfe:	op_inc(am_absx());	break;
        //case 0xff:	op_sbc(am_alnx());	break;
        }
    }
}

//==============================================================================
// Debugging Utilities
//------------------------------------------------------------------------------

// The current PC and opcode quad word
void emu65x64::show()
{
//	cout << '{' << toHex(cycles, 4) << "} ";
    std::cout << toHex(pc, 16);
    std::cout << ' ' << toHex(getQWord(pc), 16);
    std::cout << std::endl;
}

// Display the operand bytes
void emu65x64::bytes(unsigned int count)
{
    for (unsigned int i = 0; i < count; i++) {
        std::cout << ' ' << toHex(getByte(pc + i), 2);
    }

    std::cout << "|";
}

// Display registers and top of stack
void emu65x64::dump(const char *mnem, QWord ea)
{
	std::cout << "{";
	std::cout << toHex(ea, 16) << '}' << mnem;

	std::cout << " P=" <<
		(p.f_n ? 'N' : '.') <<
		(p.f_v ? 'V' : '.') <<
		(p.f_p ? 'P' : '.') <<
		(p.f_t ? 'T' : '.') <<
		(p.f_d ? 'D' : '.') <<
		(p.f_i ? 'I' : '.') <<
		(p.f_z ? 'Z' : '.') <<
		(p.f_c ? 'C' : '.');
	std::cout << " A=";
		std::cout << '[' << toHex(hi(a.q), 16) << ']';
    std::cout << " B=";
		std::cout << '[' << toHex(hi(a.q), 16) << ']';
	std::cout << " X=";
        std::cout << '[' << toHex(hi(x.q), 16) << ']';
	std::cout << " Y=";
        std::cout << '[' << toHex(hi(y.q), 16) << ']';
    //std::cout << std::endl;
	std::cout << " SP=";
		std::cout << '[' << toHex(hi(sp.q), 16) << ']';
	std::cout << " {";
	std::cout << ' ' << toHex(getQWord(sp.q), 16);
	std::cout << ' ' << toHex(getQWord(sp.q + 8), 16);
	std::cout << ' ' << toHex(getQWord(sp.q + 16), 16);
	std::cout << ' ' << toHex(getQWord(sp.q + 24), 16);
	std::cout << " }";
    std::cout << std::endl;
}

// Rust ffi OUT

extern "C" {
    void emu_reset(bool trace) {
        emu65x64::reset(trace);
    }

    void emu_step() {
        emu65x64::step();
    }

    unsigned long emu_getCycles() {
        return emu65x64::getCycles();
    }

    bool emu_isStopped() {
        return emu65x64::isStopped();
    }

    void emu_resume() {
        emu65x64::resume();
    }

    int emu_getStopReason() {
        return (int) emu65x64::getStopReason();
    }

    void emu_interrupt() {
        emu65x64::interrupt();
    }

    unsigned char emu_getCopInstSize() {
        return emu65x64::getCopInstSize();
    }

    unsigned char emu_getCopInst(unsigned short *inst) {
        return emu65x64::getCopInst(inst);
    }
}

// test

int main() {
    emu_reset(true);

    while (!emu_isStopped()) {
        emu_step();
    }

    printf("STOP!: %d\n", emu_getStopReason());
}
