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

    Byte C : 1; // carry flag
    Byte Z : 1; // zero flag
    Byte I : 1; // interrupt disable flag
    Byte D : 1; // decimal flag
    Byte B : 1; // break flag
    Byte V : 1; // overflow flag
    Byte N : 1; // negative flag
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

void set_pc(Word val) {
    cpu.PC = val;
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
    cpu->V = overflow;

    // Update accumulator
    cpu->A = result & 0xFF;
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
            Byte val = read_byte(addr) + cpu.X;
            adc(&cpu, val);
            break;
        }
        case ADC_ABS: {
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = (addr << 8) | addr2;
            Byte val = read_word(addr);
            adc(&cpu, val);
            break;
        }
        case ADC_ABSX: {
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.X;
            Byte val = read_word(addr);
            adc(&cpu, val);
            break;
        }
        case ADC_ABSY: {
            Word addr = (Word)read_from_pc();
            Word addr2 = (Word)read_from_pc();
            addr = ((addr << 8) | addr2) + cpu.Y;
            Byte val = read_word(addr);
            adc(&cpu, val);
            break;
        }
        case ADC_INDX: {
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
            Word addr = (Word)read_from_pc();
            Word low = (Word)read_byte(addr);
            Word high = (Word)read_byte(addr);
            addr = ((high << 8) | low) + (Word)cpu.Y;
            Byte val = read_byte(addr);
            adc(&cpu, val);
            break;
        }
        default: {
            printf("Unhandled Opcode\n");
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
