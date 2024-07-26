#ifndef OPCODES_H
#define OPCODES_H

// opcodes

#define ADC_IM 0x69
#define ADC_ZP 0x65
#define ADC_ZPX 0x75
#define ADC_ABS 0x6D
#define ADC_ABSX 0x7D
#define ADC_ABSY 0x79
#define ADC_INDX 0x61
#define ADC_INDY 0x71

#define AND_IM 0x29
#define AND_ZP 0x25
#define AND_ZPX 0x35
#define AND_ABS 0x2D
#define AND_ABSX 0x3D
#define AND_ABSY 0x39
#define AND_INDX 0x21
#define AND_INDY 0x31

#define ASL_ACC 0x0A
#define ASL_ZP 0x06
#define ASL_ZPX 0x16
#define ASL_ABS 0x0E
#define ASL_ABSX 0x1E

#define BCC_REL 0x90

#define BCS_REL 0xB0

#define BEQ_REL 0xF0

#define BIT_ZP 0x24
#define BIT_ABS 0x2C

#define BMI_REL 0x30

#define BNE_REL 0xD0

#define BPL_REL 0x10

#define BRK_IMPL 0x00

#define BVC_REL 0x50

#define BVS_REL 0x70

#define CLC_REL 0x18

#define CLD_IMPL 0xD8

#define CLI_IMPL 0x58

#define CLV_IMPL 0xB8

#define CMP_IM 0xC9
#define CMP_ZP 0xC5
#define CMP_ZPX 0xD5
#define CMP_ABS 0xCD
#define CMP_ABSX 0xDD
#define CMP_ABSY 0xD9
#define CMP_INDX 0xC1
#define CMP_INDY 0xD1

#define CPX_IM 0xE0
#define CPX_ZP 0xE4
#define CPX_ABS 0xEC

#define CPY_IM 0xC0
#define CPY_ZP 0xC4
#define CPY_ABS 0xCC

#define DEC_ZP 0xC6
#define DEC_ZPX 0xD6
#define DEC_ABS 0xCE
#define DEC_ABSX 0xDE

#define DEX_IMPL 0xCA

#define DEY_IMPL 0x88

#define EOR_IM 0x49
#define EOR_ZP 0x45
#define EOR_ZPX 0x55
#define EOR_ABS 0x4D
#define EOR_ABSX 0x5D
#define EOR_ABSY 0x59
#define EOR_INDX 0x41
#define EOR_INDY 0x51

#define INC_ZP 0xE6
#define INC_ZPX 0xF6
#define INC_ABS 0xEE
#define INC_ABSX 0xFE

#define INX_IMPL 0xE8

#define INY_IMPL 0xC8

#define JMP_ABS 0x4C
#define JMP_IND 0x6C

#define JSR_ABS 0x20

#define LDA_IM 0xA9
#define LDA_ZP 0xA5
#define LDA_ZPX 0xB5
#define LDA_ABS 0xAD
#define LDA_ABSX 0xBD
#define LDA_ABSY 0xB9
#define LDA_INDX 0xA1
#define LDA_INDY 0xB1

#define LDX_IM 0xA2
#define LDX_ZP 0xA6
#define LDX_ZPY 0xB6
#define LDX_ABS 0xAE
#define LDX_ABSY 0xBE

#define LDY_IM 0xA0
#define LDY_ZP 0xA4
#define LDY_ZPX 0xB4
#define LDY_ABS 0xAC
#define LDY_ABSX 0xBC

#define LSR_ACC 0x4A
#define LSR_ZP  0x46
#define LSR_ZPX 0x56
#define LSR_ABS 0x4E
#define LSR_ABSX 0x5E

#define NOP_IMPL 0xEA

#define ORA_IM 0x09
#define ORA_ZP 0x05
#define ORA_ZPX 0x15
#define ORA_ABS 0x0D
#define ORA_ABSX 0x1D
#define ORA_ABSY 0x19
#define ORA_INDX 0x01
#define ORA_INDY 0x11

#define PHA_IMPL 0x48

#define PHP_IMPL 0x08

#define PLA_IMPL 0x68

#define PLP_IMPL 0x28

#define ROL_ACC 0x2A
#define ROL_ZP 0x26
#define ROL_ZPX 0x36
#define ROL_ABS 0x2E
#define ROL_ABSX 0x3E

#define ROR_ACC 0x6A
#define ROR_ZP 0x66
#define ROR_ZPX 0x76
#define ROR_ABS 0x6E
#define ROR_ABSX 0x7E

#define RTI_IMPL 0x40

#define RTS_IMPL 0x60

#define SBC_IM 0xE9
#define SBC_ZP 0xE5
#define SBC_ZPX 0xF5
#define SBC_ABS 0xED
#define SBC_ABSX 0xFD
#define SBC_ABSY 0xF9
#define SBC_INDX 0xE1
#define SBC_INDY 0xF1

#define SEC_IMPL 0x38

#define SED_IMPL 0xF8

#define SEI_IMPL 0x78

#define STA_ZP 0x85
#define STA_ZPX 0x95
#define STA_ABS 0x8D
#define STA_ABSX 0x9D
#define STA_ABSY 0x99
#define STA_INDX 0x81
#define STA_INDY 0x91

#define STX_ZP 0x86
#define STX_ZPY 0x96
#define STX_ABS 0x8E

#define STY_ZP 0x84
#define STY_ZPX 0x94
#define STY_ABS 0x8C

#define TAX_IMPL 0xAA

#define TAY_IMPL 0xA8

#define TSX_IMPL 0xBA

#define TXA_IMPL 0x8A

#define TXS_IMPL 0x9A

#define TYA_IMPL 0x98

#endif
