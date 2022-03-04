#include "Pipeline.h"
#define BOOST_TEST_MODULE Pipeline Tests
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_CATCH_SYSTEM_ERRORS yes
#include <boost/test/unit_test.hpp>
// this runs it 'g++ test.cpp  -lboost_unit_test_framework'

/*
 * Just tests to make sure boose and header imports work as I expect
 */
BOOST_AUTO_TEST_SUITE( TestPipelinePhases )

BOOST_AUTO_TEST_CASE( TestSimplePhase )
{
  pipeline::PipelinePhase* p = new pipeline::InstructionFetch("IF");
  BOOST_CHECK_EQUAL(p->getName(), "IF");
  p->processInstr("Ld");
  BOOST_CHECK(p->isBusy());
  p->updateCycle(1);
  BOOST_CHECK( !(p->isBusy()) );
}

BOOST_AUTO_TEST_SUITE_END()
