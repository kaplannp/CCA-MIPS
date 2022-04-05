#include "Pipeline.h"
#include "Mem.h"
#include "Instruction.h"
#include "Debug.h"

#define BOOST_TEST_MODULE Pipeline Tests
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_CATCH_SYSTEM_ERRORS yes

#include <boost/test/unit_test.hpp>
#include <exception>
#include <string>

// this runs it 'g++ test.cpp  -lboost_unit_test_framework'

BOOST_AUTO_TEST_SUITE( TestInstruction )

  BOOST_AUTO_TEST_CASE( TestRInstruction ){
    #define SIZE 13
    std::string types[SIZE] = {"R-Type", "J-Type:j", "J-Type:jal", "I-Type:eq", 
      "I-Type:ne", 
      "I-Type:ddi", "I-Type:ddiu", "I-Type:lti", "I-Type:ltiu", "I-Type:ndi", 
      "I-Type:ri", "I-Type:ori", "I-Type:ui"
    };
    int codes[SIZE] = {
      0x0, 0x2, 0x3, 0x4, 0x5, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
    };
    for(int i = 0; i < SIZE; i++){
      instruction::Instruction instr = instruction::Instruction(
          codes[i] << 26
          );
      BOOST_CHECK(instr.toString().find(types[i]) != std::string::npos);
    }
    #undef SIZE
  }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( TestMemory )

  BOOST_AUTO_TEST_CASE( TestDRAM ){
    size_t s1 = 1000;
    mem::MemoryUnit* m1 = new mem::DRAM(s1, "m1");

    //test some simple store
    for(int i = 0; i < 10; i++)
      m1->sw(i, i);
    for(int i = 0; i < 10; i++)
      BOOST_CHECK_EQUAL(m1->ld(i), i);

    //test random stores
    m1->sw(459, 8);
    BOOST_CHECK_EQUAL(m1->ld(459), 8);
    m1->sw(999, 23); // edge of mem
    BOOST_CHECK_EQUAL(m1->ld(999), 23);

    mem::data32 mo2[] = {4,5,2,4,50,29};
    size_t s2 = 6;
    mem::MemoryUnit* m2 = new mem::DRAM(s2, mo2, "m2");
    for(int i = 0; i < s2; i++)
      BOOST_CHECK_EQUAL(m2->ld(i), mo2[i]);
  }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( TestPipelinePhases )

  BOOST_AUTO_TEST_CASE( TestIF )
  {
    //TODO would love to have m1 be a reference instead of a pointer
    mem::MemoryUnit* m1 = new mem::DRAM(100, "m1");
    //store a couple words in the DRAM
    m1->sw(0,5);
    m1->sw(5,10);
    pipeline::PipelinePhase* p = new pipeline::InstructionFetch("IF", *m1);
    BOOST_CHECK_EQUAL(p->getName(), "IF");
    pipeline::PCOut args = {0};
    p->execute(&args);
    BOOST_CHECK(p->isBusy());
    p->updateCycle(1);
    BOOST_CHECK( !(p->isBusy()) );
    // TODO catching exceptions with boost
    //BOOST_CHECK_THROW(p->updateCycle(1),std::exception);
    
    //Now, see if you can get the instructions right
    // expected instructions to be fetched
    //TODO how's that for memory mamnagement?
    instruction::Instruction instr1 = instruction::Instruction(5);
    instruction::Instruction instr2 = instruction::Instruction(10);
    pipeline::IFOut o1 = {instr1};
    pipeline::IFOut o2 = {instr2};
    pipeline::IFOut trueOut = *( (pipeline::IFOut*) (p->getOut()));
    //TODO in a better world, I would not specify getInstr because c++ would
    //call the operator I carefully designed. Alas
    BOOST_CHECK_EQUAL(trueOut.instr.getInstr(), o1.instr.getInstr());
    pipeline::PCOut op2 = {5}; //addr 5
    p->execute(&op2);
    p->updateCycle(1);
    trueOut = *( (pipeline::IFOut*) (p->getOut()));
    BOOST_CHECK_EQUAL(trueOut.instr.getInstr(), o2.instr.getInstr());
  }

  BOOST_AUTO_TEST_CASE( TestID ){
    mem::MemoryUnit* rf1 = new mem::DRAM(100, "rf1");
    pipeline::PipelinePhase* id = new pipeline::InstructionDecode("ID",*rf1);
    id->updateCycle(1); //burn the bubble

    //Check an R-Type Instruction
    int rs = 1;
    int rt = 2;
    int rd = 4;
    int shamt = 0;
    int func = 0;
    int regVals[3] = {1,2,4};
    //write some registers
    rf1->sw(rs, regVals[0]);
    rf1->sw(rt, regVals[1]);
    rf1->sw(rd, regVals[2]);
    //create instruction and execute it
    unsigned int instructionVal = constructRInstr(rs, rt, rd, shamt, func);
    instruction::Instruction instr = instruction::Instruction(instructionVal);
    BOOST_CHECK(instr.toString().find("R-Type") != std::string::npos);
    pipeline::IFOut ifOut = pipeline::IFOut(instr);
    std::cout << "is busy is " << id->isBusy() << std::endl;
    id->execute((void*)&ifOut);
    id->updateCycle(1); //pass one timestep
    //check for correct output
    pipeline::IDOut* idOut = (pipeline::IDOut*) id->getOut();
    for(int i = 0; i < 3; i++){
      BOOST_CHECK_EQUAL(idOut->regVals[i], regVals[i]);
    }
    //Check an I-Type Instruction
    int opcode2 = 4;
    int rs2 = 15;
    int rtOrRD = 30;
    int instrVal = 0;
    int regVals2[2] = {7,20};
    //write some registers
    rf1->sw(rs2, regVals2[0]);
    rf1->sw(rtOrRD, regVals2[1]);
    //create instruction and execute it
    unsigned int instructionVal2 = constructIInstr(opcode2, rs2, rtOrRD, 
        instrVal);
    instruction::Instruction instr2 = instruction::Instruction(instructionVal2);
    BOOST_CHECK(instr2.toString().find("I-Type") != std::string::npos);
    pipeline::IFOut ifOut2 = pipeline::IFOut(instr2);
    id->execute((void*)&ifOut2);
    id->updateCycle(1); //pass one timestep
    //check for correct output
    pipeline::IDOut* idOut2 = (pipeline::IDOut*) id->getOut();
    for(int i = 0; i < idOut2->regVals.size(); i++){
      BOOST_CHECK_EQUAL(idOut2->regVals[i], regVals2[i]);
    }
    //Check a J-Type Instruction
    int opcode3 = 2;
    int instrVal3 = 32;
    unsigned int instructionVal3 = constructJInstr(opcode3, instrVal3);
    instruction::Instruction instr3 = instruction::Instruction(instructionVal3);
    BOOST_CHECK(instr3.getType().find("J-Type") != std::string::npos);
    pipeline::IFOut ifOut3 = pipeline::IFOut(instr3);
    id->execute((void*)&ifOut3);
    id->updateCycle(1); //pass one timestep
    //check for correct output
    pipeline::IDOut* idOut3 = (pipeline::IDOut*) id->getOut();
    BOOST_CHECK(idOut3->regVals.empty());
  }

  BOOST_AUTO_TEST_CASE( TestExecute ){
    pipeline::PipelinePhase* ex = new pipeline::Execute("EX");
    ex->updateCycle(1); // Burn the bubble 
    typedef struct runArgs {
      mem::data32 rs;
      mem::data32 rt;
      mem::data32 rd;
      unsigned int shamt;
      unsigned int func;
      mem::data32 expected;
      runArgs( mem::data32 rs, mem::data32 rt, mem::data32 rd,
          unsigned int shamt, unsigned int func, mem::data32 expected) : 
          rs{rs}, rt{rt}, rd{rd}, shamt{shamt}, func{func}, expected{expected}{}
    } runArgs;
    std::vector<runArgs> runs = {
      runArgs(2,4,0,0,0x21,6), //addu 2+4 = 6
      runArgs(2,4,0,0,0x23,2), //subu 4-2 = 2
      runArgs(1,0,0,0,0x23,-1), //subu 0-1 = -1 *note -1 = 4294967295
      runArgs(-1,1,0,0,0x21,0), //addu -1+1 = 0 *note see above
    };
    for(runArgs args : runs){
      //Note that rs, rt, rd addrs don't matter, only their values, so that
      //is what is stored here
      mem::data32 rs = args.rs; //val to invert
      mem::data32 rt = args.rt; //val not inverted
      mem::data32 rd = args.rd; //destination
      unsigned int shamt = args.shamt;
      unsigned int func = args.func;
      unsigned int instructionVal = constructRInstr(0, 0, 0, shamt, func);
      instruction::Instruction instr = instruction::Instruction(instructionVal);
      std::vector<mem::data32> regVals = {rs, rt, rd};
      pipeline::IDOut idOut = pipeline::IDOut(instr, regVals);
      ex->execute(&idOut);
      ex->updateCycle(1);
      pipeline::EXOut* exOut = (pipeline::EXOut*) ex->getOut();
      BOOST_CHECK_EQUAL(exOut->comp, args.expected);
    }
  }

BOOST_AUTO_TEST_SUITE_END()
