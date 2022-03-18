#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "barrier.h"

unsigned _Atomic __centralized_counter = ATOMIC_VAR_INIT(1);
bool __centralized_sense = true;

/// A centralized barrier.
void barrier(const n_threads_t p, const pthread_t id __attribute__((unused)),
             void *local __attribute__((unused)),
             void *global __attribute__((unused))) {
  const bool local_sense = !__centralized_sense;
  if (atomic_fetch_add(&__centralized_counter, 1) == p) {
    __centralized_counter = 1;
    __centralized_sense = local_sense;
  } else {
    while (__centralized_sense != local_sense) {
    }
  }
}

void *init_global_barrier_state(const n_threads_t p __attribute__((unused))) {
  return NULL;
}
void *init_local_barrier_state(const n_threads_t p __attribute__((unused))) {
  return NULL;
}
