#include<iostream>
#include<bitset>
/*
 * takes in the values of each subset of instruction, and returns the integer
 * value of the instruction
 */
unsigned int constructRInstr(int rs, int rt, int rd, int shamt, 
    int func){
  unsigned int instr = 0;
  instr += rs << 21;
  instr += rt << 16;
  instr += rd << 11;
  instr += shamt << 26;
  instr += func;
  return instr;
}
