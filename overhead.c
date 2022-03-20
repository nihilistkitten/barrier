#include <pthread.h>
#include <stdint.h>

#include "barrier.h"

// NOP barrier, obviously incorrect, used for computing loop overhead for
// benchmarking.
void barrier(const n_threads_t p __attribute__((unused)),
             const pthread_t id __attribute__((unused)),
             void *local __attribute__((unused)),
             void *global __attribute__((unused))) {}

void *init_global_barrier_state(const n_threads_t p __attribute__((unused))) {
  return NULL;
}
void *init_local_barrier_state(const n_threads_t p __attribute__((unused))) {
  return NULL;
}
void free_local_barrier_state(void *state __attribute__((unused))) {}
void free_global_barrier_state(void *state __attribute__((unused))) {}
