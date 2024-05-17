#include <vsync/spinlock/mcslock.h>
#include <vsync/spinlock/caslock.h>
#include <vsync/spinlock/ticketlock.h>
#include <vsync/spinlock/ttaslock.h>
#include <vsync/common/assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX_THREADS 1000
#define EXPECTED_VAL(thread_count) (thread_count)
#define RUNS 10

typedef enum {
    LOCK_TYPE_MCS,
    LOCK_TYPE_CAS,
    LOCK_TYPE_TICKET,
    LOCK_TYPE_TTAS
} lock_type_t;

mcslock_t mcs_lock = MCSLOCK_INIT();
caslock_t cas_lock = CASLOCK_INIT();
ticketlock_t ticket_lock = TICKETLOCK_INIT();
ttaslock_t ttas_lock = TTASLOCK_INIT();
mcs_node_t mcs_nodes[MAX_THREADS];

vuint32_t g_x = 0;
vuint32_t g_y = 0;

lock_type_t g_lock_type;

void perform_additional_operations() {
    volatile int sum = 0;
    for (int i = 0; i < 100000; i++) {
        sum += i;
    }
}

void *run(void *args)
{
    vsize_t tid = (vsize_t)args;

    if (g_lock_type == LOCK_TYPE_MCS) {
        mcs_node_t *node = &mcs_nodes[tid];
        mcslock_acquire(&mcs_lock, node);
        g_x++;
        perform_additional_operations();
        g_y++;
        mcslock_release(&mcs_lock, node);
    } else if (g_lock_type == LOCK_TYPE_CAS) {
        caslock_acquire(&cas_lock);
        g_x++;
        perform_additional_operations();
        g_y++;
        caslock_release(&cas_lock);
    } else if (g_lock_type == LOCK_TYPE_TICKET) {
        ticketlock_acquire(&ticket_lock);
        g_x++;
        perform_additional_operations();
        g_y++;
        ticketlock_release(&ticket_lock);
    } else if (g_lock_type == LOCK_TYPE_TTAS) {
        ttaslock_acquire(&ttas_lock);
        g_x++;
        perform_additional_operations();
        g_y++;
        ttaslock_release(&ttas_lock);
    }

    (void)args;
    return NULL;
}

double get_time_diff(struct timeval start, struct timeval end)
{
    return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
}

void run_benchmark(lock_type_t lock_type, const char *lock_name, FILE *file) {
    pthread_t threads[MAX_THREADS];
    int thread_counts[] = {2,4,6,8,12,16};
    size_t num_thread_counts = sizeof(thread_counts) / sizeof(thread_counts[0]);

    g_lock_type = lock_type;

    for (size_t t = 0; t < num_thread_counts; t++) {
        int thread_count = thread_counts[t];
        double total_time = 0.0;

        for (int r = 0; r < RUNS; r++) {
            g_x = 0;
            g_y = 0;
            struct timeval start, end;

            gettimeofday(&start, NULL);

            for (vsize_t i = 0; i < thread_count; i++) {
                pthread_create(&threads[i], NULL, run, (void *)i);
            }

            for (vsize_t i = 0; i < thread_count; i++) {
                pthread_join(threads[i], NULL);
            }

            gettimeofday(&end, NULL);

            total_time += get_time_diff(start, end);

            ASSERT(g_x == EXPECTED_VAL(thread_count));
            ASSERT(g_x == g_y);
        }

        double avg_time = total_time / RUNS;
        printf("Average time with %d threads using %s lock: %f ms\n", thread_count, lock_name, avg_time);
        fprintf(file, "%s,%d,%f\n", lock_name, thread_count, avg_time);
    }
}

int main(void)
{
    FILE *file = fopen("benchmark_results.csv", "w");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    fprintf(file, "LockType,ThreadCount,Time(ms)\n");

    run_benchmark(LOCK_TYPE_MCS, "MCS", file);
    run_benchmark(LOCK_TYPE_CAS, "CAS", file);
    run_benchmark(LOCK_TYPE_TICKET, "Ticket", file);
    run_benchmark(LOCK_TYPE_TTAS, "TTAS", file);

    fclose(file);

    return 0;
}
 #Pour afficher le resultats run le fichier python  plotresults.py
 #Ajouter le fichier ici pour acceder au locks https://github.com/open-s4c/libvsync/tree/main/doc/api/vsync/spinlock
 