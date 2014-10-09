// Do not include manually.

#include <windows.h>

namespace putki
{
	namespace sys
	{
		typedef void* (*thread_fn)(void *userptr);

		struct thread
		{
			HANDLE thr;
			thread_fn func;
			void *userptr;
		};

		inline thread* thread_create(thread_fn fn, void *userptr)
		{
			thread *t = new thread();
			t->func = fn;
			t->userptr = userptr;
			t->thr = 0;
			return t;
		}

		inline void thread_join(thread* thr)
		{
			WaitForSingleObject(thr->thr, 0);
		}

		inline void thread_free(thread* thr)
		{
			CloseHandle(thr->thr);
			delete thr;
		}

		struct mutex
		{	
			mutex()
			{
				InitializeCriticalSection(&_cs);
			}

			~mutex()
			{
				DeleteCriticalSection(&_cs);
			}

			void lock()
			{
				EnterCriticalSection(&_cs);
			}

			void unlock()
			{
				LeaveCriticalSection(&_cs);
			}

			CRITICAL_SECTION _cs;
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
			void unlock()
			{
				if (_m)
				{
					_m->unlock();
					_m = 0;
				}
			}
		};

		struct condition
		{
			condition()
			{
			}
			condition(const condition &a)
			{
			}
			condition& operator=(condition &b)
			{
				return *this;
			}

			~condition()
			{
			}

			void broadcast()
			{
			}

			void wait(mutex *m)
			{
			}
		};
	}
}
