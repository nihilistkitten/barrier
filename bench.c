#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "barrier.h"
#include "utils.h"

#define MAX_THREADS 32
#define N_BARRIERS 100
#define N_ITERS 10

void *thread(void *global) {
  ThreadState *state = global;

  void *local = init_local_barrier_state(state->p);
  for (int32_t i = 0; i < N_BARRIERS; i++) {
    barrier(state->p, state->id, local, state->global);
  }
  free_local_barrier_state(local);

  return NULL;
}

double bench(n_threads_t p) {
  struct timespec start, stop;
  double time;
  size_t counter;
  pthread_t *ids = alloc(p, sizeof(pthread_t));
  ThreadState *states = alloc(p, sizeof(ThreadState));

  void *global = init_global_barrier_state(p);

  for (counter = 0; counter < p; counter++) {
    states[counter].global = global;
    states[counter].id = counter;
    states[counter].p = p;
  }

  clock_gettime(CLOCK_MONOTONIC, &start);

  for (counter = 0; counter < p; counter++) {
    pthread_create(&ids[counter], NULL, *thread, &states[counter]);
  }

  for (counter = 0; counter < p; counter++) {
    pthread_join(ids[counter], NULL);
  }
  clock_gettime(CLOCK_MONOTONIC, &stop);

  free_global_barrier_state(global);

  time = (stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec);

  return time;
}

double bench_many(n_threads_t p) {
  double time;

  for (int32_t counter = 0; counter < N_ITERS; counter++) {
    time = time + bench(p);
  }

  return time / N_ITERS;
}

void bench_and_print(n_threads_t p) {
  double time = bench_many(p);
  printf("%u,%f\n", p, time);
}

int main() {
  printf("Threadcount,Time\n");

  srand(0);
  for (int32_t counter = 1; counter <= MAX_THREADS; counter++) {
    bench_and_print(counter);
  }

  return 0;
}
