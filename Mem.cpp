#define BOOST_LOG_DYN_LINK
#include <boost/log/trivial.hpp>
#include <exception>

#include "Mem.h"

namespace mem{

  /*BEGIN DRAM IMPLEMENTATION*/

  const std::string& MemoryUnit::getName(){
    return name;
  }

  MemoryUnit::MemoryUnit(std::string name): name(name){}

  /*
   * throws: exception if address is invalid
   */
  void DRAM::isValidAddr(int addr){
    if(addr > size){
      BOOST_LOG_TRIVIAL(fatal) << "<<" << getName() << 
        ">> tried to access invalid memory "
        "address was " << addr << " but size of " << getName() << " is " <<
        size << std::endl;
      throw std::exception();
    }
  }
  /*
   * TODO If I make const, everything goes downhill
   * Initialize a memory unit with a dynamically allocated memory array
   * params:
   *   size: the size of the memory in words
   */
  DRAM::DRAM(size_t size, std::string name): MemoryUnit(name){
    this->size = size;
    mem = new data32[size];
  }

  /*
   * Initizlize memory unit with a given array. Will copy array into
   * new allocated array of size given
   * params:
   *   size: the size of the memory in words
   *   mem: a pointer to an array of size, size 
   *
   */
  DRAM::DRAM(size_t size, data32* mem, std::string name): MemoryUnit(name){
    this->size = size;
    this->mem = new data32[size];
    //TODO this is not legal? DRAM(size); overshaddows parameter?
    std::copy(mem, mem + size, this->mem); //copy
  }
  
  /*
   * loads the requested word
   * params:
   *   addr: the address to load
   * returns: the word at that address
   * throws: exception if address is invalid
   */
  data32 DRAM::ld(unsigned int addr){
    isValidAddr(addr);
    BOOST_LOG_TRIVIAL(debug) << "<<" << getName() << ">>" << " loading " << 
      addr << "." << std::endl;
    return mem[addr];
  }

  /*
   * stores word at loaction specified by address
   * params:
   *   addr: the address
   *   word: the word to store
   * throws: exception if address is invalid
   */
  void DRAM::sw(unsigned int addr, data32 word){
    isValidAddr(addr);
    BOOST_LOG_TRIVIAL(debug) << "<<" << getName() << ">>" << "storing " << 
      word << " @" << addr << "." << std::endl;
    mem[addr] = word;
  }

  size_t DRAM::getSize(){
    return size;
  }

  /*
   * destructor will deallocate mem array
   */
  DRAM::~DRAM(){
    delete [] mem;
  }
}
