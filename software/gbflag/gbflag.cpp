#include <cstdio>
#include <iostream>
#include <ctime>

#include "system.h"
#include "io.h"
#include "FatFs/ff.h"

#undef main

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

using namespace std;

const int zoom = 4;

bool bRestart = false;
int iGame = 0; int iGameMax;

// NIOD II Accelerators
#define PACK16(A)		ALT_CI_GBACC_0(0,A)
#define RESET_DIV()		ALT_CI_GBACC_0(1,0)
#define GET_DIV()			ALT_CI_GBACC_0(2,0)
#define IO_TICK()			ALT_CI_GBACC_0(3,0)
#define GET_IF(A)		ALT_CI_GBACC_0(4,A)
#define SET_IF(A)       ALT_CI_GBACC_0(5,A)
#define SET_TMA(A)		ALT_CI_GBACC_0(6,A)
#define SET_TAC(A)		ALT_CI_GBACC_0(7,A)
#define GET_TIMA()		ALT_CI_GBACC_0(8,0)
#define SET_TIMA(A)		ALT_CI_GBACC_0(9,A)

namespace Video {
    void mliner(u8* linebuf);}

namespace CPU
{
    bool bQuit = false;
    bool bEIDelay = false;

    u8 RAM[8 * 1024];
    u8 HRAM[128];

    // Registers
    struct
    {
        union {struct {u8 C; u8 B;}; u16 BC;};
        union {struct {u8 E; u8 D;}; u16 DE;};
        union {struct {u8 L; u8 H;}; u16 HL;};
        union {struct {u8 FLAGS; u8 A;}; u16 AF;};
        struct {    // Optimization : use a full byte for each flag
            u8 C;    // Carry
            u8 H;    // Half-Carry
            u8 N;    // Substract
            u8 Z;    // Zero
        } F;
        u16 PC;
        u16 SP;
    } R;

    // Internal state
    bool IME;    // Interrupt enable

    union IFLAG {
        struct  {
            u8 vblank   : 1;
            u8 lcdstat  : 1;
            u8 timer    : 1;
            u8 serial   : 1;
            u8 joypad   : 1;
        };
        u8 value;
    };

    IFLAG& IE = (IFLAG&)HRAM[127];
    IFLAG IF;

    bool bHalt;

    // Clock
    void tick();

    // Bus
    template<bool write> u8 BusAccess(const u16 addr, const u8 v = 0);
    u8   RB(const u16 addr)             {tick(); return BusAccess<false>(addr);}
    void WB(const u16 addr, const u8 v) {tick(); BusAccess<true>(addr, v);}

    // OpCode table
    typedef void (*TOpCode) ();

    void Execute();
}

namespace Video
{
    u8 VRAM[8 * 1024];
    u8 OAM[256];    // only 160 bytes actually used

    union { struct {
        u8 bgdisplay        : 1;
        u8 objdisplay       : 1;
        u8 objsize          : 1;
        u8 bgtilemap        : 1;
        u8 bgwintiledata    : 1;
        u8 windisp          : 1;
        u8 wintilemap       : 1;
        u8 lcden            : 1;
    }; u8 value;} LCDC;

    union { struct {
        u8 mode             : 2;
        u8 coincidence      : 1;
        u8 mode0int         : 1;
        u8 mode1int         : 1;
        u8 mode2int         : 1;
        u8 coincidenceint   : 1;
    }; u8 value;} STAT;

    u8 SCY;
    u8 SCX;
    u8 LY;
    u8 LYC;
    u8 BGP;
    u8 OBP0;
    u8 OBP1;
    u8 WY;
    u8 WX;

    u8 LCDX;
    u8 mode;

    void Init();

    void tick();

    u8 GetSTAT();
    void SetSTAT(const u8 v);

    template<bool write> u8 VRAMAccess(const u16 addr, const u8 v = 0);
    template<bool write> u8 OAMAccess(const u8 addr, const u8 v = 0);
}

namespace GamePak
{
    u8* ROM = NULL;
    u8* RAM = NULL;
    u32 ROMSize = 0, RAMSize = 0;
    u32 ROMBankCount = 0, RAMBankCount = 0;
    u8 MBC;

    u8* CurROMBank;

    union 
    {
        struct
        {
            u8 EntryPoint[4];   u8 Logo[48];        u8 Title[16];       u8 NewLicenseCode[2];
            u8 SGBFlag;         u8 CartType;        u8 ROMSize;         u8 RAMSize;             
            u8 DestinationCode; u8 OldLicenseCode;  u8 MaskROMVersion;  u8 HeaderChecksum; 
            u16 GlobalChecksum;
        };
        u8 raw[80];
    } header;

    bool Init(const char* filename);
    void Release();
 
    template<bool write> u8 ROMAccess(const u16 addr, const u8 v = 0);
    template<bool write> u8 RAMAccess(const u16 addr, const u8 v = 0);
}

namespace IO
{
    u8 P1;
    union {
        struct {
            u8 low;
            u8 DIV;
        };
        u16 value;
    } Divider;

    union {
        struct {
            u8 low;
            u8 TIMA;
            u16 overflow;
        }; u32 value;
    } Timer;
    u8 TMA;
    u8 TAC;

    u8 NR10;
    u8 NR11;
    u8 NR12;
    u8 NR13;
    u8 NR14;
    u8 NR21;
    u8 NR22;
    u8 NR23;
    u8 NR24;
    u8 NR30;
    u8 NR31;
    u8 NR32;
    u8 NR33;
    u8 NR34;
    u8 NR41;
    u8 NR42;
    u8 NR43;
    u8 NR44;
    u8 NR50;
    u8 NR51;
    u8 NR52;
    u8 AUD3WAVERAM[16];

    union { 
        struct {
            u8 right    : 1;
            u8 left     : 1;
            u8 up       : 1;
            u8 down     : 1;
            u8 a        : 1;
            u8 b        : 1;
            u8 select   : 1;
            u8 start    : 1;
        }; u8 value;
    } Keys;

    void Init();

    void tick();

    template<bool write> u8 Access(const u8 addr, const u8 v = 0);

    void SetTAC(u8 v);

    void PollEvents();
}

namespace CPU
{
    void Init()
    {
        R.PC = 0x100;
        R.SP = 0xFFFE;
        R.A = 0x01;
        R.FLAGS = 0xB0;
        R.F.Z = 1; R.F.N = 0; R.F.C = 1; R.F.H = 1;
        R.BC = 0x0013;
        R.DE = 0x00d8;
        R.HL = 0x014d;

        IF.value = 0;
        memset(&HRAM[0], 0, sizeof(HRAM));

        bQuit = false;
        bHalt = false;
    }

    void tick()
    {
        Video::tick();
        IO::tick();
    }

    template<bool write> u8 BusAccess(const u16 addr, const u8 v)
    {
        if (addr < 0x8000)
        {
            return GamePak::ROMAccess<write>(addr, v);
        }
        else if (addr < 0xa000)
        {
            return Video::VRAMAccess<write>(addr, v);
        }
        else if (addr < 0xc000)
        {
            return GamePak::RAMAccess<write>(addr, v);
        }
        else if (addr < 0xfe00)
        {
            if (write) RAM[addr & 0x1fff] = v; else return RAM[addr & 0x1fff];
        }
        else if (addr < 0xff00)
        {
            return Video::OAMAccess<write>(addr & 0xff, v);
        }
        else
        {
            return IO::Access<write>(addr & 0xff, v);
        }

        return 0;
    }

