#include <sys/types.h>
#include <stdlib.h>

typedef u_int8_t Byte;
typedef u_int16_t Word;

// opcodes
#define LDA_IM 0xA9;


typedef struct {
    u_int32_t MAX_MEM;
    Byte Data[];
} Memory;

typedef struct {
    Word PC;    // Program Counter
    Word SP;    // Stack Pointer

    Byte A, X, Y;    // Registers

    Byte C : 1; // carry flag
    Byte Z : 1; // zero flag
    Byte I : 1; // interrupt disable flag
    Byte D : 1; // decimal flag
    Byte B : 1; // break flag
    Byte V : 1; // overflow flag
    Byte N : 1; // negative flag
} CPU;

void cpu_reset(CPU *cpu) {
    cpu->PC = 0xFFFC;
    cpu->SP = 0x0100;
    cpu->A = cpu->X = cpu->Y = 0;
    cpu->C = cpu->Z = cpu->I = cpu->D = cpu->B = cpu->V = cpu->N = 0;
}

void init_mem(Memory *mem) {
    mem->MAX_MEM = 1024 * 64;
    mem->Data[mem->MAX_MEM] = 0;
}

Byte Fetch(u_int32_t *Cycles, CPU* cpu, Memory* mem) {
    Byte Data = mem->Data[cpu->PC];
    cpu->PC++;
    (*Cycles)--;
    return Data;
}

void Execute(u_int32_t Cycles, CPU* cpu, Memory* mem) {
    while(Cycles > 0) {
        Byte Ins = Fetch(&Cycles, cpu, mem);
        switch (Ins) {
            case LDA_IM:
            {
                break;
            }
        }
    }
}

int main(void) {
    CPU *cpu = (CPU *)malloc(sizeof(CPU));
    Memory *mem = (Memory *)malloc(sizeof(Memory));

    // init
    cpu_reset(cpu);
    init_mem(mem);
    Execute(2, cpu, mem);

    free(cpu);
	return 0;
}
