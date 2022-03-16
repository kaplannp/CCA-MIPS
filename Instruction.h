#ifndef INSTRUCTION_INCLUDED
#define INSTRUCTION_INCLUDED
#include<string>
#include<bitset>
#include<unordered_map>
#include "Mem.h"

namespace instruction{
  typedef std::bitset<32> InstrBits;
  /*
   * Instruction class is used to handle the MIPS translation from bits into
   * useful information
   */
  class Instruction{
    private:
      //TODO would love to make this const and static
      std::unordered_map<std::bitset<6>, std::string>* OPCODE2STR;
      InstrBits instr;
      /*
       * Used to enable python like slicing of instructions.
       * returns a pointer to dynamically allocated slice.
       * params:
       *   start: the start index inclusive
       *   end: the end index not included
       * returns instr[start:end)
       */
      std::bitset<6>* getSlice(int start, int end) const;
      /*
       * params:
       *   opcode: the 6 high order bits organized 26:31. That is the opcode
       *     of the instruction.
       * returns: copy of string, the type of this instruction
       */
      std::string getType(const std::bitset<6>& opcode) const;
      /*
       * returns the value located in the register specified by the slice
       * params:
       *   addr: the address of the register
       * returns: the value in the memory at that location
       */
      mem::data32 loadReg(const std::bitset<6>& addr) const;

      //Initializes the opcode2string map
      std::unordered_map<std::bitset<6>, std::string>* initOpcode2Str();

    public:
      Instruction(unsigned int instr);
      /*
       * copy constructor necessary
       */
      //Instruction(const Instruction& instr);
      std::string toString();
      /*
       * Returns a copy of this instruction
       */
      InstrBits getInstr() const;
  };

  bool operator==(const Instruction& left, const Instruction& right);

}

std::ostream& operator<<(std::ostream& out, instruction::Instruction instr);
#endif
