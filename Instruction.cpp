#include "Instruction.h"

namespace instr{
 
  Instruction::Instruction(unsigned int instr): instr(instr){}

  //TODO there are better ways for dealing with bits in c++
  unsigned int Instruction::getOpcode(){
    return (instr >> 25);
  }

  InstructionType Instruction::getType(){
    return "Not Implemeneted";
  }

  std::string Instruction::toString(){
    return getType();
  }

}
