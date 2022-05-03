#define BOOST_LOG_DYN_LINK
#include <boost/log/trivial.hpp>
#include "Instruction.h"
using namespace std; 

namespace instruction{
 

  InstrBits Instruction::getInstr() const{
    return instr;
  }

  Instruction::Instruction(unsigned int instr): instr(instr){}

  std::string Instruction::getType() const{
    //TODO destroy here?
    const std::bitset<6> opcode = getSlice<26,32>();
    try{
      return Instruction::OPCODE2STR.at(opcode);
    } catch (exception& e){
      BOOST_LOG_TRIVIAL(fatal) << "error " << e.what() << endl;
      BOOST_LOG_TRIVIAL(fatal) << "invalid opcode " << hex << opcode.to_ulong()
        << endl;
      throw e;
    }
    //should never get here 
  }

  
  std::string Instruction::getFuncType() const{
    std::bitset<6> funcBits = getSlice<0,6>();
    try{
      std::string func = FUNC_2_MNEMONIC.at((unsigned int) funcBits.to_ulong());
      return func;
    } catch (exception& e){
      BOOST_LOG_TRIVIAL(fatal) << "error " << e.what() << endl;
      BOOST_LOG_TRIVIAL(fatal) << "invalid function code for r instruction, "
        << "0x" << hex << funcBits.to_ulong() << endl;
      throw e;
    }
    //You should never reach here
  }

  std::string Instruction::toString() const{
    return getType() + " " + instr.to_string();
  }

  
//Initialize OPCODE
  const std::unordered_map<std::bitset<6>, std::string> Instruction::OPCODE2STR = 
    std::unordered_map<std::bitset<6>, std::string>(
      {
        {std::bitset<6>(0), "R-Type"},
        {std::bitset<6>(0x2), "J-Type:j"},
        {std::bitset<6>(0x3), "J-Type:jal"},
        {std::bitset<6>(0x4), "I-Type:beq"},
        {std::bitset<6>(0x5), "I-Type:bne"},
        {std::bitset<6>(0x8), "I-Type:addi"},
        {std::bitset<6>(0x9), "I-Type:addiu"},
        {std::bitset<6>(0xa), "I-Type:slti"},
        {std::bitset<6>(0xb), "I-Type:sltiu"},
        {std::bitset<6>(0xc), "I-Type:andi"},
        {std::bitset<6>(0xd), "I-Type:ori"},
        {std::bitset<6>(0xe), "I-Type:xori"},
        {std::bitset<6>(0xf), "I-Type:lui"},
        {std::bitset<6>(0x23), "I-Type:lw"},
        {std::bitset<6>(0x2b), "I-Type:sw"},
        {bitset<6>(0x1), "I-Type:bltz"},
        //Note following instructions go to different sizes, but I treat them
        //as load word so you can run straight assmebly from gcc Xcompiler
        {bitset<6>(0x20), "I-Type:lw"},
        {bitset<6>(0x21), "I-Type:lw"},
        {bitset<6>(0x24), "I-Type:lw"},
        {bitset<6>(0x25), "I-Type:lw"},
        {bitset<6>(0x28), "I-Type:sw"},
        {bitset<6>(0x29), "I-Type:sw"}
      }
   );

//Operators
  bool operator==(const Instruction& left, const Instruction& right){
    return left.getInstr() == right.getInstr();
  }

}

std::ostream& operator<<(std::ostream& out, instruction::Instruction instr){
  return out << instr.getInstr();
}
