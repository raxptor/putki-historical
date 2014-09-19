#ifndef __SYS_THREAD_H__
#define __SYS_THREAD_H__

#include <pthread.h>

namespace putki
{
	namespace sys
	{
		typedef void* (*thread_fn)(void *userptr);
		
		struct thread
		{
			pthread_t t;
		};
		
		inline thread* thread_create(thread_fn fn, void *userptr)
		{
			thread *t = new thread();
			pthread_create(&t->t, 0, fn, userptr);
			return t;
		}
		
		inline void thread_join(thread* thr)
		{
			pthread_join(thr->t, 0);
		}
		
		inline void thread_free(thread* thr)
		{
			delete thr;
		}
		
		struct mutex
		{
			mutex()
			{
				pthread_mutex_init(&mtx, 0);
			}
			
			~mutex()
			{
				pthread_mutex_destroy(&mtx);
			}
			
			void lock()
			{
				pthread_mutex_lock(&mtx);
			}
			
			void unlock()
			{
				pthread_mutex_unlock(&mtx);
			}
			
			pthread_mutex_t mtx;
		};
		
		struct scoped_maybe_lock
		{
			mutex *_m;
			scoped_maybe_lock(mutex *m) : _m(m)
			{
				if (_m) _m->lock();
			}
			~scoped_maybe_lock()
			{
				if (_m) _m->unlock();
			}
		};
		
		struct condition
		{
			condition()
			{
				pthread_cond_init(&c, 0);
			}
			
			~condition()
			{
				pthread_cond_destroy(&c);
			}
			
			void broadcast()
			{
				pthread_cond_broadcast(&c);
			}
			
			void wait(mutex *m)
			{
				if (m) pthread_cond_wait(&c, &m->mtx);
			}
			
			pthread_cond_t c;
		};
	}
}

#endif