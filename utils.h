#pragma once

#include "barrier.h"
#include <stdlib.h>

/// The state needed to be passed to a thread for it to run barriers.
typedef struct ThreadState {
  // we maintain a separate id from the os to make math simple
  pthread_t id;
  n_threads_t p;
  void *global;
} ThreadState;

/// Safely allocate.
void *alloc(size_t num, size_t size);