    enum Operand16 {OpBC, OpDE, OpHL, OpSP, OpAF, OpImm16};
    template<Operand16 op16>
	u16 GetOperand16()
	{
		u16 t;
		switch(op16) {
		case OpBC : return R.BC;
		case OpDE : return R.DE;
		case OpHL : return R.HL;
		case OpSP : return R.SP;
		case OpAF : R.FLAGS = (R.F.Z << 7) | (R.F.N << 6) | (R.F.H << 5) | (R.F.C << 4); return R.AF;
		case OpImm16 : R.PC++; t = RB(R.PC); R.PC++; t = t + ((RB(R.PC) << 8)); return t;
		}
		return 0;
	}

    enum Operand8 { OpA, OpB, OpC, OpD, OpE, OpH, OpL, OpIHL, OpImm , OpIBC, OpIDE, OpIoImm, OpIoC, OpHLI, OpHLD, OpIImm};

    template<Operand8 op8>
    u8 GetOperand8()
    {
        u8 t; u16 t16;
        switch(op8) {
        case OpA : return R.A;
        case OpB : return R.B;
        case OpC : return R.C;
        case OpD : return R.D;
        case OpE : return R.E;
        case OpH : return R.H;
        case OpL : return R.L;
        case OpIHL : return RB(R.HL);
        case OpIBC : return RB(R.BC);
        case OpIDE : return RB(R.DE);
        case OpImm : R.PC++; return RB(R.PC);
        case OpIoImm : R.PC++; t = RB(R.PC); return RB(0xff00 | t);
        case OpIoC : return RB(0xff00 | R.C);
        case OpHLI : t = RB(R.HL); R.HL++; return t;
        case OpHLD : t = RB(R.HL); R.HL--; return t;
        case OpIImm : t16 = GetOperand16<OpImm16>(); return RB(t16);
        }
        return 0;
    }

    template<Operand8 op8>
    void SetOperand8(const u8 v)
    {
        u8 t; u16 t16;
        switch(op8) {
        case OpA : R.A = v; break;
        case OpB : R.B = v; break;
        case OpC : R.C = v; break;
        case OpD : R.D = v; break;
        case OpE : R.E = v; break;
        case OpH : R.H = v; break;
        case OpL : R.L = v; break;
        case OpIHL : WB(R.HL, v); break;
        case OpIBC : WB(R.BC, v); break;
        case OpIDE : WB(R.DE, v); break;
        case OpIoImm : R.PC++; t = RB(R.PC); WB(0xff00 | t, v); break;
        case OpIoC : WB(0xff00 | R.C, v); break;
        case OpHLI : WB(R.HL, v); R.HL++; break;
        case OpHLD : WB(R.HL, v); R.HL--; break;
        case OpIImm : t16 = GetOperand16<OpImm16>(); WB(t16, v); break;
        }
    }

    template<Operand16 op16>
    void SetOperand16(const u16 v)
    {
        switch(op16) {
        case OpBC : R.BC = v; break;
        case OpDE : R.DE = v; break;
        case OpHL : R.HL = v; break;
        case OpSP : R.SP = v; break;
        case OpAF : R.AF = v; 
            R.F.Z = (R.FLAGS & 0x80) ? 1 : 0;
            R.F.N = (R.FLAGS & 0x40) ? 1 : 0;
            R.F.H = (R.FLAGS & 0x20) ? 1 : 0;
            R.F.C = (R.FLAGS & 0x10) ? 1 : 0;
            break;
        }
    }

    void NOP(){R.PC++;}

    template<Operand8 src>
    void INC() 
    {
        u8 t = GetOperand8<src>();
        R.F.H = ((t & 0xf) == 0xf) ? 1 : 0;
        t = (t + 1) & 0xff; SetOperand8<src>(t);
        R.F.N = 0; R.F.Z = (t == 0) ? 1 : 0; 
        R.PC++;
    }

    template<Operand16 src>
    void INC16(){u16 t = GetOperand16<src>(); t = t + 1; SetOperand16<src>(t); R.PC++;}

    template<Operand16 src>
    void DEC16(){u16 t = GetOperand16<src>(); t = t - 1; SetOperand16<src>(t); R.PC++;}

    template<Operand8 src>
    void DEC() 
    {
        u8 t = GetOperand8<src>();
        R.F.H = ((t & 0xf) == 0) ? 1 : 0;
        t = (t - 1) & 0xff; SetOperand8<src>(t);
        R.F.N = 1; R.F.Z = (t == 0) ? 1 : 0; 
        R.PC++;
    }

    enum OpALU { ADD, ADC, SUB, SBC, CP};

    template<Operand8 src, OpALU alu>
    void ALU()
    {
        bool carry = (alu == ADC || alu == SBC);
        bool sub = !(alu == ADD || alu == ADC);
        u32 t = GetOperand8<src>(); 
        if (sub) {
            R.F.H = (((R.A & 0xf) - (t & 0xf) - (carry ? R.F.C : 0)) >> 4) & 1;
            t = R.A - t - (carry ? R.F.C : 0);
        } else {
            R.F.H = ((t & 0xf) + (R.A & 0xf) + (carry ? R.F.C : 0)) >> 4;
            t += R.A + (carry ? R.F.C : 0); 
        }
        if (alu != CP) R.A = t & 0xff; 
        R.F.Z = ((t & 0xff) == 0) ? 1 : 0;  
        R.F.C = (t >> 8) & 1;
        R.F.N = sub ? 1 : 0;
        R.PC++;
    }

    template<Operand16 src>
    void ADDHL()
    {
        u32 t = GetOperand16<src>();
        R.F.H = ((R.HL & 0x0fff) + (t & 0x0fff) > 0x0fff) ? 1 : 0;
        t += R.HL; R.HL = (t & 0xffff);
        R.F.C = (t >> 16) & 1; R.F.N = 0; R.PC++; tick(); 
    }

    enum OpBITW {AND, OR, XOR};

    template<Operand8 src, OpBITW op>
    void BITW()
    {
        u8 t = GetOperand8<src>();
        switch (op){
            case AND : R.A &= t; break;
            case OR :  R.A |= t; break;
            case XOR:  R.A ^= t; break;
        }
        R.F.C = 0; R.F.N = 0; R.F.H = (op == AND) ? 1 : 0; R.F.Z = (R.A == 0) ? 1 : 0;
        R.PC++;
    }

    enum OpBITS { RL, RLA, RLC, RLCA, RR, RRA, RRC, RRCA, SLA, SRA, SRL, SWP};

