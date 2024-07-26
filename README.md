# MOS 6502 Emulator in C

This project is a simple emulator for the 6502 microprocessor written in C. It supports all the valid operations

## Features

- **16-bit Address Space**: Emulates the 16-bit address range of the 6502, from `0x0000` to `0xFFFF`.
- **8-bit Data Storage**: Each addressable location in memory stores an 8-bit value.
- **Registers**: Emulates the 6502 registers: Accumulator (A), Index Registers (X and Y), Program Counter (PC), Stack Pointer (SP), and Status Flags (C, Z, I, D, B, V, N).
- **Memory Initialization**: Initializes a 64KB memory space and supports reading and writing bytes and words.
- **Addressing Modes**: Implements all addressing modes supported by 6502
- **Basic Instruction Execution**: Executes basic instructions like LDA (Load Accumulator).
- **Note**: Does not support illegal opcodes
- 
## Acknowledgements

- This emulator is inspired by the classic 6502 microprocessor.
- Special thanks to the [6502.org](http://6502.org) community for extensive documentation and resources.


## References

- http://6502.org/users/obelisk/
- https://www.masswerk.at/6502/6502_instruction_set.html#ADC
- https://en.wikipedia.org/wiki/MOS_Technology_6502
---
