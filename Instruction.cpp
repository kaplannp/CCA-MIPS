#include "Instruction.h"
#include<iostream>

namespace instruction{
 
  std::unordered_map<std::bitset<6>, std::string>* Instruction::initOpcode2Str()
  {
    std::unordered_map<std::bitset<6>, std::string>* map = new 
      std::unordered_map<std::bitset<6>, std::string>();
    map->insert({std::bitset<6>(0), "R-Type"});
    map->insert({std::bitset<6>(0x2), "J-Type:j"});
    map->insert({std::bitset<6>(0x3), "J-Type:jal"});
    map->insert({std::bitset<6>(0x4), "I-Type:eq"});
    map->insert({std::bitset<6>(0x5), "I-Type:ne"});
    map->insert({std::bitset<6>(0x8), "I-Type:ddi"});
    map->insert({std::bitset<6>(0x9), "I-Type:ddiu"});
    map->insert({std::bitset<6>(0xa), "I-Type:lti"});
    map->insert({std::bitset<6>(0xb), "I-Type:ltiu"});
    map->insert({std::bitset<6>(0xc), "I-Type:ndi"});
    map->insert({std::bitset<6>(0xd), "I-Type:ri"});
    map->insert({std::bitset<6>(0xe), "I-Type:ori"});
    map->insert({std::bitset<6>(0xf), "I-Type:ui"});
    return map;
  }

  InstrBits Instruction::getInstr() const{
    return instr;
  }

  Instruction::Instruction(unsigned int instr): instr(instr){
    //TODO this has really got to go
    OPCODE2STR = initOpcode2Str();
  }


  //TODO I got nothing dynamic, therefore copying is ok?

  std::bitset<6>* Instruction::getSlice(int start, int end) const{
    std::bitset<6>* slice = new std::bitset<6>();
    for(int i = start; i < end; i++){
      slice->set(i-start, instr[i]);
    }
    return slice;
  }

  std::string Instruction::getType() const{
    //TODO destroy here?
    const std::bitset<6>* opcode = getSlice(26,32);
    return (*OPCODE2STR)[*opcode];
  }

  std::string Instruction::toString(){
    return getType() + " " + instr.to_string();
  }

//Operators
  bool operator==(const Instruction& left, const Instruction& right){
    return left.getInstr() == right.getInstr();
  }

}

std::ostream& operator<<(std::ostream& out, instruction::Instruction instr){
  return out << instr.getInstr();
}
