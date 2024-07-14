 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Threading support, using pthreads
  *
  * Copyright 1997 Bernd Schmidt
  * Copyright 2004 Richard Drummond
  */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
typedef HANDLE uae_sem_t;
typedef HANDLE uae_thread_id;

extern void uae_sem_destroy(uae_sem_t*);
extern int uae_sem_trywait(uae_sem_t*);
extern int uae_sem_trywait_delay(uae_sem_t*, int);
extern void uae_sem_post(uae_sem_t*);
extern void uae_sem_wait(uae_sem_t*t);
extern void uae_sem_init(uae_sem_t*, int manual_reset, int initial_state);
extern int uae_start_thread(const TCHAR *name, void (*f)(void *), void *arg, uae_thread_id *thread);
extern int uae_start_thread_fast(void (*f)(void *), void *arg, uae_thread_id *thread);
extern void uae_end_thread(uae_thread_id *thread);
extern void uae_set_thread_priority(uae_thread_id *, int);
extern uae_thread_id uae_thread_get_id(void);

#include "commpipe.h"

STATIC_INLINE void uae_wait_thread (uae_thread_id tid)
{
    WaitForSingleObject (tid, INFINITE);
    CloseHandle (tid);
}

#else /* WIN32 */

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    sem_t *sem;
} uae_sem_t;

#ifndef USE_NAMED_SEMAPHORES
int uae_sem_init (uae_sem_t *sem, int pshared, unsigned int value);

STATIC_INLINE int uae_sem_destroy (uae_sem_t *sem)
{
    return sem_destroy (sem->sem);
}

STATIC_INLINE int uae_sem_post (uae_sem_t *sem)
{
    return sem_post (sem->sem);
}

STATIC_INLINE int uae_sem_wait (uae_sem_t *sem)
{
    return sem_wait (sem->sem);
}

STATIC_INLINE int uae_sem_trywait_delay(uae_sem_t *sem, int millis)
{
    return sem_trywait (sem->sem);
}

STATIC_INLINE int uae_sem_trywait (uae_sem_t *sem)
{
	return uae_sem_trywait_delay(sem, 0);
}

STATIC_INLINE int uae_sem_getvalue (uae_sem_t *sem, int *sval)
{
    return sem_getvalue (sem->sem, sval);
}

#else

int uae_sem_init (uae_sem_t *sem, int pshared, unsigned int value);

STATIC_INLINE int uae_sem_destroy (uae_sem_t *sem)
{
    return sem->sem == 0 ? -1 : sem_close (sem->sem);
}

STATIC_INLINE int uae_sem_post (uae_sem_t *sem)
{
    return sem->sem == 0 ? -1 : sem_post (sem->sem);
}

STATIC_INLINE int uae_sem_wait (uae_sem_t *sem)
{
    return sem->sem == 0 ? -1 : sem_wait (sem->sem);
}

STATIC_INLINE int uae_sem_trywait_delay(uae_sem_t *sem, int millis)
{
    return sem->sem == 0 ? -1 : sem_trywait (sem->sem);
}

STATIC_INLINE int uae_sem_trywait (uae_sem_t *sem)
{
	return uae_sem_trywait_delay(sem, 0);
}

STATIC_INLINE int uae_sem_getvalue (uae_sem_t *sem, int *sval)
{
    return sem->sem == 0 ? -1 : sem_getvalue (sem->sem, sval);
}
#endif /* USE_NAMED_SEMAPHORES */

#include "commpipe.h"

typedef pthread_t uae_thread_id;
#define BAD_THREAD 0

STATIC_INLINE int uae_start_thread (char *name, void (*f)(void *), void *arg, uae_thread_id *foo)
{
    int result;
    uae_thread_id new_foo;
    if (!foo)
       foo = &new_foo;
    result = pthread_create (foo, NULL, (void*)f, arg);

    return 0 == result;
}

STATIC_INLINE int uae_start_thread_fast (void (*f)(void *), void *arg, uae_thread_id *tid)
{
    int v = uae_start_thread (NULL, f, arg, tid);
#if 0
    if (*tid)
    {
        int policy;
        struct sched_param param;
        pthread_getschedparam(pthread_self(), &policy, &param);
        param.sched_priority = sched_get_priority_max(policy);
        pthread_setschedparam(pthread_self(), policy, &param);
    }
#endif
    return v;
}

STATIC_INLINE int uae_wait_thread (uae_thread_id thread)
{
    return pthread_join (thread, (void**)0);
}

STATIC_INLINE void uae_kill_thread (uae_thread_id* thread)
{
    pthread_detach(*thread);
}

STATIC_INLINE uae_thread_id uae_thread_get_id(void)
{
    return pthread_self();
}

STATIC_INLINE void uae_end_thread (uae_thread_id *tid)
{
#if 0
    if (*tid)
        pthread_cancel(*tid);
#endif
}

STATIC_INLINE void uae_set_thread_priority (uae_thread_id *tid, int pri)
{
#if 0
    if (*tid)
        pthread_setschedprio(*tid, pri);
#endif
}

#endif