    template<Operand8 src, OpBITS op>
    void  BITS()
    {
        bool z0 = (op == RLA || op == RLCA || op == RRA || op == RRCA);
        u32 t = GetOperand8<src>();
        u8 prevFC = R.F.C;
        if (op == RL || op == RLA || op == RLC || op == RLCA || op == SLA) {
            R.F.C = (t & 0x80) ? 1 : 0;
        }
        else if (op == RR || op == RRA || op == RRC || op == RRCA || op == SRA || op == SRL) {
            R.F.C = t & 1;
        }
        else { R.F.C = 0;}

        switch (op){
            case RL:
            case RLA:  t = (t << 1) + prevFC; break;
            case RLC:
            case RLCA: t = (t << 1) + ((t >> 7) & 1); break;
            case RR:
            case RRA: t = (t >> 1) + (prevFC << 7); break;
            case RRC:
            case RRCA: t = (t >> 1) + ((t & 1) << 7); break;
            case SLA: t = t << 1; break;
            case SRA: t = (t >> 1) + (t & 0x80); break;
            case SRL: t = (t >> 1); break;
            case SWP: t = ((t & 0xf) << 4) + ((t & 0xf0) >> 4); break;
        }
        R.F.N = 0; R.F.H = 0; R.F.Z = (!z0 && (t & 0xff) == 0) ? 1 : 0;
        SetOperand8<src>(t);
        R.PC++;
    }

    template<Operand8 src, u8 bit>
    void SET(){SetOperand8<src>(GetOperand8<src>() | (1 << bit)); R.PC++;}

    template<Operand8 src, u8 bit>
    void RES(){SetOperand8<src>(GetOperand8<src>() & (~(1 << bit))); R.PC++;}

    template<Operand8 src, u8 bit>
    void BIT(){R.F.N = 0; R.F.H = 1; R.F.Z = (GetOperand8<src>() & (1 << bit)) ? 0 : 1; R.PC++;}

    void HALT(){bHalt = true; R.PC++;}

    template<Operand8 src, Operand8 dst>
    void LD(){if (src == OpIHL && dst == OpIHL) {HALT();} else {SetOperand8<dst>(GetOperand8<src>()); R.PC++;}}

    template<Operand16 src, Operand16 dst>
    void LD16(){SetOperand16<dst>(GetOperand16<src>()); R.PC++;}

    void LDSP(){u16 t = GetOperand16<OpImm16>(); WB(t, R.SP & 0xff); t++; WB(t, (R.SP >> 8) & 0xff); R.PC++;}

    template<Operand16 dst>
    void LDSPOF()
    {
        R.PC++; signed char t = RB(R.PC); R.PC++;
        R.F.H = (((R.SP & 0x0f) + (t & 0x0f)) >> 4) & 1;
        R.F.C = (((u32)(R.SP & 0xff) + (u8)(t)) >> 8) & 1;
        u32 t32 = R.SP + t; 
        SetOperand16<dst>(t32 & 0xffff); 
        R.F.Z = 0; R.F.N = 0;tick();
    }

    void STOP(){R.PC++;}

    void CB();

    void SCF() {R.F.C = 1; R.F.H = 0; R.F.N = 0; R.PC++;}
    void CCF() {R.F.C = R.F.C^1; ; R.F.H = 0; R.F.N = 0; R.PC++;}

    void PUSH16(const u16 v) {
        union {struct {u8 L; u8 H;}; u16 HL;} t; t.HL = v;
        R.SP--; WB(R.SP, t.H); R.SP--; WB(R.SP, t.L);
    }

    u16 POP16() {
        union {struct {u8 L; u8 H;}; u16 HL;} t;
        t.L = RB(R.SP); R.SP++; t.H = RB(R.SP); R.SP++;
        return t.HL;
    }

    template<Operand16 src>
    void PUSH(){PUSH16(GetOperand16<src>()); R.PC++;}

    template<Operand16 dst>
    void POP(){SetOperand16<dst>(POP16()); R.PC++;}

    enum CondFlag {Always, CondZ, CondNZ, CondC, CondNC, RETI};
    template<CondFlag con>
    bool Condition()
    {
        switch(con){
            case CondZ : return (R.F.Z != 0);
            case CondNZ : return (R.F.Z == 0);
            case CondC : return (R.F.C !=  0);
            case CondNC : return (R.F.C == 0);
        }
        return true;
    }

    template<CondFlag con>
    void CALL()
    {
        u16 t = GetOperand16<OpImm16>(); R.PC++;
        if (Condition<con>()) {PUSH16(R.PC); R.PC = t; tick();}
    }

    template<u8 addr>
    void RST(){R.PC++; PUSH16(R.PC); R.PC = addr; tick();}

    template<CondFlag con>
    void RET()
    {
        if (Condition<con>()) {R.PC = POP16(); tick();} else {R.PC++;}
        if (con == RETI) {IME = true;}
    }

    template<CondFlag con>
    void JP()
    {
        u16 t = GetOperand16<OpImm16>();
        if (Condition<con>()) {R.PC = t; tick();} else {R.PC++;}
    }

    template<CondFlag con>
    void JR()
    {
        R.PC++; u8 t = RB(R.PC); R.PC++;
        if (Condition<con>()) {R.PC += (signed char)(t); tick();}
    }

    void JPHL() {R.PC = R.HL;}

    void EI() {IME = true; R.PC++; bEIDelay = true;};
    void DI() {IME = false; R.PC++;bEIDelay = false;};

    void CPL(){R.A = ~R.A; R.F.N = 1; R.F.H = 1; R.PC++;}
    void DAA()
    {
		/*int tmp = R.A;

		if ( ! ( R.F.N ) ) {
			if ( ( R.F.H ) || ( tmp & 0x0F ) > 9 )
				tmp += 6;
			if ( ( R.F.C ) || tmp > 0x9F )
				tmp += 0x60;
		} else {
			if ( R.F.H ) {
				tmp -= 6;
				if ( ! ( R.F.C ) )
					tmp &= 0xFF;
			}
			if ( R.F.C)
					tmp -= 0x60;
		}
        R.F.H = 0; R.F.Z = 0;
		if ( tmp & 0x100 )
			R.F.C = 1;
		R.A = tmp & 0xFF;
		if ( ! R.A )
			R.F.Z = 1;

        R.PC++;*/

		int tmp = R.A;

        if (R.F.N){
			if ( R.F.H ) {
				tmp -= 6;
                if ( ! ( R.F.C ) ) {tmp &= 0xFF;}
			}
            if ( R.F.C) {tmp -= 0x60;}
		} else {
            if ( ( R.F.H ) || ( tmp & 0x0F ) > 9 ) {tmp += 6;}
            if ( ( R.F.C ) || tmp > 0x9F ) {tmp += 0x60;}
        }

        R.F.H = 0;
        if (tmp & 0x100) {R.F.C = 1;}
		R.A = tmp & 0xFF;
        R.F.Z = (R.A == 0) ? 1 : 0;

        R.PC++;

    }

#define STD_OPERANDS(x,y) x<OpB, y>, x<OpC, y>, x<OpD, y>, x<OpE, y>, x<OpH, y>, x<OpL, y>, x<OpIHL, y>, x<OpA, y>

