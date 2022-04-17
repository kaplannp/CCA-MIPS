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

//TODO I can't use severity?
//https://www.boost.org/doc/libs/1_79_0/libs/log/doc/html/log/tutorial/sinks.html
//void initLog(){
//  boost::log::add_file_log("out.log");
//  
//  //filter the log
//  boost::log::core::get()->set_filter
//    (
//        boost::log::trivial::severity >= boost::log::trivial::info
//    );
//
//}
