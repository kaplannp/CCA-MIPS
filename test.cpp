#include "Pipeline.h"
#include "Mem.h"
#include "Instruction.h"

#define BOOST_TEST_MODULE Pipeline Tests
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_CATCH_SYSTEM_ERRORS yes

#include <boost/test/unit_test.hpp>
#include <exception>

// this runs it 'g++ test.cpp  -lboost_unit_test_framework'

BOOST_AUTO_TEST_SUITE( TestInstruction )

  BOOST_AUTO_TEST_CASE( TestRInstruction ){
    instr::Instruction instr = instr::Instruction(0);
    //BOOST_CHECK_EQUAL(instr.toString(), "R-instruction");
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

    int mo2[] = {4,5,2,4,50,29};
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
    pipeline::IFOut o1 = {5};
    pipeline::IFOut o2 = {10};
    pipeline::IFOut trueOut = *( (pipeline::IFOut*) (p->getOut()));
    BOOST_CHECK_EQUAL(trueOut.instr, o1.instr);
    pipeline::PCOut op2 = {5}; //addr 5
    p->execute(&op2);
    p->updateCycle(1);
    trueOut = *( (pipeline::IFOut*) (p->getOut()));
    BOOST_CHECK_EQUAL(trueOut.instr, o2.instr);
  }

  BOOST_AUTO_TEST_CASE( TestID ){
   
  }

BOOST_AUTO_TEST_SUITE_END()
