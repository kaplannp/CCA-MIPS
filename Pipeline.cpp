#define BOOST_LOG_DYN_LINK
#include <string>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <exception>
#include "Pipeline.h"
#include "Mem.h"

namespace pipeline{

  bool PipelinePhase::checkInvariants() const{
    return checkCyclesRemaining();
  }

  void PipelinePhase::setCyclesRemaining(int cycles){
   cyclesRemaining = cycles;
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "setting cycle to "
      << cyclesRemaining << "." << std::endl;
    //It is required that cycles Remaining should never fall below 0
    assert(checkInvariants());
  }

  bool PipelinePhase::checkCyclesRemaining() const{
    bool invalidCycles = cyclesRemaining < 0;
    if(invalidCycles){
      BOOST_LOG_TRIVIAL(fatal) << "<<" << getName() << ">> " << 
        "cyclesRemaining has fallen below 0. "
        "You may have missed an event.";
    }
    return !invalidCycles;
  }

  std::string PipelinePhase::getName() const{
    return name;
  }

  bool PipelinePhase::isBusy() const{
    return cyclesRemaining > 0;
  }

  PipelinePhase::PipelinePhase(std::string name) : name(name){
    cyclesRemaining = 1;
  }

  void PipelinePhase::updateCycle(int cycleChange){
    // this function includes nice logging and error checking
    setCyclesRemaining(cyclesRemaining - cycleChange);
  }

  //TODO If I change the brackets to () I don't get compiler error, I get
  //linker obscure error?
  InstructionFetch::InstructionFetch(std::string name, mem::MemoryUnit& mem):
    PipelinePhase{ name },
    mem{ mem }{
    cyclesRemaining = 0;
  }

  void InstructionFetch::updateArgs(void* args){
    try{
      assert(!isBusy());
    } catch (std::exception& e){
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> " <<
        "tried to execute, but Pipeline stage was busy. " << e.what();
      throw e;
    }
    this->args = *( (PCOut*) args );
    setCyclesRemaining(1);
  }


  /* internally this method is very simple. It saves the current instruction,
   * and then calls updateInstr to update the instruction to passed. */
  void InstructionFetch::execute(void* args){
    updateArgs(args);
  }

  void* InstructionFetch::getOut(){
    unsigned int addr = args.addr;
    int instr = mem.ld(addr);
    IFOut* out = new IFOut();
    out->instr = instr;
    return out;
  }

}
