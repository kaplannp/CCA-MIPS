#include<iostream>
#include<bitset>
/*
 * takes in the values of each subset of an R instruction, and returns the 
 * integer value of the instruction
 */
unsigned int constructRInstr(unsigned int rs, unsigned int rt, unsigned int rd,
    unsigned int shamt, unsigned int func){
  unsigned int instr = 0;
  instr += rs << 21;
  instr += rt << 16;
  instr += rd << 11;
  instr += shamt << 6;
  instr += func;
  return instr;
}


/*
 * takes in the values of each subset of an I instruction, and returns the 
 * integer value of the instruction
 */
unsigned int constructIInstr(unsigned int opcode, unsigned int rs,
    unsigned int rtOrRD, unsigned int val){
  unsigned int instr = 0;
  instr += opcode << 26;
  instr += rs << 21;
  instr += rtOrRD << 16;
  instr += val;
  return instr;
}

/*
 * takes in the values of each subset of an I instruction, and returns the 
 * integer value of the instruction
 */
unsigned int constructJInstr(unsigned int opcode, unsigned int val){
  unsigned int instr = 0;
  instr += opcode << 26;
  instr += val;
  return instr;
}


