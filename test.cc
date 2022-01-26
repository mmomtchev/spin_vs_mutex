#include "spinlock.h"
#include <chrono>
#include <mutex>
#include <stdio.h>
#include <thread>
#include <vector>

thread_local unsigned data_local[4] = {0, 1, 2, 3};

void work_local(unsigned nanoseconds) {
    auto start = std::chrono::high_resolution_clock::now();

    while (static_cast<unsigned>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) <
           nanoseconds) {
        data_local[0] = data_local[1] * data_local[2] + data_local[3];
        data_local[1] = data_local[0] * data_local[2] + data_local[3];
        data_local[2] = data_local[0] * data_local[1] + data_local[3];
        data_local[3] = data_local[0] * data_local[2] + data_local[2];
    }
}

unsigned data_shared[4] = {0, 1, 2, 3};

void work_shared(unsigned nanoseconds) {
    auto start = std::chrono::high_resolution_clock::now();

    while (static_cast<unsigned>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count()) <
           nanoseconds) {
        data_shared[0] = data_shared[1] * data_shared[2] + data_shared[3];
        data_shared[1] = data_shared[0] * data_shared[2] + data_shared[3];
        data_shared[2] = data_shared[0] * data_shared[1] + data_shared[3];
        data_shared[3] = data_shared[0] * data_shared[2] + data_shared[2];
    }
}

unsigned iteration = 0;

/**
 * test <% locked work> <work cycle duration> <test duration>
 */
int main(int argc, char *argv[]) {
    const char usage[] = "Usage: %s <%% locked work> <work cycle duration µs> <test duration seconds> <number of threads>\n";

    if (argc < 5) {
        printf(usage, argv[0]);
        return 1;
    }

#if defined(MUTEX)
    std::mutex lock;
#elif defined(WTFMUTEX)
    WTF::Lock lock;
#elif defined(TASLOCK)
    tas_lock lock;
#elif defined(TASLOCK_RELAXED)
    tas_lock_nofence lock;
#elif defined(TTASLOCK)
    ttas_lock lock;
#elif defined(AMDLOCK)
    amd_lock lock;
#endif
    unsigned cycle = std::atoi(argv[2]);
    unsigned duration = std::atoi(argv[3]);
    unsigned local_duration = 1000 * std::atoi(argv[1]) * cycle / 100;
    unsigned shared_duration = 1000 * (100 - std::atoi(argv[1])) * cycle / 100;
    unsigned nthreads = std::atoi(argv[4]);

    if (!cycle || !duration || !local_duration || !nthreads) {
        printf(usage, argv[0]);
        return 1;
    }

    // printf("running %u µs local + %u µs shared for %u seconds with %u threads\n", local_duration, shared_duration, duration, nthreads);

    std::vector<std::thread> threads(nthreads);
    for (unsigned i = 0; i < nthreads; i++) {
        threads[i] = std::thread([duration, local_duration, shared_duration, &lock]() {
            auto start = std::chrono::high_resolution_clock::now();

            while (static_cast<unsigned>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start).count()) <
                   duration) {
                work_local(local_duration);
                lock.lock();
                work_shared(shared_duration);
                iteration++;
                lock.unlock();
            }
        });
    }

    for (auto &t : threads)
        t.join();

    unsigned long long expected = std::thread::hardware_concurrency() * duration * 1000000 / cycle;
    printf("%llu%%", 100ULL * iteration / expected);
    // printf("%u %u %u\n", iteration, nthreads, duration * 1000000000 / iteration);
    // printf("%u ([%u+%u] %u/%u)\n", 100 * iteration / expected, local_duration, shared_duration, expected, iteration);
    return 0;
}
