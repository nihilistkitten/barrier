#include <pthread.h>
#include <stdint.h>

typedef uint32_t n_threads_t;

/// Initialize the global state.
void *init_global_barrier_state(const n_threads_t p);

/// Initialize the thread-local state.
void *init_local_barrier_state(const n_threads_t p);

/// A barrier.
void barrier(const uint32_t p, const pthread_t id, void *local, void *global);

/// Free the local state.
void free_local_barrier_state(void *state);

/// Free the global state.
void free_global_barrier_state(void *state);