    TOpCode opcodes[256] = {
        NOP,        LD16<OpImm16, OpBC>,    LD<OpA, OpIBC>, INC16<OpBC>, INC<OpB>,      DEC<OpB>,   LD<OpImm, OpB>,     BITS<OpA, RLCA>,
        LDSP,       ADDHL<OpBC>,            LD<OpIBC, OpA>, DEC16<OpBC>, INC<OpC>,      DEC<OpC>,   LD<OpImm, OpC>,     BITS<OpA, RRCA>,
        STOP,       LD16<OpImm16, OpDE>,    LD<OpA, OpIDE>, INC16<OpDE>, INC<OpD>,      DEC<OpD>,   LD<OpImm, OpD>,     BITS<OpA, RLA>,
        JR<Always>, ADDHL<OpDE>,            LD<OpIDE, OpA>, DEC16<OpDE>, INC<OpE>,      DEC<OpE>,   LD<OpImm, OpE>,     BITS<OpA, RRA>,
        JR<CondNZ>, LD16<OpImm16, OpHL>,    LD<OpA, OpHLI>, INC16<OpHL>, INC<OpH>,      DEC<OpH>,   LD<OpImm, OpH>,     DAA,
        JR<CondZ>,  ADDHL<OpHL>,            LD<OpHLI, OpA>, DEC16<OpHL>, INC<OpL>,      DEC<OpL>,   LD<OpImm, OpL>,     CPL,
        JR<CondNC>, LD16<OpImm16, OpSP>,    LD<OpA, OpHLD>, INC16<OpSP>, INC<OpIHL>,    DEC<OpIHL>, LD<OpImm, OpIHL>,   SCF,
        JR<CondC>,  ADDHL<OpSP>,            LD<OpHLD, OpA>, DEC16<OpSP>, INC<OpA>,      DEC<OpA>,   LD<OpImm, OpA>,     CCF,
        STD_OPERANDS(LD, OpB),
        STD_OPERANDS(LD, OpC),
        STD_OPERANDS(LD, OpD),
        STD_OPERANDS(LD, OpE),
        STD_OPERANDS(LD, OpH),
        STD_OPERANDS(LD, OpL),
        STD_OPERANDS(LD, OpIHL),
        STD_OPERANDS(LD, OpA),
        STD_OPERANDS(ALU, ADD),
        STD_OPERANDS(ALU, ADC),
        STD_OPERANDS(ALU, SUB),
        STD_OPERANDS(ALU, SBC),
        STD_OPERANDS(BITW, AND),
        STD_OPERANDS(BITW, XOR),
        STD_OPERANDS(BITW, OR),
        STD_OPERANDS(ALU, CP),
        RET<CondNZ>,        POP<OpBC>,          JP<CondNZ>,         JP<Always>, CALL<CondNZ>,  PUSH<OpBC>,     ALU<OpImm, ADD>,    RST<0x00>,
        RET<CondZ>,         RET<Always>,        JP<CondZ>,          CB,         CALL<CondZ>,   CALL<Always>,   ALU<OpImm, ADC>,    RST<0x08>,
        RET<CondNC>,        POP<OpDE>,          JP<CondNC>,         NOP,        CALL<CondNC>,  PUSH<OpDE>,     ALU<OpImm, SUB>,    RST<0x10>,
        RET<CondC>,         RET<RETI>,          JP<CondC>,          NOP,        CALL<CondC>,   NOP,            ALU<OpImm, SBC>,    RST<0x18>,
        LD<OpA, OpIoImm>,   POP<OpHL>,          LD<OpA, OpIoC>,     NOP,        NOP,           PUSH<OpHL>,     BITW<OpImm, AND>,   RST<0x20>,
        LDSPOF<OpSP>,       JPHL,               LD<OpA, OpIImm>,    NOP,        NOP,           NOP,            BITW<OpImm, XOR>,   RST<0x28>,
        LD<OpIoImm, OpA>,   POP<OpAF>,          LD<OpIoC, OpA>,     DI,         NOP,           PUSH<OpAF>,     BITW<OpImm, OR>,    RST<0x30>,
        LDSPOF<OpHL>,       LD16<OpHL, OpSP>,   LD<OpIImm, OpA>,    EI,         NOP,           NOP,            ALU<OpImm, CP>,     RST<0x38>,
    };

    TOpCode opcodesCB[256] = {
        STD_OPERANDS(BITS, RLC), STD_OPERANDS(BITS, RRC), STD_OPERANDS(BITS, RL), STD_OPERANDS(BITS, RR),
        STD_OPERANDS(BITS, SLA), STD_OPERANDS(BITS, SRA), STD_OPERANDS(BITS, SWP), STD_OPERANDS(BITS, SRL),
        STD_OPERANDS(BIT, 0),   STD_OPERANDS(BIT, 1),   STD_OPERANDS(BIT, 2),   STD_OPERANDS(BIT, 3),
        STD_OPERANDS(BIT, 4),   STD_OPERANDS(BIT, 5),   STD_OPERANDS(BIT, 6),   STD_OPERANDS(BIT, 7),
        STD_OPERANDS(RES, 0),   STD_OPERANDS(RES, 1),   STD_OPERANDS(RES, 2),   STD_OPERANDS(RES, 3),
        STD_OPERANDS(RES, 4),   STD_OPERANDS(RES, 5),   STD_OPERANDS(RES, 6),   STD_OPERANDS(RES, 7),
        STD_OPERANDS(SET, 0),   STD_OPERANDS(SET, 1),   STD_OPERANDS(SET, 2),   STD_OPERANDS(SET, 3),
        STD_OPERANDS(SET, 4),   STD_OPERANDS(SET, 5),   STD_OPERANDS(SET, 6),   STD_OPERANDS(SET, 7),
    };

    void CB() {R.PC++; opcodesCB[RB(R.PC)]();}

    void Execute()
    {
        while(!bQuit)
        {
            if (bHalt)
            {
                tick();
            }
            else
            {
                //u16 oPC = R.PC;
                //if (oPC == 0xc31a)
                //{
                //    oPC += 0;
                //}
            	//IOWR_16DIRECT(SEG7_LUT_4_0_BASE, 0, R.PC);
                u8 opcode = RB(R.PC);
                //static u8 l_opcode = 0;

                //if (opcode == 0xf5 && l_opcode == 0xf1)
                //{
                //    opcode += 0;
                //}
                //l_opcode = opcode;
                opcodes[opcode]();
                //if (R.PC == 0xC086)
                //{
                //    R.PC += 0;
                //}
            }

            if (bEIDelay)
            {
            	bEIDelay = false;
            }
            else
            {
            	IF.value = GET_IF(IF.value);
				if (IF.value & IE.value & 0x1f)
				{
					bHalt = false;
					if (IME) {
						u8 Ivector;
						if (IF.vblank & IE.vblank) {
							IF.vblank = 0;
							Ivector = 0x40;
						}
						else if (IF.lcdstat & IE.lcdstat) {
							IF.lcdstat = 0;
							Ivector = 0x48;
						}
						else if (IF.timer & IE.timer) {
							IF.timer = 0;
							Ivector = 0x50;
						}
						else if (IF.serial & IE.serial) {
							IF.serial = 0;
							Ivector = 0x58;
						}
						else {
							IF.joypad = 0;
							Ivector = 0x60;
						}
						SET_IF(IF.value);
						IME = false;
						PUSH16(R.PC); R.PC = Ivector;
					}
				}
            }
        }
    }
}

