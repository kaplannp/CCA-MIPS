#define BOOST_LOG_DYN_LINK
#include "Pipeline.h"
#include <string>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <exception>

namespace pipeline{

  bool PipelinePhase::checkInvariants() const{
    return checkCyclesRemaining();
  }

  bool PipelinePhase::checkCyclesRemaining() const{
    bool invalidCycles = cyclesRemaining <= 0;
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
    instr = "Bubble";
    cyclesRemaining = 1;
  }

  /* note that this function temporarilly modifies the invariants. This is
   * a point in the code (the only point?) in which the cyclesRemaining drops to
   * 0. At all other points, the code should have a cycleRemainign of at least
   * 1. */
  void PipelinePhase::updateCycle(int cycleChange){
    if(instr != "Bubble"){
      //only update cycles if you actually are executing an instruction
      cyclesRemaining -= cycleChange;
      //Log event
      BOOST_LOG_TRIVIAL(debug) << "<<" << getName() << ">>" << 
        "cycle updated. " << cyclesRemaining << " cycles remaining";
    }
  }

  /* internally this method is very simple. It saves the current instruction,
   * and then calls updateInstr to update the instruction to passed. */
  std::string PipelinePhase::processInstr(std::string instr){
    std::string oldInstr = this->instr;
    updateInstr(instr);
    //It is required that cycles Remaining should never fall below 0
    assert(checkInvariants());
    return oldInstr;
  }

  //TODO If I change the brackets to () I don't get compiler error, I get
  //linker obscure error?
  InstructionFetch::InstructionFetch(std::string name): PipelinePhase{ name } {
    cyclesRemaining = 0;
  }

  void InstructionFetch::updateInstr(std::string instr){
    try{
      assert(!isBusy());
    } catch (std::exception& e){
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> " <<
        "tried to processInstr, but Pipeline stage was busy" << e.what();
      throw e;
    }
    this->instr = instr;
    this->cyclesRemaining = 1;
  }

}
