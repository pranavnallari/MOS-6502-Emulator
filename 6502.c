#include "opcodes.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdbool.h>

#define CLOCK_TIME 10000

typedef u_int8_t Byte;
typedef u_int16_t Word;

typedef struct {
    Byte Data[0x10000];
} Memory;

typedef struct {
    Word PC;    // Program Counter
    Byte SP;    // Stack Pointer

    Byte A, X, Y;    // Registers

    Byte C : 1; // carry flag (LSB)
    Byte Z : 1; // zero flag
    Byte I : 1; // interrupt disable flag
    Byte D : 1; // decimal flag
    Byte B : 1; // break flag
    Byte V : 1; // overflow flag
    Byte N : 1; // negative flag (MSB)
} CPU;

CPU cpu;
Memory mem;

void cpu_reset() {
    cpu.PC = (mem.Data[0xFFFD] << 8) | mem.Data[0xFFFC];
    cpu.SP = 0xFF;
    cpu.A = cpu.X = cpu.Y = 0;
    cpu.C = cpu.Z = cpu.I = cpu.D = cpu.B = cpu.V = cpu.N = 0;
}

void init_mem() {
    memset(mem.Data, 0, 0x10000);
}

Byte peek_stack() {
    return mem.Data[0x100 + cpu.SP];
}

void push_to_stack(Byte val) {
    mem.Data[0x0100 + cpu.SP] = val;
    cpu.SP--;
}

Byte pop_from_stack() {
    cpu.SP++;
    return mem.Data[0x0100 + cpu.SP];
}

Byte read_byte(Word addr) {
    return mem.Data[addr];
}

void write_byte(Word addr, Byte value) {
    mem.Data[addr] = value;
}

Word read_word(Word offset) {
    Word val = read_byte(offset) | (read_byte(offset + 1) << 8);
    return val;
}

Byte read_from_pc() {
    Byte val = read_byte(cpu.PC);
    cpu.PC++;
    return val;
}

void adc(CPU *cpu, Byte val) {
    Word result = cpu->A + val + cpu->C;

    // Set carry flag
    cpu->C = (result > 0xFF);

    // Set zero flag
    cpu->Z = ((result & 0xFF) == 0);

    // Set negative flag
    cpu->N = (result & 0x80) != 0;

    // Set overflow flag
    bool overflow = (~(cpu->A ^ val) & (cpu->A ^ result) & 0x80);
    cpu->V = overflow != 0;

    // Update accumulator
    cpu->A = result & 0xFF;
}

void and(CPU *cpu, Byte val) {
    cpu->A = cpu->A & val;
    // set flags
    cpu->Z = ((cpu->A & 0xFF) == 0);
    cpu->N = (cpu->A & 0x80) != 0;
}

void cmp (CPU *cpu, Byte val) {
    Byte ans = cpu->A - val;
    if (ans == 0x00) cpu->Z = 1;
    if (cpu->A >= val) cpu->C = 1;
    cpu->N = (ans & 0x80) != 0;
}

void cpx (CPU *cpu, Byte val) {
    Byte ans = cpu->X - val;
    if (ans == 0x00) cpu->Z = 1;
    if (cpu->X >= val) cpu->C = 1;
    cpu->N = (ans & 0x80) != 0;
}

void cpy (CPU *cpu, Byte val) {
    Byte ans = cpu->Y - val;
    if (ans == 0x00) cpu->Z = 1;
    if (cpu->Y >= val) cpu->C = 1;
    cpu->N = (ans & 0x80) != 0;
}

void dec(CPU *cpu, Word addr) {
    Byte val = read_byte(addr);
    val--;
    if (val == 0) cpu->Z = 1;
    cpu->N = (val & 0x80) != 0;
    write_byte(addr, val);
}

void eor(CPU *cpu, Byte val) {
    cpu->A = cpu->A ^ val;
    cpu->Z = (cpu->A == 0) ? 1 : 0;
    cpu->N = (cpu->A & 0x80) != 0;
}

void inc(CPU *cpu, Word addr) {
    Byte val = read_byte(addr);
    val = val + 1;
    cpu->Z = (val == 0) ? 1: 0;
    cpu->N = (val & 0x80) != 0;
    write_byte(addr, val);
}

void lda(CPU *cpu, Byte val) {
    cpu->A = val;
    cpu->Z = (cpu->A == 0) ? 1 : 0;
    cpu->N = (cpu->A & 0x80) != 0;
}

void ldx(CPU *cpu, Byte val) {
    cpu->X = val;
    cpu->Z = (cpu->X == 0) ? 1 : 0;
    cpu->N = (cpu->X & 0x80) != 0;
}

