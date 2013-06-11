namespace claw
{
	namespace appwindow
	{
		struct data;

		data* create(const char *title, int width, int height);

		bool update(data *d); // return false if want to close.
		void destroy(data *);
		void set_title(data *, const char *title);

#if defined(WIN32)
		void* hwnd(data *);
#endif
	}
}