namespace Video
{
    void Init()
    {
        STAT.value = 0x84;
        mode = 2;
        LCDX = 0;

        LCDC.value = 0x91;
        LY = 0;
        SCX = 0;
        SCY = 0;
        WX = 0;
        WY = 0;
        BGP = 0xFC;
        OBP0 = 0xff;
        OBP1 = 0xff;

        memset(VRAM, 0, sizeof(VRAM));
        memset(OAM, 0, sizeof(OAM));
    }

    void SetSTAT(const u8 v){STAT.value = v & 0xfc;}
    u8 GetSTAT(){return STAT.value | mode;}

    template<bool write> u8 VRAMAccess(const u16 addr, const u8 v)
    {
        //if (LCDC.lcden && mode == 3) return 0;
        if (write) VRAM[addr & 0x1fff] = v; else return VRAM[addr & 0x1fff];
        return 0;
    }

    template<bool write> u8 OAMAccess(const u8 addr, const u8 v)
    {
        //if (LCDC.lcden && (mode & 2)) return 0;
        if (write) OAM[addr] = v; else return OAM[addr];
        return 0;
    }

    void RenderLine();
    void Flip();

    void tick()
    {
        switch(mode)
        {
        case 0 : 
            LCDX++; 
            if (LCDX == 114) {
                LCDX = 0; LY++;
                STAT.coincidence = (LY == LYC) ? 1 : 0;
                if (STAT.coincidence && STAT.coincidenceint) {CPU::IF.lcdstat = 1;}
                if (LY == 144) {
                    mode = 1; // Transition to Mode 1 VBlank
                    Flip();
                    CPU::IF.vblank = 1;
                    if (STAT.mode1int) {CPU::IF.lcdstat = 1;}
                }
                else { mode = 2;}
            } break;
        case 1:
            LCDX++;
            if (LCDX == 114) {
                LCDX = 0; LY++;
                if (LY == 154) {
                    LY = 0;
                    mode = 2; // Transition to Mode 2 OAM  
                    if (STAT.mode2int) {CPU::IF.lcdstat = 1;}
                }
                STAT.coincidence = (LY == LYC) ? 1 : 0;
                if (STAT.coincidence && STAT.coincidenceint) {CPU::IF.lcdstat = 1;}
            } break;
        case 2:
            LCDX++;
            if (LCDX == 20) {
                mode = 3;    // Transition to Mode 3 OAM+VRAM
                RenderLine();
            } break;
        case 3:
            LCDX++;
            if (LCDX == 63) {
                mode = 0;   // Transition to Mode 0 HBlank
                if (STAT.mode0int) {CPU::IF.lcdstat = 1;}
            } break;
        }
    }

    u16 GetTile(const u8 tile, const u8 yofs, const u8* vramdata)
    {
        u16 tiledata = (tile << 4) + (yofs << 1);
        /*u8 datah = vramdata[tiledata]; u8 datal = vramdata[tiledata + 1];
        u16 data = ((datal & 0x01) << 14)| ((datah & 0x01) << 15)| ((datal & 0x02) << 11)| ((datah & 0x02) << 12)|
                   ((datal & 0x04) << 8) | ((datah & 0x04) << 9) | ((datal & 0x08) << 5) | ((datah & 0x08) << 6) |
                   ((datal & 0x10) << 2) | ((datah & 0x10) << 3) | ((datal & 0x20) >> 1) | ((datah & 0x20)     ) |
                   ((datal & 0x40) >> 4) | ((datah & 0x40) >> 3) | ((datal & 0x80) >> 7) | ((datah & 0x80) >> 6);
        data = ((data & 0x5555) << 1) | ((data & 0xaaaa) >> 1);*/

		u16 data = *((u16*)(&vramdata[tiledata]));
		data = PACK16(data);

        return data;
    }

    void RenderLine()
    {
        u8 line[176];
        u8 bgcol[4] = {BGP & 0x3, (BGP >> 2) & 3, (BGP >> 4) & 3, (BGP >> 6) & 3};
        memset(line, bgcol[0], sizeof(line));

        u8 sprcache[40]; u8 sprcacheidx = 0; u8 lastsprcacheidx = 0;
        u8 sprsize = LCDC.objsize ? 16 : 8;

        // Find sprites
        if (LCDC.objdisplay)
        {
            for (u8 spridx = 0; spridx < 160; spridx+=4)
            {
                u8 y = OAM[spridx];
                if ((y <= LY + 16) && (y + sprsize > LY + 16))
                {
                    u8 attrib = OAM[spridx + 3];
                    u8 yofs = ((LY + 16) - y);
                    if (attrib & 0x40) {yofs = sprsize - 1 - yofs;}
                    u8 tile;
                    if (LCDC.objsize){tile = (OAM[spridx + 2] & 0xfe) | ((yofs & 0x08) >> 3);}
                    else{tile = OAM[spridx + 2];}
                    u16 data = GetTile(tile, yofs & 7, &VRAM[0]);
                    if (attrib & 0x20) {
                        data = ((data & 0x3333) << 2) | ((data & 0xcccc) >> 2);
                        data = ((data & 0x0f0f) << 4) | ((data & 0xf0f0) >> 4);
                        data = ((data & 0x00ff) << 8) | ((data & 0xff00) >> 8);
                    }
                    sprcache[sprcacheidx] = OAM[spridx + 1];sprcacheidx++;
                    sprcache[sprcacheidx] = attrib;sprcacheidx++;
                    *(u16*)(&sprcache[sprcacheidx]) = data; sprcacheidx+=2;
                    lastsprcacheidx = sprcacheidx;
                    if (sprcacheidx == 40) break;
                }
            }
        }

        for (u8 state = 0; state < 3; state++)
        {
            if ((state & 1) == 0)
            {
                u8 objcol0[4] = {OBP0 & 0x3, (OBP0 >> 2) & 3, (OBP0 >> 4) & 3, (OBP0 >> 6) & 3};
                u8 objcol1[4] = {OBP1 & 0x3, (OBP1 >> 2) & 3, (OBP1 >> 4) & 3, (OBP1 >> 6) & 3};
                // Sprite
                for (u8 spridx = 0; spridx < sprcacheidx; spridx+=4)
                {
                    u8 attrib = sprcache[spridx+1];
                    if ((state == 0 && (attrib & 0x80)) || (state == 2 && !(attrib & 0x80)))
                    {
                        u8 sprx = sprcache[spridx];
                        if (sprx < 168)
                        {
                            u16 data = *(u16*)(&sprcache[spridx + 2]);
                            for (u8 x = sprx; x < sprx + 8;x++)
                            {
                                u8 pixel = data & 0x3;
                                if (pixel) {line[x] = (attrib & 0x10) ? objcol1[pixel] : objcol0[pixel];}
                                data >>=2;
                            }
                        }
                    }
                }
            }
            else
            {
                const u8* bgData = &VRAM[LCDC.bgwintiledata ? 0 : 0x800];
                u8 bgAdd = LCDC.bgwintiledata ? 0 : 128;
                u8 bgwinx = SCX & 0x7;
                u16 tileaddr = LCDC.bgtilemap ? 0x1c00 : 0x1800;
                tileaddr += ((((LY + SCY) & 0xff) >> 3) << 5) + (SCX >> 3);
                u16 bgwindata = 0; bool winmode = false;
                u8 bgyofs = (LY + SCY) & 0x7;
                bool bWinOn = LCDC.windisp && WY <= LY;
                for (u8 x = 0; x < 168; x++)
                {
                    if (bWinOn && x == WX )
                    {
                        tileaddr = LCDC.wintilemap ? 0x1c00 : 0x1800;
                        tileaddr += (((LY - WY) >> 3) << 5);
                        bgyofs = (LY - WY) & 0x7;
                        bgwinx = 7;
                    }

                    u8 pixel = bgwindata & 3;
                    if (pixel > 0) {line[x] = bgcol[pixel];}

                    bgwinx++;
                    if (bgwinx == 8 && LCDC.bgdisplay)
                    {
                        u8 tile = VRAM[tileaddr]; tileaddr = (tileaddr & 0xffe0) | ((tileaddr+1) & 0x1f);
                        tile+=bgAdd;
                        bgwindata = GetTile(tile, bgyofs, bgData);
                        bgwinx = 0;
                    }
                    else
                    {
                        bgwindata >>=2;
                    }

                }
            }
        }

        //u16* p = (u16*)(0x81000000);
        //p+=LY*160;
        u32 o = 160 * LY * 2;
        for (u32 x = 0; x < 160; x++)
        {
            u8 t = 3 - line[8 + x];
            //*p = (0x5555 * t);
            u16 p = (0x294a * t);
            //++p;
            IOWR_16DIRECT(SRAM_0_BASE, o, p);
            o+=2;
        }

        /*
        mliner(&line[0]);

        for (u32 y = 0; y < zoom; y++)
        {
            u32* p = (u32*)(&((u8*)s->pixels)[s->pitch * (LY * zoom + y)]);
            for (u32 x = 0; x < 160 * zoom; x++)
            {
                u8 t = 3 - line[8 + x / zoom];
                *p = (0x00555555 * t) | 0x7f000000;
                ++p; 
            }
        }*/
        //TODO video
    }