void ldy(CPU *cpu, Byte val) {
    cpu->Y = val;
    cpu->Z = (cpu->Y == 0) ? 1 : 0;
    cpu->N = (cpu->Y & 0x80) != 0;
}

void lsr(CPU *cpu, Word addr) {
    Byte val = read_byte(addr);
    val = val >> 1;
    cpu->C = ((val & 0x01) != 0);
    cpu->Z = (val == 0x00);
    cpu->N = (val & 0x80) != 0;
    write_byte(addr, val);
}

void ora(CPU *cpu, Byte val) {
    cpu->A = cpu->A | val;
    cpu->Z = (cpu->A == 0x00) ? 1 : 0;
    cpu->N = (cpu->A & 0x80) != 0;
}

void rol(CPU *cpu, Byte val) {
    cpu->C =  (val & 0x80) != 0;
    val = val << 1;
    cpu->Z = (val == 0x00);
    cpu->N = (val & 0x80) != 0;
}

void ror(CPU *cpu, Byte val) {
    cpu->C =  (val & 0x80) != 0;
    val = val >> 1;
    cpu->Z = (val == 0x00);
    cpu->N = (val & 0x80) != 0;
}

void sbc(CPU *cpu, Byte val) {
    Word result = (Word)cpu->A - (Word)val - (1 - cpu->C);
    cpu->Z = (result & 0xFF) == 0x00;
    cpu->N = (result & 0x80) != 0;
    cpu->C = (result < 0x100);
    cpu->V = (((cpu->A ^ result ) & (val ^ result)) & 0x80) != 0;
    cpu->A = (Byte)(result & 0xFF);

}

void sta(CPU *cpu, Word mem) {
    write_byte(mem, cpu->A);
}

void stx(CPU *cpu, Word mem) {
    write_byte(mem, cpu->X);
}

void sty(CPU *cpu, Word mem) {
    write_byte(mem, cpu->Y);
}

void print_debug() {
    printf("------------------------------\n");
    printf("PC : 0x%04X\n", cpu.PC);
    printf("Memory at PC : 0x%04X\n", read_byte(cpu.PC));
    printf("SP : 0x%04X\n", cpu.SP);
    printf("A : 0x%04X\n", cpu.A);
    printf("X : 0x%04X\n", cpu.X);
    printf("Y : 0x%04X\n", cpu.Y);
    printf("C : 0x%04X\n", cpu.C);
    printf("Z : 0x%04X\n", cpu.Z);
    printf("I : 0x%04X\n", cpu.I);
    printf("D : 0x%04X\n", cpu.D);
    printf("B : 0x%04X\n", cpu.B);
    printf("V : 0x%04X\n", cpu.V);
    printf("N : 0x%04X\n--------------------------------------\n", cpu.N);
}

