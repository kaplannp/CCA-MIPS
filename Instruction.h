#ifndef INSTRUCTION_INCLUDED
#define INSTRUCTION_INCLUDED
#include<string>
#include<bitset>
#include<unordered_map>
#include "Mem.h"
//#define BOOST_LOG_DYN_LINK
//#include <boost/log/trivial.hpp>
//#include <boost/log/attributes.hpp>
//#include <boost/log/common.hpp>
//#include <boost/log/core.hpp>
//#include <boost/log/exceptions.hpp>
//#include <boost/log/expressions.hpp>
//#include <boost/log/sinks.hpp>
//#include <boost/log/trivial.hpp>

namespace instruction{

  //TODO would love to not have this in header
  //Also, why does this work with const, but not without?
  const std::unordered_map<unsigned int, std::string> FUNC_2_MNEMONIC = 
    std::unordered_map<unsigned int, std::string>({
        {0x23,"subu"},
        {0x22,"sub"},
        {0x21,"addu"},
        {0x20, "add"},
        {0x0,"sll"},
        {0x4,"sllv"},
        {0x2,"srl"},
        {0x6,"srlv"},
        {0x8,"jr"},
        {0x24,"and"},
        {0x25,"or"},
        {0x26,"xor"},
        {0x27,"nor"},
        {0x2a,"slt"},
        {0x2b,"sltu"},
        {0x18,"mult"},
        {0x19,"multu"},
        {0x1a,"div"},
        {0x1b,"divu"},
        {0x10,"mfhi"},
        {0x12,"mflo"},
        {0x3,"sra"},
        {0x7,"srav"},
        {0xc,"syscall"},
        {0x11,"move"},
        {0x9,"jalr"},
        //completely unimplemented
        {0x1,"movef"}
        });

  typedef std::bitset<32> InstrBits;
  /*
   * Instruction class is used to handle the MIPS translation from bits into
   * useful information
   */
  class Instruction{
    private:
      static const std::unordered_map<std::bitset<6>, std::string> OPCODE2STR;
      InstrBits instr;

    public:
      Instruction(unsigned int instr);
      std::string toString() const;
      /*
       * Returns a copy of this instruction
       */
      InstrBits getInstr() const;
      /*
       * Used to enable python like slicing of instructions.
       * returns a pointer to dynamically allocated slice.
       * params:
       *   start: the start index inclusive
       *   end: the end index not included
       * returns instr[start:end)
       */
      template<int start, int end>
      std::bitset<end-start> getSlice() const{
        const size_t width = end - start;
        std::bitset<width> slice = std::bitset<width>(0);
        for(int i = start; i < end; i++){
          slice.set(i-start, instr[i]);
        }
        return slice;
      }
      /*
       * params:
       *   opcode: the 6 high order bits organized 26:31. That is the opcode
       *     of the instruction.
       * returns: copy of string, the type of this instruction
       */
      std::string getType() const;

      /*
       * returns the type of the function as specified in the hashtable
       * FUNC_2_MNEMONIC
       */
      std::string getFuncType() const;
  };

  bool operator==(const Instruction& left, const Instruction& right);

}

std::ostream& operator<<(std::ostream& out, instruction::Instruction instr);
#endif