    void Flip()
    {
        /*static u32 last = 0;
        static u16 last_low = 0;

        if (last == 0){last = SDL_GetTicks();}

        u32 diff;
        do
        {
            diff = SDL_GetTicks() - last;
            diff = ((diff & 255) << 16) + last_low;
            if (diff < 1097204) {SDL_Delay((1097204 - diff) >> 16);}
        }while (diff < 1097204);

        diff -= 1097204; last = diff >> 16; last_low = diff & 0xffff;*/

    	// TODO delay

        IO::PollEvents();
    }
}

namespace GamePak
{
    bool Init(const char* filename)
    {

        try
        {
        	FATFS oFatFs;
			FRESULT res;

			res = f_mount(0, &oFatFs);

			printf("f_mount result:%x\n", res);

			if (res != FR_OK)
			{
				return 0;
			}

			FIL oFile;

			res = f_open(&oFile, filename, FA_READ | FA_OPEN_EXISTING);

			f_lseek(&oFile, 0x100);

			unsigned int bytesread = 0;
			f_read(&oFile, &header.raw[0], sizeof(header.raw), &bytesread);

            char title[sizeof(header.Title) + 1] = {0};

            memcpy(title, header.Title, sizeof(header.Title));

            printf("Title : %s\n", title);

            switch(header.CartType)
            {
            case 0x00 : MBC = 0; break;
            case 0x01: case 0x02: case 0x03: MBC = 1;break;
            case 0x05: case 0x06: MBC = 2; break;
            case 0x0f: case 0x10: case 0x11: case 0x12: case 0x13: MBC = 3; break;
            case 0x15: case 0x16: case 0x17: MBC = 4; break;
            case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: MBC = 5; break;
            }

            printf("MBC : %d\n", MBC);

            bool bValid = true;
            switch (header.RAMSize)
            {
            case 0: RAMSize = 0; break;
            case 1: RAMSize = 2048; break;
            case 2: RAMSize = 8192; break;
            case 3: RAMSize = 32768; break;
            default: bValid = false; break;
            }

            if (MBC == 2) { RAMSize = 512;}

            if (bValid && header.ROMSize <  9)
            {
                if (RAMSize > 0)
                {
                    if (RAMSize < 8192) RAM = new u8[8192]; else RAM = new u8[RAMSize];
                }
                ROMSize = 32768 << header.ROMSize;
                ROM = new u8[ROMSize];

                printf("ROM Size : %d\n", ROMSize);
                printf("RAM Size : %d\n", RAMSize);

    			f_lseek(&oFile, 0);
    			f_read(&oFile, &ROM[0], ROMSize, &bytesread);

                CurROMBank = &ROM[0x4000];
                ROMBankCount = ROMSize / 16384;

                f_close(&oFile);

                return true;
            }
        }
        catch (...)
        {

        }

        if (ROM) delete[] ROM;
        if (RAM) delete[] RAM;
        return false;
    }

    void Release()
    {
        if (ROM) {delete[] ROM; ROM = NULL;}
        if (RAM) {delete[] RAM; RAM = NULL;}
    }
    
    template<bool write> u8 ROMAccess(const u16 addr, const u8 v)
    {
        if (write)
        {
            // Emulate MBC
            if (addr >= 0x2000 && addr <= 0x3fff)
            {
                if (v < ROMBankCount)
                {
                    CurROMBank = &ROM[v << 14];
                }
            }
        }
        else
        {
            if (addr < 0x4000) {return ROM[addr];} else {return CurROMBank[addr & 0x3fff];}
        }
        return 0;
    }

    template<bool write> u8 RAMAccess(const u16 addr, const u8 v)
    {
        if (RAMSize == 0) { return 0;}
        if (write) {RAM[addr & 0x1fff] = v;} else {return RAM[addr & 0x1fff];}
        return 0;
    }
}

namespace IO
{
    u32 CycTilDiv;

    u8 TimerInc;

    void Init()
    {
        //Divider.value = 0;
        //Timer.value = 0;
        //TMA = 0;
        //TAC = 0;
        //TimerInc = 1;
        RESET_DIV();
        SET_TMA(0);
        SET_TIMA(0);
        SET_TAC(0);
    	P1 = 0x0f;
        Keys.value = 0xff;
    }

    void tick()
    {
        //Divider.value += 4;
    	IO_TICK();
/*
        if (TAC & 4) {
            Timer.value += TimerInc;
            if (Timer.overflow) {
                Timer.overflow = 0;
                Timer.TIMA = TMA;
                CPU::IF.timer = 1;
                SET_IF(CPU::IF.value);
            }
        }*/
    }

    u8 MakeP1()
    {
        u8 OldP1 = P1;
        u8 v = 0xf;
        if (P1 & 0x20) {v = v & Keys.value;}
        if (P1 & 0x10) {v = v & (Keys.value >> 4);}
        P1 = (P1 & 0x30) | v | 0xc0;
        if (OldP1 & (~P1) & 0xf)
        {
            CPU::IF.joypad = 1;
        }
        return P1;
    }

#define STD_REG(x,y) case x : if (write) y = v; else return y; break;

