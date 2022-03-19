#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "barrier.h"
#include "utils.h"

void assert_test(bool result, char name[], char message[]) {
  if (!result) {
    printf("Test %s failed: %s\n", name, message);
  }
}

typedef struct ThreadState {
  n_threads_t p;
  // we maintain a separate id from the os to make math simple
  n_threads_t id;
  void *global;
} ThreadState;

unsigned _Atomic wbw_counter = ATOMIC_VAR_INIT(0);

void *test_thread_zero(void *global) {
  ThreadState *state = global;

  void *local = init_local_barrier_state(state->p);
  sleep(1);
  atomic_store(&wbw_counter, 1);
  barrier(state->p, state->id, local, state->global);

  free_local_barrier_state(local);
  return NULL;
}

void *test_thread_one(void *global) {
  ThreadState *state = global;

  void *local = init_local_barrier_state(state->p);
  barrier(state->p, state->id, local, state->global);
  atomic_store(&wbw_counter, 2);

  free_local_barrier_state(local);
  return NULL;
}

bool test_wbw() {
  pthread_t thread_id_zero, thread_id_one;
  void *global = init_global_barrier_state(2);

  atomic_store(&wbw_counter, 0);

  ThreadState state_zero;
  state_zero.global = global;
  state_zero.id = 0;
  pthread_create(&thread_id_zero, NULL, *test_thread_zero, &state_zero);

  ThreadState state_one;
  state_one.global = global;
  state_one.id = 1;
  pthread_create(&thread_id_one, NULL, *test_thread_one, &state_one);

  pthread_join(thread_id_zero, NULL);
  pthread_join(thread_id_one, NULL);

  free_global_barrier_state(global);
  return (wbw_counter == 2);
}

unsigned _Atomic one_barrier_counter = ATOMIC_VAR_INIT(false);

/// A thread which guarantees no thread has left before it enters.
void *test_single_barrier_thread(void *global) {
  ThreadState *state = global;
  bool out = true;

  void *local = init_local_barrier_state(state->p);
  if (one_barrier_counter) {
    out = false;
  }
  barrier(state->p, state->id, local, state->global);
  atomic_store(&one_barrier_counter, true);

  free_local_barrier_state(local);
  return (void *)out;
}

bool test_single_barrier(size_t n) {
  size_t counter;
  pthread_t *ids = alloc(n, sizeof(pthread_t));
  ThreadState *states = alloc(n, sizeof(ThreadState));
  void *thread_ret = (void *)true;
  bool out = true;

  void *global = init_global_barrier_state(2);

  for (counter = 0; counter < n; counter++) {
    states[counter].global = global;
    states[counter].id = counter;
    pthread_create(&ids[counter], NULL, *test_single_barrier_thread,
                   &states[counter]);
  }

  for (counter = 0; counter < n; counter++) {
    states[counter].global = global;
    states[counter].id = counter;
    pthread_join(ids[counter], thread_ret);
    // if out is true, replace it with the thread return value; this way if
    // any thread returns false we will return false
    if (out && !*(bool *)thread_ret) {
      out = false;
    }
  }

  return out;
}

int main() {
  assert_test(test_wbw(), "write barrier write",
              "thread one wrote after thread two");
  assert_test(test_single_barrier(2), "one barrier",
              "a thread left the single barrier before every thread entered");
}
