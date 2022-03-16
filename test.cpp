#include "Pipeline.h"
#include "Mem.h"
#include "Instruction.h"
#include "Debug.h"

#define BOOST_TEST_MODULE Pipeline Tests
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_CATCH_SYSTEM_ERRORS yes

#include <boost/test/unit_test.hpp>
#include <exception>

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
      BOOST_CHECK_EQUAL(instr.toString(), types[i]);
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
    int opcode = 0;
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
    BOOST_CHECK_EQUAL(instr.toString(), "R-Type");
    pipeline::IFOut ifOut = pipeline::IFOut(instr);
    id->execute((void*)&ifOut);
    id->updateCycle(1); //pass one timestep
    //check for correct output
    pipeline::IDOut* idOut = (pipeline::IDOut*) id->getOut();
    for(int i = 0; i < 3; i++){
      std::cout << i << std::endl;
      BOOST_CHECK_EQUAL(idOut->regVals[i], regVals[i]);
    }
  }

BOOST_AUTO_TEST_SUITE_END()