    template<bool write> u8 Access(const u8 addr, const u8 v)
    {
        if (addr & 0x80)
        {
            if (write) CPU::HRAM[addr & 0x7f] = v; else return CPU::HRAM[addr & 0x7f];
        }
        else
        {
            switch (addr)
            {
            case 0x00 : if (write) P1 = v; else return MakeP1(); break;
            //case 0x04 : if (write) Divider.value = 0; else return Divider.DIV; break;
            case 0x04 : if (write) RESET_DIV(); else return GET_DIV(); break;
            //case 0x05 : if (write) Timer.TIMA = v; else return Timer.TIMA; break;
            //STD_REG(0x06, TMA);
            //case 0x07 : if (write) SetTAC(v); else return TAC; break;
            case 0x05: if (write) SET_TIMA(v); else return GET_TIMA(); break;
            case 0x06: if (write) {TMA = v; SET_TMA(v);} else return TMA; break;
            case 0x07: if (write) {TAC = v; SET_TAC(v);} else return TAC; break;
            case 0x0f : if (write) CPU::IF.value = SET_IF(v); else return GET_IF(CPU::IF.value); break;
            STD_REG(0x10, NR10);
            STD_REG(0x11, NR11);
            STD_REG(0x12, NR12);
            STD_REG(0x13, NR13);
            STD_REG(0x14, NR14);
            STD_REG(0x16, NR21);
            STD_REG(0x17, NR22);
            STD_REG(0x18, NR23);
            STD_REG(0x19, NR24);
            STD_REG(0x1a, NR30);
            STD_REG(0x1b, NR31);
            STD_REG(0x1c, NR32);
            STD_REG(0x1d, NR33);
            STD_REG(0x1e, NR34);
            STD_REG(0x20, NR41);
            STD_REG(0x21, NR42);
            STD_REG(0x22, NR43);
            STD_REG(0x23, NR44);
            STD_REG(0x24, NR30);
            STD_REG(0x25, NR31);
            STD_REG(0x26, NR32);
            case 0x40 : if (write) Video::LCDC.value = v; else return Video::LCDC.value; break;
            case 0x41 : if (write) Video::SetSTAT(v); else return Video::GetSTAT(); break;
            STD_REG(0x42, Video::SCY);
            STD_REG(0x43, Video::SCX);
            case 0x44 : if (write) Video::LY = 0; else return Video::LY; break;
            STD_REG(0x45, Video::LYC);
            case 0x46 : if (write) {
                for (u8 b = 0; b < 160; b++){
                    Video::OAM[b] = CPU::BusAccess<0>(((u16)v << 8) | b);
                }} break;
            STD_REG(0x47, Video::BGP);
            STD_REG(0x48, Video::OBP0);
            STD_REG(0x49, Video::OBP1);
            STD_REG(0x4A, Video::WY);
            STD_REG(0x4B, Video::WX);
           }
        }
        return 0;
    }

    void SetTAC(u8 v)
    {
        TAC = v;
        switch (v & 3) {
        case 0 : TimerInc = 1; break;
        case 1 : TimerInc = 64; break;
        case 2 : TimerInc = 16; break;
        case 3 : TimerInc = 4; break;
        }
    }

    void PollEvents()
    {
    	u8 nespad;
    	nespad = IORD_8DIRECT(NESPAD_0_BASE, 0);

    	static clock_t starttime = 0;
    	static int framecount = 0;

    	if (starttime == 0)
    	{
    		starttime = clock();
    		framecount = 0;
    	}
    	else
    	{
    		framecount++;
    		if (framecount == 60)
    		{
    			framecount = 0;
    			clock_t diff = clock() - starttime;
    			starttime = clock();
    			if (diff > 0)
    			{
    				int fps = 60 * CLOCKS_PER_SEC / diff;
    				int fpsdec = (fps % 10) + (fps / 10) * 16;
    				IOWR_16DIRECT(SEG7_LUT_4_0_BASE, 0, fpsdec);
    			}
    		}
    	}

    	//IOWR_16DIRECT(SEG7_LUT_4_0_BASE, 0, nespad);
    	/* TODO keys
        SDL_Event event;
        while (SDL_PollEvent( &event ))
        {
            switch(event.type)
            {
            case SDL_QUIT: CPU::bQuit = true; break;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym){
                  case SDLK_LEFT:   Keys.left = 0;  break;
                  case SDLK_RIGHT:  Keys.right = 0; break;
                  case SDLK_UP:     Keys.up = 0;    break;
                  case SDLK_DOWN:   Keys.down = 0;  break;
                  case SDLK_z:      Keys.b = 0;     break;
                  case SDLK_x:      Keys.a = 0;     break;
                  case SDLK_c:      Keys.select = 0;break;
                  case SDLK_v:      Keys.start = 0; break;
                }; break;
            case SDL_KEYUP :
                switch(event.key.keysym.sym){
                  case SDLK_LEFT:   Keys.left = 1;  break;
                  case SDLK_RIGHT:  Keys.right = 1; break;
                  case SDLK_UP:     Keys.up = 1;    break;
                  case SDLK_DOWN:   Keys.down = 1;  break;
                  case SDLK_z:      Keys.b = 1;     break;
                  case SDLK_x:      Keys.a = 1;     break;
                  case SDLK_c:      Keys.select = 1;break;
                  case SDLK_v:      Keys.start = 1; break;
                  case SDLK_q: if(iGame > 0) {iGame--; bRestart = true; CPU::bQuit = true;} break;
                  case SDLK_w: if(iGame < iGameMax) {iGame++; bRestart = true; CPU::bQuit = true;} break;
                } break;
            }
        }
        */
        MakeP1();
    }
}

