#include "appwindow.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace claw
{
	namespace appwindow
	{
		struct data
		{
			HWND window;
		};

		LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
		{
			switch( msg )
			{
				case WM_DESTROY:
					PostQuitMessage( 0 );
					return 0;

				case WM_PAINT:
					ValidateRect( hWnd, NULL );
					return 0;

				case WM_SIZE:
					RECT r;
					GetClientRect(hWnd, &r);
					return 0;
			}

			return DefWindowProc( hWnd, msg, wParam, lParam );
		}

		void* hwnd(data *d)
		{
			return d->window;
		}

		data* create(const char *title, int width, int height)
		{
			data *d = new data;

			WNDCLASSEX wc =
			{
				sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
				GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
				"CLAW-NG", NULL
			};

			wc.hIcon = 0; // LoadIcon(wc.hInstance, MAKEINTRESOURCE(desc.icon));
			wc.hCursor = LoadCursor( NULL, IDC_ARROW );

			::RegisterClassEx( &wc );

			RECT rc = { 
				0, 
				0, 
				width, 
				height
			};

			AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );

			// Create the application's window
			d->window = ::CreateWindowA("CLAW-NG", title,
			            WS_OVERLAPPEDWINDOW, 100, 100, rc.right - rc.left, rc.bottom - rc.top,
			            NULL, NULL, wc.hInstance, NULL );

			ShowWindow(d->window, true);

			return d;
		}

		bool update(data *d)
		{
			bool stay = true;

			MSG m;
			while (PeekMessage(&m, 0, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&m);
				DispatchMessage(&m);

				if (m.message == WM_QUIT)
					stay = false;
			}
			
			return stay;
		}

		void destroy(data *d)
		{
			delete d;
		}
	}
}
