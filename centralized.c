#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "barrier.h"

unsigned _Atomic __centralized_counter = ATOMIC_VAR_INIT(1);
bool __centralized_sense = true;

/// A centralized barrier.
void barrier(const uint32_t p, const pthread_t id __attribute__((unused))) {
  bool local_sense = !__centralized_sense;
  if (atomic_fetch_add(&__centralized_counter, 1) == p) {
    __centralized_counter = 1;
    __centralized_sense = local_sense;
  } else {
    while (__centralized_sense != local_sense) {
    }
  }
}
