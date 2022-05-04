#ifndef MEM_INCLUDED
#define MEM_INCLUDED
#include <unordered_map>
#include <boost/log/trivial.hpp>
#define BOOST_LOG_DYN_LINK

using namespace std;
/*
 * This module includes code of memory
 */
namespace mem{

  typedef unsigned char data8;
  typedef char signedData8;
  typedef unsigned short data16;
  typedef short signedData16;
  typedef unsigned int data32;
  typedef long data64; 
  typedef int signedData32; //not used for storage, but can be used for cast
  typedef long signedData64; //not used for storage, but can be used for cast
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
      MemoryUnit();

    public:
      /*
       * loads a word from address location specified by addr
       * params: 
       *   addr: the address
       * returns: the word
       */
      virtual data32 ld(unsigned int addr) = 0;

      /*
       * stores word at loaction specified by address
       * params:
       *   addr: the address
       *   word: the word to store
       */
      virtual void sw(unsigned int addr, data32 word) = 0;

      /*
       * used to load a block of words. Useful for initialization
       */
      virtual void storeBlock(data32 addr, data32* words, size_t size) = 0;

      const std::string& getName();

      virtual ~MemoryUnit() = default;

      virtual size_t getSize() = 0;

  };

  class DRAM: public MemoryUnit{

    private:
      data32* mem;
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
       DRAM(size_t size, data32* mem, std::string name);
    
      /*
       * loads the requested word
       * params:
       *   addr: the address to load
       * returns: the word at that address
       * throws: exception if address is invalid
       */
      data32 ld(unsigned int addr);

    /*
     * stores word at loaction specified by address
     * params:
     *   addr: the address
     *   word: the word to store
     * throws: exception if address is invalid
     */
    void sw(unsigned int addr, data32 word);
    
    /*
     * Used to load a block of words. Useful for initialization
     * @param addr: the address in the mem to start writing words
     * @param words: an array of words to write 
     * @param size: the number of words to write
     */
    void storeBlock(data32 addr, data32* words, size_t size);

    size_t getSize();

    /*
     * destructor will deallocate mem array
     */
    virtual ~DRAM();

    //TODO implement the shallow copy

  };

  /*
   * This class imitates some of the nice things about true virtual memory
   * by a hashtable on the frontend of address lookups. In this way, you
   * can have a much smaller physical memory than virtual memory, which is 
   * what is required for running a simulation without using copious amounts
   * of memory. This is NOT true virtual memory. This project does not implement
   * an os. There will be no OSing.
   *
   * The class wraps another MemoryUnit object
   *
   * If you ever try to use more memory than the given MemoryUnit object has,
   * this class will try to access out of bounds memory of that object, and the
   * composed MemoryUnit must deal with the consequences of that
   */
  class VirtualMem : public MemoryUnit{
    private:
      unordered_map<data32, data32> addrLookup;
      MemoryUnit* mem;
      int count;
      data32 lookup(data32);

    public:
      VirtualMem(MemoryUnit* mem);
      /*
       * loads a word from address location specified by addr
       * params: 
       *   addr: the address
       * returns: the word
       */
      data32 ld(unsigned int addr);

      /*
       * stores word at loaction specified by address
       * params:
       *   addr: the address
       *   word: the word to store
       */
      void sw(unsigned int addr, data32 word);

      /*
       * used to load a block of words. Useful for initialization
       */
      void storeBlock(data32 addr, data32* words, size_t size);

      const std::string& getName();

      ~VirtualMem();

      size_t getSize();
    
  };
}

#endif
