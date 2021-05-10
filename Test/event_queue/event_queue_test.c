#include "event_queue.h"
#include "CUnit/CUnitCI.h"

#include <stdio.h>

static void assert_events(event a, event b)
{
    CU_ASSERT_EQUAL(a.id, b.id);
    CU_ASSERT_EQUAL(a.val, b.val);
}

void eq_m7_add_event_test(void)
{
    int32_t add_status;
    event e = { .id = EVENT_M7_TRACE, .val = 10 };

    eq_m7_init();
    add_status = eq_m7_add_event(e);

    CU_ASSERT_EQUAL(add_status, 0);
}

void eq_m7_add_and_get_event_test(void)
{
    int32_t add_status, get_status;
    event e1 = { .id = EVENT_M7_TRACE, .val = 10 };
    event e2 = { .id = EVENT_N, .val = 20 };

    eq_m7_init();

    add_status = eq_m7_add_event(e1);
    get_status = eq_m7_get_event(&e2);

    CU_ASSERT_EQUAL(add_status, 0);
    CU_ASSERT_EQUAL(get_status, 0);
    assert_events(e1, e2);
}

void eq_m7_get_event_empty_eq_test(void)
{
    int32_t get_status;
    event e;

    eq_m7_init();
    get_status = eq_m7_get_event(&e);

    CU_ASSERT_EQUAL(get_status, -1);
}

void eq_m7_add_event_full_eq_test(void)
{
    int32_t add_status;
    event e = { .id = EVENT_M7_TRACE, .val = 10 };

    eq_m7_init();

    for (int32_t i = 0; i < eq_m7_get_size(); ++i)
    {
        add_status = eq_m7_add_event(e);
        CU_ASSERT_EQUAL(add_status, 0);
    }

    add_status = eq_m7_add_event(e);
    CU_ASSERT_EQUAL(add_status, -1);
}

void eq_m7_add_get_full_empty_test(void)
{
    int32_t add_status;
    int32_t get_status;
    event e = { .id = EVENT_M7_TRACE};
    event e_ref = { .id = EVENT_M7_TRACE};

    eq_m7_init();

    for (int32_t i = 0; i < eq_m7_get_size(); ++i)
    {
        e.val = i;
        add_status = eq_m7_add_event(e);
        CU_ASSERT_EQUAL(add_status, 0);
    }

    add_status = eq_m7_add_event(e);
    CU_ASSERT_EQUAL(add_status, -1);

    for (int32_t i = 0; i < eq_m7_get_size(); ++i)
    {
        e_ref.val = i;
        get_status = eq_m7_get_event(&e);
        CU_ASSERT_EQUAL(get_status, 0);
        assert_events(e, e_ref);
    }

    get_status = eq_m7_get_event(&e);
    CU_ASSERT_EQUAL(get_status, -1);
}

void eq_m7_add_get_full_empty_rotate_test(void)
{
    int32_t n_iter = 3;
    int32_t add_status;
    int32_t get_status;
    event e = { .id = EVENT_M7_TRACE};
    event e_ref = { .id = EVENT_M7_TRACE};

    eq_m7_init();

    for (int32_t iter = 0; iter < n_iter; ++iter)
    {
        printf("iter: %d\n", iter);
        for (int32_t i = 0; i < eq_m7_get_size(); ++i)
        {
            printf("add_event: %d\n", i);
            e.val = i;
            add_status = eq_m7_add_event(e);
            CU_ASSERT_EQUAL(add_status, 0);
        }

        add_status = eq_m7_add_event(e);
        CU_ASSERT_EQUAL(add_status, -1);

        for (int32_t i = 0; i < eq_m7_get_size(); ++i)
        {
            printf("get_event: %d\n", i);
            e_ref.val = i;
            get_status = eq_m7_get_event(&e);
            CU_ASSERT_EQUAL(get_status, 0);
            assert_events(e, e_ref);
        }

        get_status = eq_m7_get_event(&e);
        CU_ASSERT_EQUAL(get_status, -1);
    }
}


#define TESTS                                               \
    CUNIT_CI_TEST(eq_m7_add_event_test),                    \
    CUNIT_CI_TEST(eq_m7_add_and_get_event_test),            \
    CUNIT_CI_TEST(eq_m7_get_event_empty_eq_test),           \
    CUNIT_CI_TEST(eq_m7_add_event_full_eq_test),            \
    CUNIT_CI_TEST(eq_m7_add_get_full_empty_test),           \
    CUNIT_CI_TEST(eq_m7_add_get_full_empty_rotate_test)

CUNIT_CI_RUN("event_queue_test", TESTS);

