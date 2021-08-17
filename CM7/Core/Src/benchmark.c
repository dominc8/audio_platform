#include "benchmark.h"
#include "perf_meas.h"
#include "event_queue.h"

#define N_BENCHMARKS    2

static uint32_t benchmark_empty(void)
{
    uint32_t start, stop;
    uint32_t acc = 0;
    int32_t n = 1 << 15;

    while (n-- > 0)
    {
        start = GET_CCNT();
        stop = GET_CCNT();
        acc += DIFF_CCNT(start, stop);
    }

    acc = acc >> 15;
    return acc;
}

static uint32_t benchmark_add(void)
{
    uint32_t start, stop;
    uint32_t acc = 0;
    int32_t n = 1 << 15;

    while (n-- > 0)
    {
        start = GET_CCNT();
        acc += 10;
        stop = GET_CCNT();
        acc += DIFF_CCNT(start, stop);
    }

    acc -= 10 << 15;
    acc = acc >> 15;
    return acc;
}

static uint32_t (*benchmarks[N_BENCHMARKS])(void) =
{
    &benchmark_empty,
    &benchmark_add,
};

static EVENT_ID bm_events[N_BENCHMARKS] =
{
    EVENT_BM_EMPTY,
    EVENT_BM_ADD,
};


void benchmark(void)
{
    for (int32_t bm = 0; bm < N_BENCHMARKS; ++bm)
    {
        uint32_t result = benchmarks[bm]();
        event e = { .id = bm_events[bm], .val = result };
        eq_m7_add_event(e);
    }
}
