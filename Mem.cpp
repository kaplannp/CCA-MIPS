#define BOOST_LOG_DYN_LINK
#include <boost/log/trivial.hpp>
#include <exception>
#include <algorithm>
#include <unordered_map>

#include "Mem.h"

using namespace std;

namespace mem{

  /*BEGIN DRAM IMPLEMENTATION*/

  const std::string& MemoryUnit::getName(){
    return name;
  }

  MemoryUnit::MemoryUnit(std::string name): name(name){}

  MemoryUnit::MemoryUnit(): name(""){}

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
    for(int i = 0; i < getSize(); i++)
      mem[i] = 0;
  }

  /*
   * Initizlize memory unit with a given array. Will copy array into
   * new allocated array of size given
   * params:
   *   size: the size of the memory in words
   *   mem: a pointer to an array of size, size 
   *
   */
  DRAM::DRAM(size_t size, data32* mem, std::string name):
  MemoryUnit{name}, mem{mem}
  {
    this->size = size;
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

  void DRAM::storeBlock(data32 addr, data32* words, size_t size){
    BOOST_LOG_TRIVIAL(debug) << "<<" << getName() << ">>" << " loading block of" 
      << size << " words starting at mem[" << addr << "]" << endl;
    copy(words, &words[size], &mem[addr]);
  }

  DRAM::~DRAM(){
    delete [] mem;
  }

  VirtualMem::VirtualMem(MemoryUnit* m1) : mem{m1}, MemoryUnit(){
    count = 0;
  }

  data32 VirtualMem::lookup(data32 addr){
    data32 physicalAddr = 0;
    if(!addrLookup.count(addr)){
      physicalAddr = count++;
      addrLookup[addr] = physicalAddr;
    } else {
      physicalAddr = addrLookup[addr];
    }
    return physicalAddr;
  }

  data32 VirtualMem::ld(unsigned int addr){
    return mem->ld(lookup(addr));
  }

  void VirtualMem::sw(unsigned int addr, data32 word){
    mem->sw(lookup(addr), word);
  }

  void VirtualMem::storeBlock(data32 addr, data32* words, size_t size){
    for(int i = 0; i < size; i++){
      sw(addr+i, words[i]);
    }
  }

  const string& VirtualMem::getName(){
    return mem->getName();
  }

  VirtualMem::~VirtualMem(){
    delete mem;
  }
  
  size_t VirtualMem::getSize(){
    return mem->getSize();
  }

}