int main(int argc, char* argv[])
{
    const char* games2[] = {
    		//"Legend of Zelda, The - Link's Awakening (Canada).gb",
    		//"Tetris Attack (USA) (SGB Enhanced).gb",
        "ASTEROID.gb",
    };


    iGame = 0; iGameMax = sizeof(games2) / sizeof(const char*) - 1; 
    do 
    {
        bRestart = false;

        if (!GamePak::Init(games2[iGame]))
        {
            return 0;
        }

        Video::Init();
        IO::Init();
        CPU::Init();
        
        printf("Allo\n");
        CPU::Execute();
        printf("Done\n");
        GamePak::Release();
    } while (bRestart);

    return 0;
}
/*
void PACK16(u16& A)
{
    union { struct {
        u8 b0: 1;
        u8 b1: 1;
        u8 b2: 1;
        u8 b3: 1;
        u8 b4: 1;
        u8 b5: 1;
        u8 b6: 1;
        u8 b7: 1;
    }; u8 byte;} srcH, srcL, dstH, dstL;

    srcL.byte = (u8)(A & 0xff); srcH.byte = (u8)(A >> 8);
    dstH.byte = 0; dstL.byte = 0;

    dstL.b0 = srcL.b7;
    dstL.b1 = srcH.b7;
    dstL.b2 = srcL.b6;
    dstL.b3 = srcH.b6;
    dstL.b4 = srcL.b5;
    dstL.b5 = srcH.b5;
    dstL.b6 = srcL.b4;
    dstL.b7 = srcH.b4;
    dstH.b0 = srcL.b3;
    dstH.b1 = srcH.b3;
    dstH.b2 = srcL.b2;
    dstH.b3 = srcH.b2;
    dstH.b4 = srcL.b1;
    dstH.b5 = srcH.b1;
    dstH.b6 = srcL.b0;
    dstH.b7 = srcH.b0;
    A = dstL.byte | (dstH.byte << 8);
}

void PACKFLIP16(u16& A)
{
    A = ((A & 0x3333) << 2) | ((A & 0xcccc) >> 2);
    A = ((A & 0x0f0f) << 4) | ((A & 0xf0f0) >> 4);
    A = ((A & 0x00ff) << 8) | ((A & 0xff00) >> 8);
}
/*
void Video::mliner(u8* line)
{
    u8  C, D;
    u16 A, B;

    u16 data;
    u8 sprsize;
    u8 sprcache[40];
    u8 adjustedLY;
    u8 x;
    u8 y;
    u8 tile;
    u8 attrib;
    bool priority;
    u8 check;
    u8 sprx;
    u8 sprxend;
    u8 pal;
    u8 pixel;
    u8 sprcacheidx;
    u8 bgAdd;
    u8 bgwinx;
    u16 tileaddr;
    u8 adjustedy;
    u8 bgyofs;
    u8 WinStartX;

    // Fill line buffer with background color 0
    C = 0;
    A = BGP;
    A &= 0x3;
ClearLineBufLoop:
    line[C++] = A & 0xFF;
    if (C < 176) {goto ClearLineBufLoop;}

    // Initialize sprite cache variables
   
    C = 0;
    sprsize = LCDC.objsize ? 16 : 8;

    // Test for spirte enabled
    if (LCDC.objdisplay == 0) {goto StartOfMode3;}

    // Find sprites visible on scanline
    D = 0;
CheckSpriteY:
    y = OAM[D++];
    adjustedLY = LY;
    adjustedLY += 16;

    if (y > adjustedLY) {goto YMismatch;}
    y+= sprsize;
    if (y <= adjustedLY) {goto YMismatch;}
    y-= sprsize;

    // Load sprite data
    x = OAM[D++];
    sprcache[C] = x;
    tile = OAM[D++];
    attrib = OAM[D++];

    // Compute Y Offset
    B = adjustedLY;
    B -= y;

    // Check for Y Flip
    if ((attrib & 0x40) == 0) {goto CheckBigSprite;}
    B = ~B;
    B += sprsize;

CheckBigSprite:

    // Check if big tile needs second tile
    if (LCDC.objsize == 0) {goto FetchTile;}
    tile &= 0xfe;
    if ((B & 0x08) == 0) {goto FetchTile;}
    tile++;

FetchTile:
    //u16 data = GetTile(tile, yofs & 7, &VRAM[0]);

    B &= 7;
    A = tile;
    A <<=3;
    A += B;
    A <<=1;
    A = *(u16*)(&VRAM[A]);
    PACK16(A);

    // Check for X flip
    if ((attrib & 0x20) == 0) {goto FillSprCache;}
    PACKFLIP16(A);

FillSprCache:
    sprcache[C++] = attrib;
    sprcache[C++] = x;
    *(u16*)(&sprcache[C]) = A;
    C+=2;

    if (C == 40) {goto StartOfMode3;}
    goto CheckSprIdx;

YMismatch:
    D +=3;

CheckSprIdx:

    // Check if last sprite has been reached
    if (D < 160) {goto CheckSpriteY;}

StartOfMode3:

    priority = false;

StartSprites:

    // Sprite
    D = 0;
    sprcacheidx = C;

NextSprite:
    if (D >= sprcacheidx) {goto SpritesEnd;}

    attrib = sprcache[D++];
    check = 0x00;
    if (priority) {check = 0x80;}
    A = attrib;
    A &= 0x80;
    A ^= check;
    if ((A & 0x80) == 0) {goto WrongSpritePriority;}

    sprx = sprcache[D++];
    if (sprx >= 168) { goto OutOfX;}

    data = *(u16*)(&sprcache[D]);
    sprxend = sprx;
    sprxend += 8;
    pal =  ((attrib & 0x10) ? OBP1 : OBP0);
NextSpritePixel:
    pixel = data & 0x3;
    if (pixel == 0) {goto ShiftSpriteData;}
    pixel <<= 1;
    A = pal >> pixel;
    A &= 3;
    line[sprx] = (u8)A;
ShiftSpriteData:
    data >>=2;
    sprx++;
    if (sprx < sprxend) { goto NextSpritePixel;}
    goto OutOfX;

WrongSpritePriority:
    D++;
OutOfX:
    D += 2;
    goto NextSprite;

SpritesEnd:

    if (priority) {goto LinerEnd;}

    bgAdd = LCDC.bgwintiledata ? 0 : 128;
    bgwinx = SCX;
    bgwinx &= 0x7;
    tileaddr = 0x1800;
    if ( LCDC.bgtilemap) {tileaddr = 0x1c00;}
    adjustedy = LY;
    adjustedy += SCY;
    A = adjustedy >> 3;
    A <<=5;
    tileaddr += A;
    A = SCX;
    A >>= 3;
    tileaddr += A;
    u16 bgwindata = 0;
    bgyofs = adjustedy;
    bgyofs &= 0x7;
    WinStartX = 168;
    if (LCDC.windisp == 0) {goto SkipWin;}
    if (WY > LY) {goto SkipWin;}
    WinStartX = WX;
SkipWin:
    x = 0;
BGWINXLoop:
    
    if (x != WinStartX) {goto DontStartWin;}

    tileaddr = 0x1800;
    if ( LCDC.wintilemap) {tileaddr = 0x1c00;}

    A = LY;
    A -= WY;
    A >>=3;
    A <<=5;
    tileaddr += A;

    bgyofs = LY;
    bgyofs -= WY;
    bgyofs &= 0x7;
    bgwinx = 7;

DontStartWin:

    pixel = bgwindata & 3;

    if (pixel == 0) {goto BGNextData;}
    pixel <<= 1;
    A = BGP;
    A >>= pixel;
    A &= 3;
    line[x] = (u8)A;

BGNextData:

    bgwinx++;
    if (bgwinx != 8) {goto ShiftBG;}    
    if (LCDC.bgdisplay == 0) {goto ShiftBG;}
 
    A = VRAM[tileaddr]; 
    tileaddr = (tileaddr & 0xffe0) | ((tileaddr+1) & 0x1f);
    A= (u8)(A + bgAdd);

    A <<=3;
    A += bgyofs;
    A <<=1;
    A += LCDC.bgwintiledata ? 0 : 0x800;
    A = *(u16*)(&VRAM[A]);
    PACK16(A);
    bgwindata = A;

    bgwinx = 0;
    goto NextX;

ShiftBG:

    bgwindata >>=2;

NextX:

    x++;
    if (x < 168) {goto BGWINXLoop;}

    priority = true;
    goto StartSprites;

LinerEnd:

    return;

}
*/
