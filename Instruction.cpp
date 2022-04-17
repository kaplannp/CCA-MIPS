#include "Instruction.h"
#include<iostream>

namespace instruction{
 

  InstrBits Instruction::getInstr() const{
    return instr;
  }

  Instruction::Instruction(unsigned int instr): instr(instr){}


  //TODO I got nothing dynamic, therefore copying is ok?
  //TODO I don't know the size this needs to return! C++ template magic?
  std::bitset<6>* Instruction::getSlice(int start, int end) const{
    std::bitset<6>* slice = new std::bitset<6>(0);
    for(int i = start; i < end; i++){
      slice->set(i-start, instr[i]);
    }
    return slice;
  }

  std::string Instruction::getType() const{
    //TODO destroy here?
    const std::bitset<6>* opcode = getSlice(26,32);
    return Instruction::OPCODE2STR.at(*opcode);
  }
  
  std::string Instruction::getFuncType() const{
    std::bitset<6>* funcBits = getSlice(0,6);
    std::string func = FUNC_2_MNEMONIC.at((unsigned int) funcBits->to_ulong());
    return func;
  }

  std::string Instruction::toString() const{
    return getType() + " " + instr.to_string();
  }

  //std::unordered_map<std::bitset<6>, std::string>* Instruction::initOpcode2Str()
  //{
  //  std::unordered_map<std::bitset<6>, std::string>* map = new 
  //    std::unordered_map<std::bitset<6>, std::string>();
  //  map->insert({std::bitset<6>(0), "R-Type"});
  //  map->insert({std::bitset<6>(0x2), "J-Type:j"});
  //  map->insert({std::bitset<6>(0x3), "J-Type:jal"});
  //  map->insert({std::bitset<6>(0x4), "I-Type:eq"});
  //  map->insert({std::bitset<6>(0x5), "I-Type:ne"});
  //  map->insert({std::bitset<6>(0x8), "I-Type:ddi"});
  //  map->insert({std::bitset<6>(0x9), "I-Type:ddiu"});
  //  map->insert({std::bitset<6>(0xa), "I-Type:lti"});
  //  map->insert({std::bitset<6>(0xb), "I-Type:ltiu"});
  //  map->insert({std::bitset<6>(0xc), "I-Type:ndi"});
  //  map->insert({std::bitset<6>(0xd), "I-Type:ri"});
  //  map->insert({std::bitset<6>(0xe), "I-Type:ori"});
  //  map->insert({std::bitset<6>(0xf), "I-Type:ui"});
  //  return map;
  //}
  
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
        {std::bitset<6>(0xf), "I-Type:lui"}
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
