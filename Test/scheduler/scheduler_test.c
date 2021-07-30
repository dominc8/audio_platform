#include "scheduler.h"
#include "CUnit/CUnitCI.h"
#include "utils.h"

#include <stdio.h>

int32_t gVar = 0;

void setup(void)
{
    gVar = 0;
}

int32_t task1(void *arg)
{
    gVar++;
    return 0;
}

int32_t task1_looped(void *arg)
{
    gVar++;
    return scheduler_enqueue_task(&task1_looped, arg);
}

void scheduler_init_test(void)
{
    int32_t empty_scheduler_ret_val = -1;
    setup();
    scheduler_init();

    CU_ASSERT_EQUAL(scheduler_dequeue_task(), empty_scheduler_ret_val);
}

void scheduler_single_enqueue_dequeue_test(void)
{
    setup();
    scheduler_init();

    CU_ASSERT_EQUAL(scheduler_enqueue_task(&task1, NULL), 0);
    CU_ASSERT_EQUAL(scheduler_dequeue_task(), 0);

    CU_ASSERT_EQUAL(gVar, 1);
}

void scheduler_full_enqueue_test(void)
{
    setup();
    scheduler_init();

    for (int32_t i = 0; i < scheduler_get_queue_size(); ++i)
    {
        CU_ASSERT_EQUAL(scheduler_enqueue_task(&task1, NULL), 0);
    }

    CU_ASSERT_EQUAL(scheduler_enqueue_task(&task1, NULL), -1);
}

void scheduler_empty_dequeue_test(void)
{
    setup();
    scheduler_init();

    CU_ASSERT_EQUAL(scheduler_dequeue_task(), -1);
}

void scheduler_auto_enqueue_task(void)
{
    int32_t n_iterations = scheduler_get_queue_size() * 2;
    setup();
    scheduler_init();

    CU_ASSERT_EQUAL(scheduler_enqueue_task(&task1_looped, NULL), 0);
    for (int32_t i = 0; i < n_iterations; ++i)
    {
        CU_ASSERT_EQUAL(scheduler_dequeue_task(), 0);
    }

    CU_ASSERT_EQUAL(gVar, n_iterations);
}

#define TESTS                                               \
    CUNIT_CI_TEST(scheduler_init_test),                     \
    CUNIT_CI_TEST(scheduler_single_enqueue_dequeue_test),   \
    CUNIT_CI_TEST(scheduler_full_enqueue_test),             \
    CUNIT_CI_TEST(scheduler_empty_dequeue_test),            \
    CUNIT_CI_TEST(scheduler_auto_enqueue_task)

CUNIT_CI_RUN("scheduler_test", TESTS);

