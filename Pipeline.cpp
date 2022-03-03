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
      BOOST_LOG_TRIVIAL(fatal) << "<<" << getName() << ">>" << 
        "cyclesRemaining has fallen below 0."
        "You may have missed an event.";
    }
    return invalidCycles;
  }

  std::string PipelinePhase::getName() const{
    return name;
  }

  bool PipelinePhase::isBusy() const{
    return cyclesRemaining > 0;
  }

  PipelinePhase::PipelinePhase(std::string name) : name(name){};

  void PipelinePhase::updateCycle(int cycleChange){
    cyclesRemaining -= cycleChange;
    BOOST_LOG_TRIVIAL(debug) << "<<" << getName() << ">>" << 
      "cycle updated. " << cyclesRemaining << "cycles remaining";
    //It is required that cycles Remaining should never fall below 0
    assert(checkInvariants());
  }

  InstructionFetch::InstructionFetch(std::string name): PipelinePhase(name){
    instr = "Bubble";
    cyclesRemaining = 0;
  }

  void InstructionFetch::processInstr(std::string instr){
    try{
      assert(isBusy());
    } catch (std::exception& e){
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">>" <<
        "tried to processInstr, but Pipeline stage was busy" << e.what();

      throw e;
    }
    this->instr = instr;
    this->cyclesRemaining = 1;
  }

}
