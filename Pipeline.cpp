#define BOOST_LOG_DYN_LINK
#include <string>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <exception>
#include "Pipeline.h"
#include "Mem.h"

using namespace std;
using namespace instruction;
using namespace mem;

namespace pipeline{

  //Out classes 
  PCOut::PCOut(unsigned int addr) : addr{addr}{}
  PCOut::PCOut() : addr{0}{}

  IFOut::IFOut(const instruction::Instruction& instr) : 
    instr{instruction::Instruction(instr)} {};
  IFOut::IFOut() : instr{instruction::Instruction(0)}{};

  IDOut::IDOut(instruction::Instruction instr, std::vector<mem::data32> regVals)
    : instr{instr}, regVals{regVals}{};
  IDOut::IDOut(): instr{instruction::Instruction(0)}, regVals(0){};

  EXOut::EXOut(instruction::Instruction instr, std::vector<mem::data32> regVals,
      mem::data64 comp) : instr{instr}, regVals{regVals}, comp{comp}{};
  EXOut::EXOut() : instr{instruction::Instruction(0)}, regVals(0), comp{0}{};

  MAOut::MAOut(instruction::Instruction instr, std::vector<mem::data32> regVals,
    mem::data64 comp, mem::data32 loaded) : instr{instr}, regVals{regVals}, 
    comp{comp}, loaded{loaded}{};
  MAOut::MAOut() : instr{instruction::Instruction(0)}, regVals(0), comp{0},
    loaded{0}{};

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

  bool PipelinePhase::canUpdateArgs(){
    if(!isBusy()){
      return true;
    } else { 
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> " <<
        "tried to execute, but Pipeline stage was busy. ";
      return false;
    }
  }

  void PipelinePhase::updateCycle(int cycleChange){
    // this function includes nice logging and error checking
    setCyclesRemaining(cyclesRemaining - cycleChange);
  }


  //TODO If I change the brackets to () I don't get compiler error, I get
  //linker obscure error?
  InstructionFetch::InstructionFetch(std::string name, mem::MemoryUnit& mem):
    PipelinePhase(name),
    mem{ mem }{
    cyclesRemaining = 0;
    args = NULL;
  }

  void InstructionFetch::execute(StageOut** args){
    assert(canUpdateArgs());
    //Delete the old arguments
    delete this->args;
    //save the pointer to the arguments
    this->args = (PCOut*) (*args);
    //Nullify the old pointer so user can't use it anymore
    *args = NULL;
    setCyclesRemaining(1);
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with address " << this->args->addr << std::endl;
  }

  StageOut* InstructionFetch::getOut(){
    try{
      assert(!isBusy());
    } catch (std::exception& e){
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> " <<
        "tried to getOut, but Pipeline stage was busy with " << 
        cyclesRemaining << ". The instruction you're trying to get output for"
        " may not be finished executing. " << e.what();
      throw e;
    }
    unsigned int addr = args->addr;
    mem::data32 instrInt = mem.ld(addr);
    instruction::Instruction instr = instruction::Instruction(instrInt);
    IFOut* out = new IFOut(instr);
    return out;
  }

  InstructionDecode::InstructionDecode(std::string name, mem::MemoryUnit& rf) :
    PipelinePhase(name),
    rf{rf}
  {
    cyclesRemaining = 1;
    args=NULL;
  }

  mem::data32 InstructionDecode::loadReg(const std::bitset<5>& addr) const{
    unsigned long addr_int = addr.to_ulong();
    return rf.ld(addr_int);
  }

  void InstructionDecode::execute(StageOut** args){
    assert(canUpdateArgs());
    //Delete the old arguments
    delete this->args;
    this->cyclesRemaining = 1;
    //save the pointer to the Out
    this->args = (IFOut*) *args;
    //NULLIFY the users pointer
    *args = NULL;
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with instruction " << this->args->instr.toString() << std::endl;
  }

