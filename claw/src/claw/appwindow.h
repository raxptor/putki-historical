namespace claw
{
	namespace appwindow
	{
		struct data;

		data* create(const char *title, int width, int height);

		bool update(data *d); // return false if want to close.
		void destroy(data *);

#if defined(WIN32)
		void* hwnd(data *);
#endif
	}
}
