namespace claw
{
	namespace appwindow
	{
		struct data;
		
		typedef void (*updatefunc)(void);

		data* create(const char *title, int width, int height);

		void destroy(data *);
		void set_title(data *, const char *title);
		
		void run_loop(data *d, updatefunc f);

#if defined(WIN32)
		void* hwnd(data *);
#endif
	}
}
