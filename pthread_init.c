
/*
 * pthread_init.c
 *
 * Description:
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-embedded (PTE) - POSIX Threads Library for embedded systems
 *      Copyright(C) 2008 Jason Schmidlapp
 *
 *      Contact Email: jschmidlapp@users.sourceforge.net
 *
 *
 *      Based upon Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2005 Pthreads-win32 contributors
 *
 *      Contact Email: rpj@callisto.canberra.edu.au
 *
 *      The original list of contributors to the Pthreads-win32 project
 *      is contained in the file CONTRIBUTORS.ptw32 included with the
 *      source code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "pthread.h"
#include <stdio.h>
#include <stdlib.h>

#include "implement.h"

int pthread_init(void)
{
  pte_osMutexHandle *locks[] = {
    &pte_thread_reuse_lock, &pte_mutex_test_init_lock,
    &pte_cond_list_lock, &pte_cond_test_init_lock,
    &pte_rwlock_test_init_lock, &pte_spinlock_test_init_lock
  };
  unsigned int initialized = 0;

  if (pte_processInitialized)
    return PTE_TRUE;

  if (pte_osInit() != PTE_OS_OK)
    return PTE_FALSE;

  if (pthread_key_create (&pte_selfThreadKey, NULL) != 0 ||
      pthread_key_create (&pte_cleanupKey, NULL) != 0)
    goto fail;

  for (; initialized < sizeof(locks) / sizeof(locks[0]); ++initialized)
    if (pte_osMutexCreate (locks[initialized]) != PTE_OS_OK)
      goto fail;

  pte_processInitialized = PTE_TRUE;
  return PTE_TRUE;

fail:
  if (pte_cleanupKey != NULL)
    pthread_key_delete (pte_cleanupKey);
  if (pte_selfThreadKey != NULL)
    pthread_key_delete (pte_selfThreadKey);
  pte_cleanupKey = NULL;
  pte_selfThreadKey = NULL;
  while (initialized > 0)
    pte_osMutexDelete (*locks[--initialized]);
  pte_osTerminate();
  return PTE_FALSE;
}
