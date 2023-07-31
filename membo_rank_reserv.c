#include "ring-buf.h"
#include "membo_rank_reserv.h"
#include <stdio.h>
#include <dpu.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

int accumulate(int x[], int n)
{
    int ret = 0;

    for (int i = 0; i < n; ++i) {
        ret += x[i];
    }

    return ret;
}

static double calculateB(int x[], int y[], int n)
{
    int sx = accumulate(x, n);
    int sy = accumulate(y, n);
    int sxsy = 0;
    int sx2 = 0;

    for(int i = 0; i < n; i++)
    {
        sxsy += x[i] * y[i];
         sx2 += x[i] * x[i];
    }

    if (n * sx2 == sx * sx)
        return 0;

    double b = (double)(n * sxsy - sx * sy) /
                       (n * sx2 - sx * sx);
 
    return b;
}

double calculate_threshold( int X[], int Y[], int n, int current_usage)
{
    double b = calculateB(X, Y, n);

    if (b == 0)
        return -1;

    int meanX = accumulate(X, n) / n;
    int meanY = accumulate(Y, n) / n;

    double a = meanY - b * meanX;

    return a + b * current_usage;
}

RINGBUF_TYPE_DECLARE(int, dpu_usage_t);
RINGBUF_DECLARE(dpu_usage_t, dpu_usage, int, NR_SAMPLE_POINTS);

int main()
{
#if STATIC == 1
    dpu_membo_set_threshold(CONFIG_STATIC_THRESHOLD);
#elif DYNAMIC == 1
    int x[NR_SAMPLE_POINTS - 1] = {0};
    int y[NR_SAMPLE_POINTS - 1] = {0};
    RINGBUF_INIT(dpu_usage, NR_SAMPLE_POINTS, uint32_t, dpu_usage_t);

    while (1) {
        //ringBufWrite
        int *usage;
        uint32_t current_usage;
        
        dpu_membo_get_usage(&current_usage);
        ringBufWrite(&dpu_usage, current_usage);

        //printf("current usage is %d\n", current_usage);
        //system("free -m | grep Mem: | sed 's/Mem://g'");

        if (!is_ringbuf_full(&dpu_usage)) {
            goto cont;
        }

        ringBufRead(&dpu_usage, usage);

        x[0] = *usage;
        for (int i = 0; i < NR_SAMPLE_POINTS - 2; ++i) {
            ringBufRead(&dpu_usage, usage);
            y[i] = *usage;
            x[i + 1] = *usage;
        }

        ringBufRead(&dpu_usage, usage);
        y[NR_SAMPLE_POINTS - 2] = *usage;
        ringBufRead(&dpu_usage, usage);

        int next_threshold = ceil(calculate_threshold(x, y, NR_SAMPLE_POINTS - 1, current_usage));

        next_threshold *= 2;

        if (next_threshold >= 0 && next_threshold <= 16) {
            printf(KBLU"lir-membo: set threshold to %d\n", next_threshold);
            dpu_membo_set_threshold(next_threshold);
        } else if (next_threshold > 16) {
            printf(KBLU"lir-membo: set threshold to %d\n", 16);
            dpu_membo_set_threshold(16);
        } else {
            printf(KBLU"lir-membo: threshold not changed\n");
        }

cont:
        sleep(CONFIG_SAMPLING_PERIOD_IN_SEC);
    }
#endif

    return 0;
}
