#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "barrier.h"

void assert_test(bool result, char name[], char message[]) {
  if (!result) {
    printf("Test %s failed: %s", name, message);
  }
}

unsigned _Atomic __test_counter = ATOMIC_VAR_INIT(0);

void *test_thread_one(void *global) {
  void *local = init_local_barrier_state(2);
  sleep(1);
  atomic_store(&__test_counter, 1);
  barrier(2, 0, local, global);

  free(local);
  return NULL;
}

void *test_thread_two(void *global) {
  void *local = init_local_barrier_state(2);
  barrier(2, 1, local, global);
  atomic_store(&__test_counter, 2);

  free(local);
  return NULL;
}

bool test_simple() {
  pthread_t thread_id_one, thread_id_two;
  void *global = init_global_barrier_state(2);

  atomic_store(&__test_counter, 0);

  pthread_create(&thread_id_one, NULL, *test_thread_one, global);
  pthread_create(&thread_id_two, NULL, *test_thread_two, global);

  pthread_join(thread_id_one, NULL);
  pthread_join(thread_id_two, NULL);

  free(global);
  return (__test_counter == 2);
}

int main() {
  assert_test(test_simple(), "simple", "thread one wrote after thread two");
}
