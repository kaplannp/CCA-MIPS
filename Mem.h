#ifndef MEM_INCLUDED
#define MEM_INCLUDED
#include <boost/log/trivial.hpp>
#define BOOST_LOG_DYN_LINK

/*
 * This module includes code of memory
 */
namespace mem{

  /*
   * Base interface for all memory objects
   * Abstract class
   * For simplicity, memory units will operate in terms of words of 4 byte
   */
  class MemoryUnit{
    private:
      std::string name;

    protected:
      MemoryUnit(std::string name);

    public:
      /*
       * loads a word from address location specified by addr
       * params: 
       *   addr: the address
       * returns: the word
       */
      virtual int ld(unsigned int addr) = 0;

      /*
       * stores word at loaction specified by address
       * params:
       *   addr: the address
       *   word: the word to store
       */
      virtual void sw(unsigned int addr, int word) = 0;

      std::string getName();

  };

  class DRAM: public MemoryUnit{

    private:
      int* mem;
      size_t size;
      /*
       * throws: exception if address is invalid
       */
      void isValidAddr(int addr);

    public:
      /*
       * Initialize a memory unit with a dynamically allocated memory array
       * params:
       *   size: the size of the memory in words
       *   name: a name to keep track of this object
       */
      DRAM(size_t size, std::string name);

      /*
       * Initizlize memory unit with a given array. Will copy array into
       * new allocated array of size given
       * params:
       *   size: the size of the memory in words
       *   mem: a pointer to an array of size, size 
       *   name: a name to keep track of this object
       */
      DRAM(size_t size, int* mem, std::string name);
    
      /*
       * loads the requested word
       * params:
       *   addr: the address to load
       * returns: the word at that address
       * throws: exception if address is invalid
       */
      int ld(unsigned int addr);

    /*
     * stores word at loaction specified by address
     * params:
     *   addr: the address
     *   word: the word to store
     * throws: exception if address is invalid
     */
    void sw(unsigned int addr, int word);

    size_t getSize();

    /*
     * destructor will deallocate mem array
     */
    ~DRAM();

    //TODO implement the shallow copy

  };

}

#endif
