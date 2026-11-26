/* Exercise the complete pthread runtime lifecycle, using only the generic
 * OSAL interface and pthread internals. Run before the ordinary suite and
 * leave a fresh initialized runtime for its thread reuse tests. */

#include "test.h"
#include "implement.h"

static pthread_key_t implicit_key;
static int implicit_destroyed;

static void destroy_implicit(void *arg)
{
  assert(arg == &implicit_destroyed);
  assert(implicit_destroyed == 0);
  ++implicit_destroyed;

  /* POSIX permits deleting a key from its destructor. Release the key
   * itself as well as its value before the runtime discards OSAL TLS. */
  assert(pthread_key_delete(implicit_key) == 0);
  implicit_key = NULL;
}

static void *worker(void *arg)
{
  return arg;
}

int pthread_test_lifecycle1(void)
{
  pte_osThreadHandle native = pte_osThreadGetHandle();
  int i;

  for (i = 0; i < 20; ++i)
    {
      pthread_t self, thread;
      pthread_mutex_t mutex;
      void *result = NULL;

      assert(pthread_init());
      self = pthread_self();
      assert(self != NULL);
      assert(pthread_init());
      assert(pthread_equal(self, pthread_self()));

      assert(pthread_mutex_init(&mutex, NULL) == 0);
      assert(pthread_mutex_lock(&mutex) == 0);
      assert(pthread_mutex_unlock(&mutex) == 0);
      assert(pthread_mutex_destroy(&mutex) == 0);
      assert(pthread_create(&thread, NULL, worker, &i) == 0);
      assert(pthread_join(thread, &result) == 0);
      assert(result == &i);

      /* The joined thread populates the reuse pool. Teardown must discard
       * it and the calling native thread's implicit pthread record. */
      assert(pte_threadReuseTop != PTE_THREAD_REUSE_EMPTY);
      implicit_destroyed = 0;
      assert(pthread_key_create(&implicit_key, destroy_implicit) == 0);
      assert(pthread_setspecific(implicit_key, &implicit_destroyed) == 0);
      pthread_terminate();
      assert(implicit_destroyed == 1);
      assert(implicit_key == NULL);
      assert(!pte_processInitialized);
      assert(pte_selfThreadKey == NULL);
      assert(pte_cleanupKey == NULL);
      assert(pte_threadReuseTop == PTE_THREAD_REUSE_EMPTY);
      assert(pte_threadReuseBottom == PTE_THREAD_REUSE_EMPTY);
      pthread_terminate();
      assert(!pte_processInitialized);
      assert(implicit_destroyed == 1);

      assert(pthread_init());
      assert(pte_osThreadGetHandle() == native);
      assert(pthread_getspecific(pte_selfThreadKey) == NULL);
    }

  return 0;
}