void execute_instructions() {
    Byte opcode = read_from_pc();
    switch(opcode) {
        case ADC_IM: {
            printf("ADC_IM\n");
            Byte val = read_from_pc();
            adc(&cpu, val);
            break;
        }
        case ADC_ZP: {
            printf("ADC_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            adc(&cpu, val);
            break;
        }
        case ADC_ZPX: {
            printf("ADC_ZPX\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr + cpu.X);
            adc(&cpu, val);
            break;
        }
        case ADC_ABS: {
            printf("ADC_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            adc(&cpu, val);
            break;
        }
        case ADC_ABSX: {
            printf("ADC_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_byte(addr);
            adc(&cpu, val);
            break;
        }
        case ADC_ABSY: {
            printf("ADC_ABSY\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.Y;
            Byte val = read_byte(addr);
            adc(&cpu, val);
            break;
        }
        case ADC_INDX: {
            printf("ADC_INDX\n");
            Word addr = (Word)read_from_pc();
            addr += (Word)cpu.X;
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr + 1);
            addr = ((high << 8) | low);
            Byte val = read_byte(addr);
            adc(&cpu, val);
            break;
        }
        case ADC_INDY: {
            printf("ADC_INDY\n");
            Word addr = (Word)read_from_pc();
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr);
            addr = ((high << 8) | low) + (Word)cpu.Y;
            Byte val = read_byte(addr);
            adc(&cpu, val);
            break;
        }

        case AND_IM: {
            printf("AND_IM\n");
            Byte val = read_from_pc();
            and(&cpu, val);
            break;
        }
        case AND_ZP: {
            printf("AND_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            adc(&cpu, val);
            break;
        }
        case AND_ZPX: {
            printf("AND_ZPX\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr + cpu.X);
            adc(&cpu, val);
            break;
        }
        case AND_ABS: {
            printf("AND_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            and(&cpu, val);
            break;
        }
        case AND_ABSX: {
            printf("AND_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_byte(addr);
            and(&cpu, val);
            break;
        }
        case AND_ABSY: {
            printf("AND_ABSY");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.Y;
            Byte val = read_byte(addr);
            and(&cpu, val);
            break;
        }
        case AND_INDX: {
            printf("AND_INDX\n");
            Word addr = (Word)read_from_pc();
            addr += (Word)cpu.X;
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr + 1);
            addr = ((high << 8) | low);
            Byte val = read_byte(addr);
            and(&cpu, val);
            break;
        }
        case AND_INDY: {
            printf("AND_INDY\n");
            Word addr = (Word)read_from_pc();
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr);
            addr = ((high << 8) | low) + (Word)cpu.Y;
            Byte val = read_byte(addr);
            and(&cpu, val);
            break;
        }
        case ASL_ACC: {
            printf("ASL_ACC\n");
            cpu.C =  (cpu.A & 0x80) != 0;
            cpu.A = cpu.A << 1;
            cpu.Z = (cpu.A & 0xFF) == 0;
            cpu.N = (cpu.A & 0x80) != 0;
            break;
        }
        case ASL_ZP: {
            printf("ASL_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            cpu.C = (val & 0x80) != 0;
            val = val << 1;
            cpu.Z = (val & 0xFF) == 0;
            cpu.N = (val & 0x80) != 0;
            write_byte(addr, val);
            break;
        }
        case ASL_ZPX: {
            printf("ASL_ZPX\n");
            Word addr = (Word)read_from_pc();
            addr += cpu.X;
            Byte val = read_byte(addr);
            cpu.C = (val & 0x80) != 0;
            val = val << 1;
            cpu.Z = (val & 0xFF) == 0;
            cpu.N = (val & 0x80) != 0;
            write_byte(addr, val);
            break;
        }
        case ASL_ABS: {
            printf("ASL_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            cpu.C = (val & 0x80) != 0;
            val = val << 1;
            cpu.Z = (val & 0xFF) == 0;
            cpu.N = (val & 0x80) != 0;
            write_byte(addr, val);
            break;
        }
        case ASL_ABSX: {
            printf("ASL_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_byte(addr);
            cpu.C = (val & 0x80) != 0;
            val = val << 1;
            cpu.Z = (val & 0xFF) == 0;
            cpu.N = (val & 0x80) != 0;
            write_byte(addr, val);
            break;
        }
        case BCC_REL: {
            printf("BCC_REL\n");
            Byte val = read_from_pc();
            if (val & 0x80) val |= 0xFFFFFF00;
            if (cpu.C == 0) cpu.PC += (Byte)val;
            break;
        }
        case BCS_REL: {
            printf("BCS_REL\n");
            Byte val = read_from_pc();
            if (val & 0x80) val |= 0xFFFFFF00;
            if (cpu.PC == 1) cpu.PC += (Byte)val;
            break;
        }
        case BEQ_REL: {
            printf("BEQ_REL\n");
            Byte val = read_from_pc();
            if (val & 0x80) val |= 0xFFFFFF00;
            if (cpu.Z == 1) cpu.PC += (Byte)val;
            break;
        }
        case BIT_ZP: {
           printf("BIT_ZP\n");
           Word addr = (Word)read_from_pc();
           Byte val = read_byte(addr);
           Byte temp = cpu.A & val;
           cpu.Z = temp == 0;
           cpu.V = (val & 0b01000000) != 0;
           cpu.N = (val & 0b10000000) != 0;
           break;
        }
        case BIT_ABS: {
            printf("BIT_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            Byte temp = cpu.A & val;
            cpu.Z = temp == 0;
            cpu.V = (val & 0b01000000) != 0;
            cpu.N = (val & 0b10000000) != 0;
            break;
        }
        case BMI_REL: {
            printf("BMI_REL\n");
            Byte val = read_from_pc();
            if (val & 0x80) val |= 0xFFFFFF00;
            if (cpu.N == 1) cpu.PC += (Byte)val;
            break;
        }
        case BNE_REL: {
            printf("BNE_REL\n");
            Byte val = read_from_pc();
            if (val & 0x80) val |= 0xFFFFFF00;
            if (cpu.Z == 0) cpu.PC += (Byte)val;
            break;
        }
        case BPL_REL: {
            printf("BPL_REL\n");
            Byte val = read_from_pc();
            if (val & 0x80) val |= 0xFFFFFF00;
            if (cpu.N == 0) cpu.PC += (Byte)val;
            break;
        }
        case BRK_IMPL: {
            printf("BRK_IMPL\n");
            push_to_stack(((cpu.PC + 2) >> 8) & 0xFF);
            push_to_stack((cpu.PC+2) & 0xFF);

            Byte Status = ((cpu.N) << 7) | (cpu.V << 6) | (1 << 5) | (cpu.B << 4) | (cpu.D << 3) | (cpu.I << 2) | (cpu.Z << 1) | (cpu.C << 0);
            Status |= 0x10;
            push_to_stack(Status);

            cpu.I = 1;
            cpu.B = 1;

            Byte low = read_byte(0xFFFE);
            Byte high = read_byte(0xFFFF);
            cpu.PC = (high << 8) | low;
            break;
        }
        case BVC_REL: {
            printf("BVC_REL\n");
            Byte val = read_from_pc();
            if (val & 0x80) val |= 0xFFFFFF00;
            if (cpu.V == 0) cpu.PC += (Byte)val;
            break;
        }
        case BVS_REL: {
            printf("BVS_REL\n");
            Byte val = read_from_pc();
            if (val & 0x80) val |= 0xFFFFFF00;
            if (cpu.V == 1) cpu.PC += (Byte)val;
            break;
        }
        case CLC_IMPL: {
            printf("CLC_IMPL\n");
            cpu.C = 0;
            break;
        }
        case CLD_IMPL: {
            printf("CLD_IMPL\n");
            cpu.D = 0;
            break;
        }
        case CLI_IMPL: {
            printf("CLI_IMPL\n");
            cpu.I = 0;
            break;
        }
        case CLV_IMPL: {
            printf("CLV_IMPL\n");
            cpu.V = 0;
            break;
        }
        case CMP_IM: {
            printf("CMP_IM\n");
            Byte val = read_from_pc();
            cmp(&cpu, val);
            break;
        }
        case CMP_ZP: {
            printf("CMP_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            cmp(&cpu, val);
            break;
        }
        case CMP_ZPX: {
            printf("CMP_ZPX\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr + cpu.X);
            cmp(&cpu, val);
            break;
        }
        case CMP_ABS: {
            printf("CMP_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            cmp(&cpu, val);
            break;
        }
        case CMP_ABSX: {
            printf("CMP_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_byte(addr);
            cmp(&cpu, val);
            break;
        }
        case CMP_ABSY: {
            printf("CMP_ABSY\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.Y;
            Byte val = read_word(addr);
            cmp(&cpu, val);
            break;
        }
        case CMP_INDX: {
            printf("CMP_INDX\n");
            Word addr = (Word)read_from_pc();
            addr += (Word)cpu.X;
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr + 1);
            addr = ((high << 8) | low);
            Byte val = read_byte(addr);
            cmp(&cpu, val);
            break;
        }
        case CMP_INDY: {
            printf("CMP_INDY\n");
            Word addr = (Word)read_from_pc();
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr);
            addr = ((high << 8) | low) + (Word)cpu.Y;
            Byte val = read_byte(addr);
            cmp(&cpu, val);
            break;
        }
        case CPX_IM: {
            printf("CPX_IM\n");
            Byte val = read_from_pc();
            cpx(&cpu, val);
            break;
        }
        case CPX_ZP: {
            printf("CPX_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            cpx(&cpu, val);
            break;
        }
        case CPX_ABS: {
            printf("CPX_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            cpx(&cpu, val);
            break;
        }
        case CPY_IM: {
            printf("CPY_IM\n");
            Byte val = read_from_pc();
            cpy(&cpu, val);
            break;
        }
        case CPY_ZP: {
            printf("CPY_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            cpy(&cpu, val);
            break;
        }
        case CPY_ABS: {
            printf("CPY_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            cpy(&cpu, val);
            break;
        }
        case DEC_ABS: {
            printf("DEC_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            dec(&cpu, addr);
            break;
        }
        case DEC_ZP: {
            printf("DEC_ZP\n");
            Word addr = (Word)read_from_pc();
            dec(&cpu, addr);
            break;
        }
        case DEC_ZPX: {
            printf("DEC_ZPX\n");
            Word addr = (Word)read_from_pc();
            dec(&cpu, addr + cpu.X);
            break;
        }
        case DEC_ABSX: {
            printf("DEC_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            addr += cpu.X;
            dec(&cpu, addr);
            break;
        }
        case DEX_IMPL: {
            printf("DEX_IMPL\n");
            cpu.X--;
            if (cpu.X == 0) cpu.Z = 1;
            cpu.N = (cpu.X & 0x80) != 0;
            break;
        }
        case DEY_IMPL: {
            printf("DEY_IMPL\n");
            cpu.Y--;
            cpu.Z = (cpu.X == 0)? 1: 0;
            cpu.N = (cpu.X & 0x80) != 0;
            break;
        }
        case EOR_IM: {
            printf("EOR_IM\n");
            Byte val = read_from_pc();
            eor(&cpu, val);
            break;
        }
        case EOR_ZP: {
            printf("EOR_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            eor(&cpu, val);
            break;
        }
        case EOR_ZPX: {
            printf("EOR_ZPX\n");
            Word addr = (Word)read_from_pc();
            addr += cpu.X;
            Byte val = read_byte(addr);
            eor(&cpu, val);
            break;
        }
        case EOR_ABS: {
            printf("EOR_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            eor(&cpu, val);
            break;
        }
        case EOR_ABSX: {
            printf("EOR_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_byte(addr);
            eor(&cpu, val);
            break;
        }
        case EOR_ABSY: {
            printf("EOR_ABSY\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.Y;
            Byte val = read_byte(addr);
            eor(&cpu, val);
            break;
        }
        case EOR_INDX: {
            printf("EOR_INDX\n");
            Word addr = (Word)read_from_pc();
            addr += (Word)cpu.X;
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr + 1);
            addr = ((high << 8) | low);
            Byte val = read_byte(addr);
            eor(&cpu, val);
            break;
        }
        case EOR_INDY: {
            printf("EOR_INDY\n");
            Word addr = (Word)read_from_pc();
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr);
            addr = ((high << 8) | low) + (Word)cpu.Y;
            Byte val = read_byte(addr);
            eor(&cpu, val);
            break;
        }
        case INC_ABS: {
            printf("INC_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            inc(&cpu, addr);
            break;
        }
        case INC_ZP: {
            printf("INC_ZP\n");
            Word addr = (Word)read_from_pc();
            inc(&cpu, addr);
            break;
        }
        case INC_ZPX: {
            printf("INC_ZPX\n");
            Word addr = (Word)read_from_pc();
            addr += cpu.X;
            inc(&cpu, addr);
            break;
        }
        case INC_ABSX: {
            printf("INC_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            inc(&cpu, addr);
            break;
        }
        case INX_IMPL: {
            printf("INX_IMPL\n");
            cpu.X++;
            cpu.Z = (cpu.X == 0) ? 1: 0;
            cpu.N = ((cpu.X & 0x80) != 0);
            break;
        }
        case INY_IMPL: {
            printf("INY_IMPL\n");
            cpu.Y++;
            cpu.Z = (cpu.X == 0) ? 1 : 0;
            cpu.N = ((cpu.X & 0x80) != 0);
            break;
        }
        case JMP_ABS: {
            printf("JMP_ABS\n");
            Word addr1 = (Word)read_from_pc();
            Byte addr2 = read_from_pc();
            addr1 = (addr1) | (addr2 << 8);
            cpu.PC = addr1;
            break;
        }
        case JMP_IND: {
            printf("JMP_IND\n");
            Word addr = (Word)read_from_pc();
            Byte addr2 = read_from_pc();
            addr = (addr) | (addr2 << 8);
            Word val = read_word(addr);
            cpu.PC = val;
            break;
        }
        case JSR_ABS: {
            printf("JSR_ABS\n");
            Word addr1 = (Word)read_from_pc();
            Byte addr2 = read_from_pc();
            addr1 = (addr1) | (addr2 << 8);
            cpu.PC = addr1;
            Word ret_addr = cpu.PC - 1;
            push_to_stack((Byte)((ret_addr >> 8) & 0xFF));
            push_to_stack((Byte)(ret_addr & 0xFF));
            break;
        }
        case LDA_IM: {
            printf("LDA_IM\n");
            Byte val = read_from_pc();
            lda(&cpu, val);
            break;
        }
        case LDA_ZP: {
            printf("LDA_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            lda(&cpu, val);
            break;
        }
        case LDA_ZPX: {
            printf("LDA_ZPX\n");
            Word addr = (Word)read_from_pc();
            addr += cpu.X;
            Byte val = read_byte(addr);
            lda(&cpu, val);
            break;
        }
        case LDA_ABS: {
            printf("LDA_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            lda(&cpu, val);
            break;
        }
        case LDA_ABSX: {
            printf("LDA_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_byte(addr);
            lda(&cpu, val);
            break;
        }
        case LDA_ABSY: {
            printf("LDA_ABSY\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.Y;
            Byte val = read_byte(addr);
            lda(&cpu, val);
            break;
        }
        case LDA_INDX: {
            printf("LDA_INDX\n");
            Word addr = (Word)read_from_pc();
            addr += (Word)cpu.X;
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr + 1);
            addr = ((high << 8) | low);
            Byte val = read_byte(addr);
            lda(&cpu, val);
            break;
        }
        case LDA_INDY: {
            printf("LDA_INDY\n");
            Word addr = (Word)read_from_pc();
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr);
            addr = ((high << 8) | low) + (Word)cpu.Y;
            Byte val = read_byte(addr);
            lda(&cpu, val);
            break;
        }
        case LDX_IM: {
            printf("LDX_IM\n");
            Byte val = read_from_pc();
            ldx(&cpu, val);
            break;
        }
        case LDX_ZP: {
            printf("LDX_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            ldx(&cpu, val);
            break;
        }
        case LDX_ZPY: {
            printf("LDX_ZPY\n");
            Word addr = (Word)read_from_pc();
            addr += cpu.Y;
            Byte val = read_byte(addr);
            ldx(&cpu, val);
            break;
        }
        case LDX_ABS: {
            printf("LDX_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            ldx(&cpu, val);
            break;
        }
        case LDX_ABSY: {
            printf("LDX_ABSY\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.Y;
            Byte val = read_byte(addr);
            ldx(&cpu, val);
            break;
        }
        case LDY_IM: {
            printf("LDY_IM\n");
            Byte val = read_from_pc();
            ldy(&cpu, val);
            break;
        }
        case LDY_ZP: {
            printf("LDY_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            ldy(&cpu, val);
            break;
        }
        case LDY_ZPX: {
            printf("LDY_ZPY\n");
            Word addr = (Word)read_from_pc();
            addr += cpu.X;
            Byte val = read_byte(addr);
            ldy(&cpu, val);
            break;
        }
        case LDY_ABS: {
            printf("LDY_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            ldy(&cpu, val);
            break;
        }
        case LDY_ABSX: {
            printf("LDY_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_byte(addr);
            ldy(&cpu, val);
            break;
        }
        case LSR_ACC: {
            printf("LSR_ACC\n");
            cpu.A = cpu.A >> 1;
            cpu.C = ((cpu.A & 0x01) != 0);
            cpu.Z = (cpu.A == 0x00);
            cpu.N = (cpu.A & 0x80) != 0;
            break;
        }
        case LSR_ABS: {
            printf("LSR_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            lsr(&cpu, addr);
            break;
        }
        case LSR_ABSX: {
            printf("LSR_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            lsr(&cpu, addr);
            break;
        }
        case LSR_ZP: {
            printf("LSR_ZP\n");
            Word addr = (Word)read_from_pc();
            lsr(&cpu, addr);
            break;
        }
        case LSR_ZPX: {
            printf("LSR_ZPX\n");
            Word addr = (Word)read_from_pc();
            addr += cpu.X;
            lsr(&cpu, addr);
            break;
        }
        case NOP_IMPL: {
            printf("NOP_IMPL\n");
            break;
        }
        case ORA_IM: {
            printf("ORA_IM\n");
            Byte val = read_from_pc();
            ora(&cpu, val);
            break;
        }
        case ORA_ZP: {
            printf("ORA_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            ora(&cpu, val);
            break;
        }
        case ORA_ZPX: {
            printf("ORA_ZPX\n");
            Word addr = (Word)read_from_pc();
            addr += cpu.Y;
            Byte val = read_byte(addr);
            ora(&cpu, val);
            break;
        }
        case ORA_ABS: {
            printf("ORA_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            ora(&cpu, val);
            break;
        }
        case ORA_ABSX: {
            printf("ORA_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_byte(addr);
            ora(&cpu, val);
            break;
        }
        case ORA_ABSY: {
            printf("ORA_ABSY\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.Y;
            Byte val = read_byte(addr);
            ora(&cpu, val);
            break;
        }
        case ORA_INDX: {
            printf("ORA_INDX\n");
            Word addr = (Word)read_from_pc();
            addr += (Word)cpu.X;
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr + 1);
            addr = ((high << 8) | low);
            Byte val = read_byte(addr);
            ora(&cpu, val);
            break;
        }
        case ORA_INDY: {
            printf("ORA_INDY\n");
            Word addr = (Word)read_from_pc();
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr);
            addr = ((high << 8) | low) + (Word)cpu.Y;
            Byte val = read_byte(addr);
            ora(&cpu, val);
            break;
        }
        case PHA_IMPL: {
            printf("PHA_IMPL\n");
            push_to_stack(cpu.A);
            break;
        }
        case PHP_IMPL: {
            printf("PHP_IMPL\n");
            push_to_stack((Byte)(cpu.N << 7 | cpu.V << 6 | 1 << 5 | cpu.B << 4 | cpu.D << 3 | cpu.I << 2 | cpu.Z << 1 | cpu.C));
            break;
        }
        case PLA_IMPL: {
            printf("PLA_IMPL\n");
            cpu.A = pop_from_stack();
            cpu.Z = (cpu.A == 0x00) ? 1 : 0;
            cpu.N = (cpu.A & 0x80) != 0;
            break;
        }
        case PLP_IMPL: {
            printf("PLP_IMPL\n");
            Byte val = pop_from_stack();
            cpu.N = (val & (1 << 7)) != 0;
            cpu.V = (val & (1 << 6)) != 0;
            cpu.B = (val & (1 << 4)) != 0;
            cpu.D = (val & (1 << 3)) != 0;
            cpu.I = (val & (1 << 2)) != 0;
            cpu.Z = (val & (1 << 1)) != 0;
            cpu.C = (val & (1)) != 0;
            break;
        }
        case ROL_ACC: {
            printf("ROL_ACC\n");
            cpu.C =  (cpu.A & 0x80) != 0;
            cpu.A = cpu.A << 1;
            cpu.Z = (cpu.A == 0x00);
            cpu.N = (cpu.A & 0x80) != 0;
            break;
        }
        case ROL_ZP: {
            printf("ROL_ZP\n");
            Word addr = (Word) read_from_pc();
            Byte val = read_byte(addr);
            rol(&cpu, val);
            break;
        }
        case ROL_ZPX: {
            printf("ROL_ZPX\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr + cpu.X);
            rol(&cpu, val);
            break;
        }
        case ROL_ABS: {
            printf("ROL_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            rol(&cpu, val);
            break;
        }
        case ROL_ABSX: {
            printf("ROL_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_byte(addr);
            rol(&cpu, val);
            break;
        }
        case ROR_ACC: {
            printf("ROR_ACC\n");
            cpu.C =  (cpu.A & 0x80) != 0;
            cpu.A = cpu.A >> 1;
            cpu.Z = (cpu.A == 0x00);
            cpu.N = (cpu.A & 0x80) != 0;
            break;
        }
        case ROR_ZP: {
            printf("ROR_ZP\n");
            Word addr = (Word) read_from_pc();
            Byte val = read_byte(addr);
            ror(&cpu, val);
            break;
        }
        case ROR_ZPX: {
            printf("ROR_ZPX\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr + cpu.X);
            ror(&cpu, val);
            break;
        }
        case ROR_ABS: {
            printf("ROR_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            ror(&cpu, val);
            break;
        }
        case ROR_ABSX: {
            printf("ROR_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_byte(addr);
            ror(&cpu, val);
            break;
        }
        case RTI_IMPL: {
            printf("RTI_IMPL\n");
            Byte val = pop_from_stack();
            cpu.N = (val & (1 << 7)) != 0;
            cpu.V = (val & (1 << 6)) != 0;
            cpu.B = (val & (1 << 4)) != 0;
            cpu.D = (val & (1 << 3)) != 0;
            cpu.I = (val & (1 << 2)) != 0;
            cpu.Z = (val & (1 << 1)) != 0;
            cpu.C = (val & (1)) != 0;

            Byte low = pop_from_stack();
            Byte high = pop_from_stack();
            cpu.PC = ((high << 8) | low);
            break;
        }
        case RTS_IMPL: {
            printf("RTS_IMPL\n");
            Byte low = pop_from_stack();
            Byte high = pop_from_stack();
            cpu.PC = ((high << 8) | low) + 1;
            break;
        }
        case SBC_IM: {
            printf("SBC_IM\n");
            Byte val = read_from_pc();
            sbc(&cpu, val);
            break;
        }
        case SBC_ZP: {
            printf("SBC_ZP\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr);
            sbc(&cpu, val);
            break;
        }
        case SBC_ZPX: {
            printf("SBC_ZPX\n");
            Word addr = (Word)read_from_pc();
            Byte val = read_byte(addr + cpu.X);
            sbc(&cpu, val);
            break;
        }
        case SBC_ABS: {
            printf("SBC_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_byte(addr);
            sbc(&cpu, val);
            break;
        }
        case SBC_ABSX: {
            printf("SBC_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_byte(addr);
            sbc(&cpu, val);
            break;
        }
        case SBC_ABSY: {
            printf("SBC_ABSY\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.Y;
            Byte val = read_word(addr);
            cmp(&cpu, val);
            break;
        }
        case SBC_INDX: {
            printf("SBC_INDX\n");
            Word addr = (Word)read_from_pc();
            addr += (Word)cpu.X;
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr + 1);
            addr = ((high << 8) | low);
            Byte val = read_byte(addr);
            sbc(&cpu, val);
            break;
        }
        case SBC_INDY: {
            printf("SBC_INDY\n");
            Word addr = (Word)read_from_pc();
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr);
            addr = ((high << 8) | low) + (Word)cpu.Y;
            Byte val = read_byte(addr);
            sbc(&cpu, val);
            break;
        }
        case SEC_IMPL: {
            printf("SEC_IMPL\n");
            cpu.C = 1;
            break;
        }
        case SED_IMPL: {
            printf("SED_IMPL\n");
            cpu.D = 1;
            break;
        };
        case SEI_IMPL: {
            printf("SEI_IMPL\n");
            cpu.I = 1;
            break;
        }
        case STA_ZP: {
            printf("STA_ZP\n");
            Word addr = (Word)read_from_pc();
            sta(&cpu, addr);
            break;
        }
        case STA_ZPX: {
            printf("STA_ZPX\n");
            Word addr = (Word)read_from_pc() + cpu.X;
            sta(&cpu, addr);
            break;
        }
        case STA_ABS: {
            printf("STA_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            sta(&cpu, addr);
            break;
        }
        case STA_ABSX: {
            printf("STA_ABSX\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            sta(&cpu, addr);
            break;
        }
        case STA_ABSY: {
            printf("STA_ABSY\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.Y;
            sta(&cpu, addr);
            break;
        }
        case STA_INDX: {
            printf("STA_INDX\n");
            Word addr = (Word)read_from_pc();
            addr += (Word)cpu.X;
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr + 1);
            addr = ((high << 8) | low);
            sta(&cpu, addr);
            break;
        }
        case STA_INDY: {
            printf("STA_INDY\n");
            Word addr = (Word)read_from_pc();
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr);
            addr = ((high << 8) | low) + (Word)cpu.Y;
            sta(&cpu, addr);
            break;
        }
        case STX_ZP: {
            printf("STX_ZP\n");
            Word addr = (Word)read_from_pc();
            stx(&cpu, addr);
            break;
        }
        case STX_ZPY: {
            printf("STX_ZPY\n");
            Word addr = (Word)read_from_pc();
            addr += cpu.Y;
            stx(&cpu, addr);
            break;
        }
        case STX_ABS: {
            printf("STX_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            stx(&cpu, addr);
            break;
        }
        case STY_ZP: {
            printf("STY_ZP\n");
            Word addr = (Word)read_from_pc();
            sty(&cpu, addr);
            break;
        }
        case STY_ZPX: {
            printf("STY_ZPY\n");
            Word addr = (Word)read_from_pc();
            addr += cpu.X;
            sty(&cpu, addr);
            break;
        }
        case STY_ABS: {
            printf("STY_ABS\n");
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            sty(&cpu, addr);
            break;
        }
        case TAX_IMPL: {
            printf("TAX_IMPL\n");
            cpu.X = cpu.A;
            cpu.Z = (cpu.X == 0x00);
            cpu.N = ((cpu.X & 0x80) != 0);
            break;
        }
        case TAY_IMPL: {
            printf("TAY_IMPL\n");
            cpu.Y = cpu.A;
            cpu.Z = (cpu.Y == 0x00);
            cpu.N = ((cpu.Y & 0x80) != 0);
            break;
        }
        case TSX_IMPL: {
            printf("TSX_IMPL\n");
            cpu.X = peek_stack();
            cpu.Z = (cpu.X == 0x00);
            cpu.N = (cpu.X & 0x80) != 0;
            break;
        }
        case TXA_IMPL: {
            printf("TXA_IMPL\n");
            cpu.A = cpu.X;
            cpu.Z = (cpu.A == 0x00);
            cpu.N = (cpu.A & 0x80) != 0;
            break;
        }
        case TXS_IMPL: {
            printf("TXS_IMPL\n");
            push_to_stack(cpu.X);
            break;
        }
        case TYA_IMPL: {
            printf("TYA_IMPL\n");
            cpu.A = cpu.Y;
            cpu.Z = (cpu.A == 0x00);
            cpu.N = (cpu.A & 0x80) != 0;
            break;
        }
        default: {
            printf("Unhandled Opcode : 0x%04X\n", opcode);
            break;
        }

    }
}

int main(void) {
    fflush(stdout);
    init_mem();
    /* debug data */
    mem.Data[0xFFFC] = 0x00; // Low byte of reset vector
    mem.Data[0xFFFD] = 0x10; // High byte of reset vector

    mem.Data[0x1000] = ADC_INDX;
    mem.Data[0x0014] = 0x20;
    mem.Data[0x0015] = 0x30;
    mem.Data[0x3020] = 0x55;
    mem.Data[0x1001] = 0x10;



    /*------------------------------------------- */
    cpu_reset();

    print_debug();
    int i = 2;
    while (i > 0) {
        execute_instructions();
        print_debug();
        i--;
    }

    return 0;
}
