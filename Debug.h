#include <iostream>
#include <bitset>
#define BOOST_LOG_DYN_LINK
#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/exceptions.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/trivial.hpp>

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
unsigned int constructIInstr(unsigned int opcode, unsigned char rs,
    unsigned char rtOrRD, unsigned short val){
  unsigned int instr = 0;
  instr += opcode << 26;
  instr += rs << 21;
  instr += rtOrRD << 16;
  instr += val;
  return instr;
}

/*
 * takes in the values of each subset of an J instruction, and returns the 
 * integer value of the instruction.
 * Really don't use a value outside permisible range of 28 bytes
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
//  //either missing header or dynamic invocation removes it
//  //boost::log::add_file_log("out.log");
//  
//  //filter the log
//  boost::log::core::get()->set_filter
//    (
//        boost::log::trivial::severity >= boost::log::trivial::info
//    );
//
//}
