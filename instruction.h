#ifndef INSTRUCTION_H
#define INSTRUCTION_H

typedef struct{

} Instruction;

Instruction* newInstruction();

void initInstruction(Instruction* pt);

void deleteInstruction(Instruction* pt);

void freeInstruction(Instruction** ppt);

#endif