  StageOut* InstructionDecode::getOut(){
    try{
      assert(!isBusy());
    } catch (std::exception& e){
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> " <<
        "tried to getOut, but Pipeline stage was busy with " << 
        cyclesRemaining << ". The instruction you're trying to get output for"
        " may not be finished executing. " << e.what();
      throw e;
    }
    std::vector<mem::data32> regVals;
    if(args->instr.getType().find("R-Type") != std::string::npos){
      //Is R-Type
      regVals = std::vector<mem::data32>(3);
      std::bitset<5> rs = args->instr.getSlice<21,26>();
      std::bitset<5> rt = args->instr.getSlice<16,21>();
      std::bitset<5> rd = args->instr.getSlice<11,16>();
      regVals[0] = loadReg(rs);
      regVals[1] = loadReg(rt);
      regVals[2] = loadReg(rd);
    } else if(args->instr.getType().find("I-Type") != std::string::npos){
      //Is I-Type
      regVals = std::vector<mem::data32>(2);
      std::bitset<5> rs = args->instr.getSlice<21,26>();
      //depending on the type, this next register may be rt or Rd. It doesn't
      //matter at this stage
      std::bitset<5> rtOrRd = args->instr.getSlice<16,21>();
      regVals[0] = loadReg(rs);
      regVals[1] = loadReg(rtOrRd);
    } else {
      //Is J-Type
      regVals = std::vector<mem::data32>(0);
    }
    IDOut* out = new IDOut(args->instr, regVals);
    return out;
  }

  Execute::Execute(std::string name) : PipelinePhase(name){
    cyclesRemaining = 1;
    args = NULL;
  }

  void Execute::execute(StageOut** args){
    assert(canUpdateArgs());
    //Delete the old arguments
    std::cout <<"printing instr " <<  this->args << std::endl;
    std::cout << "never got here" << std::endl;
    delete this->args;
    //save the ptr to the struct
    this->args = (IDOut*) (*args);
    //Nullify the ptr for user cause they should never use again
    *args = NULL;
    setCyclesRemaining(1);
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with instruction " << this->args->instr.toString() << std::endl;
  }

