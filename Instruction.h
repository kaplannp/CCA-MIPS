#ifndef INSTRUCTION_INCLUDED
#define INSTRUCTION_INCLUDED
#include<string>

namespace instr{
  typedef std::string InstructionType;
  /*
   * Instruction class is used to handle the MIPS translation from bits into
   * useful information
   */
  class Instruction{
    private:
      unsigned int instr;
      InstructionType getType();
      unsigned int getOpcode();
       

    public:
      Instruction(unsigned int instr);
      std::string toString();

  };
}
#endif
