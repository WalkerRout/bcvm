#ifndef OPCODE_H
#define OPCODE_H

enum OpCode {
  OPCODE_CONSTANT,      // 8 bits
  OPCODE_CONSTANT_LONG, // 24 bits
  OPCODE_RETURN,
};

#endif // OPCODE_H