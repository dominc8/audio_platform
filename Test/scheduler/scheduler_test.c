#include "scheduler.h"
#include "CUnit/CUnitCI.h"
#include "utils.h"

#include <stdio.h>


void scheduler_test(void)
{
    CU_ASSERT_TRUE(2>1);
}

#define TESTS           \
    CUNIT_CI_TEST(scheduler_test)

CUNIT_CI_RUN("scheduler_test", TESTS);