  StageOut* Execute::getOut(){
    mem::data64 comp;
    std::string instrType = args->instr.getType();
    if(instrType == "R-Type"){
      //Is R-Type
      //initialize some helpful vars for the func type
      std::string func = args->instr.getFuncType();
      mem::data32 rs = args->regVals[0];
      mem::data32 rt = args->regVals[1];
      mem::data32 rd = args->regVals[2];
      std::bitset<5> shamtBits = args->instr.getSlice<6,11>();
      mem::data32 shamt = shamtBits.to_ulong();
      if(func == "subu"){
        comp = (mem::data32) (rt-rs);
      } else if(func == "sub"){
        //no trapping so does same thing
        comp = (mem::data32) (rt-rs);
      } else if(func == "addu"){
        comp = (mem::data32) (rt+rs);
      } else if(func == "add"){
        //no trapping so does same thing
        comp = (mem::data32) (rt+rs);
      } else if(func == "sll"){ //shifts
        comp = (mem::data32) (rs << shamt);
      } else if(func == "sllv"){ 
        comp = (mem::data32) (rs << (rt & 0b11111)); //The & gets lower order 5 bits
      } else if(func == "srl"){ 
        comp = (mem::data32) (rs >> shamt);
      } else if(func == "srlv"){ 
        comp = (mem::data32) (rs >> (rt & 0b11111));
      } else if(func == "jr"){ 
        comp = (mem::data32) (rs);
      } else if(func == "and"){ //Logical
        comp = (mem::data32) (rs & rt);
      } else if(func == "or"){ 
        comp = (mem::data32) (rs | rt);
      } else if(func == "xor"){ 
        comp = (mem::data32) (rs ^ rt);
      } else if(func == "nor"){ 
        comp = (mem::data32) (~(rs | rt));
      } else if(func == "slt"){ //comparison
        comp = ((mem::signedData32) rs < (mem::signedData32) rt) ? 1 : 0;
      } else if(func == "sltu"){ 
        comp = (rs < rt) ? 1 : 0;
      } else if(func == "mult"){ 
        //Yes, this casting is pretty wild, let me speak it to you.
        //ya take in two unsigned 32 bit values, but you want to treat
        //them as signed (first cast). Then ya gotta make sure if you overflow
        //that is captured as long, (second cast)
        comp = (mem::signedData64)(mem::signedData32)rs * 
          (mem::signedData64)(mem::signedData32)rt;
      } else if(func == "multu"){ 
        comp = (mem::data64)rs * (mem::data64)rt;
      } else if(func == "div"){ 
        mem::data64 quotient = (mem::signedData32)rs / (mem::signedData32)rt;
        mem::data64 rem = (mem::signedData32)rs % (mem::signedData32)rt;
        comp = (rem << 32) + quotient;
      } else if(func == "divu"){ 
        mem::data64 quotient = rs / rt;
        mem::data64 rem = rs % rt;
        comp = (rem << 32) + quotient;
      }
      else {
        //Couldn't find the function specified, so unimplemented, so dying
        BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> encountered"
          " unimplemented function type for R-Type instruction, " + func << "." 
          << std::endl;
        //TODO come up with a nicer exception
        throw std::exception();
      }
    }
    //Now we're in I instr land
    else if (instrType.find("I-Type") != std::string::npos){
      mem::data32 rs = args->regVals[0];
      mem::data32 rt = args->regVals[1];
      mem::data32 immediate = args->instr.getSlice<0,16>().to_ulong();
      if (instrType == "I-Type:beq"){
        comp = rs==rt;
      } else if (instrType == "I-Type:bne"){
        comp = rs != rt;
      } else if (instrType == "I-Type:addi"){
        comp = (mem::signedData32) rs + (short)immediate;
      } else if (instrType == "I-Type:addiu"){
        comp = (mem::signedData32) rs + (short)immediate;
      } else if (instrType == "I-Type:slti"){
        comp =  (mem::signedData32) rs < (short) immediate;
      } else if (instrType == "I-Type:sltiu"){
        comp = rs < (unsigned short)immediate;
      } else if (instrType == "I-Type:andi"){
        comp = (unsigned short) rs & (unsigned short) immediate;
      } else if (instrType == "I-Type:ori"){
        comp = (unsigned short) rs | (unsigned short) immediate;
      } else if (instrType == "I-Type:xori"){
        comp = (unsigned short) rs ^ (unsigned short) immediate;
      } else if (instrType == "I-Type:lui"){
        comp = ((unsigned short) immediate) << 16;
        //lb to lwu do same thing just an add
      } else if (instrType == "I-Type:lb"){
        comp = (mem::signedData32) rs + (short)immediate;
      } else if (instrType == "I-Type:lh"){
        comp = (mem::signedData32) rs + (short)immediate;
      } else if (instrType == "I-Type:lw"){
        comp = (mem::signedData32) rs + (short)immediate;
      } else if (instrType == "I-Type:lbu"){
        comp = (mem::signedData32) rs + (short)immediate;
      } else if (instrType == "I-Type:lhu"){
        comp = (mem::signedData32) rs + (short)immediate;
      } else if (instrType == "I-Type:lwu"){
        comp = (mem::signedData32) rs + (short)immediate;
      } else if (instrType == "I-Type:sb"){
        comp = (mem::signedData32) rt + (short)immediate;
      } else if (instrType == "I-Type:sh"){
        comp = (mem::signedData32) rt + (short)immediate;
      } else if (instrType == "I-Type:sw"){
        comp = (mem::signedData32) rt + (short)immediate;
      }
    }
    else if (instrType.find("J-Type") != std::string::npos){
      comp = 0;
    }
    else {
      //Whatever the type of this instruction, it hasn't been implemented
      BOOST_LOG_TRIVIAL(fatal) << "<<" + getName() + ">> encountered" 
        " unimplemented instruction type, " + instrType + "." << std::endl;
      throw std::exception();
    }
    //You've done the heavy lifting at this point. Now you just assemble the
    //struct
    EXOut* exOut = new EXOut(args->instr, args->regVals, comp);
    return exOut;
  }

  MemoryAccess::MemoryAccess(std::string name, mem::MemoryUnit* mem) : 
    PipelinePhase(name), mem{mem}{
      cyclesRemaining = 1;
      args = NULL;
    };

  void MemoryAccess::execute(StageOut** args){
    assert(canUpdateArgs());
    //Delete the old arguments
    delete this->args;
    //save the ptr to the struct
    this->args = (EXOut*) *args;
    //Nullify the ptr for user cause they should never use again
    *args = NULL;
    setCyclesRemaining(1);
    BOOST_LOG_TRIVIAL(debug) << "<<" + getName() + ">> " << "executing args"
      " with instruction " << this->args->instr.toString() << std::endl;
  }

  StageOut* MemoryAccess::getOut(){
    mem::data32 loaded = 0;
    std::string instrType = args->instr.getType();
    if (instrType == "I-Type:sw"){
      mem::data32 rs = args->regVals[0];
      mem->sw(args->comp, rs);
    } else if (instrType == "I-Type:lw"){
      loaded = mem->ld((mem::data32) args->comp);
    }
    MAOut* out = new MAOut(args->instr, args->regVals, args->comp, loaded);
    return out;
  }

  WriteBack::WriteBack(string name) : PipelinePhase(name){}

  void WriteBack::execute(StageOut** args){}

  StageOut* WriteBack::getOut(){return NULL;}

}
