#include <windows.h>
#include <d3d9.h>

#include <claw/log.h>

#include <cassert>
#include <map>
#include <string>
#include <vector>
#include <cassert>

#pragma comment(lib, "d3d9.lib")

#include "render.h"

namespace claw
{
	namespace render
	{
		struct data
		{
			IDirect3D9 *dx;
			IDirect3DDevice9 *device;
		};

		data* create(appwindow::data *window)
		{
			data *d = new data();

			CLAW_INFO("Creating DirectX 9 Device");
			d->dx = Direct3DCreate9(D3D_SDK_VERSION);

			D3DPRESENT_PARAMETERS d3dpresent;
			memset( &d3dpresent, 0, sizeof( D3DPRESENT_PARAMETERS ) );
			d3dpresent.Windowed = true;
			d3dpresent.SwapEffect = D3DSWAPEFFECT_DISCARD;
			d3dpresent.BackBufferCount = 1;
			d3dpresent.BackBufferFormat = D3DFMT_X8R8G8B8;
			d3dpresent.AutoDepthStencilFormat = D3DFMT_D24S8;
			d3dpresent.EnableAutoDepthStencil = true;

			if (FAILED(d->dx->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, (HWND)appwindow::hwnd(window), D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpresent, &d->device )))
			{
				CLAW_ERROR("Could not create device!");
				return 0;
			}

			return d;
		}

		void destroy(data *d)
		{
			d->device->Release();
			d->dx->Release();
			delete d;
		}

		void begin(data *d, bool clearcolor, bool cleardepth, unsigned int clear_color)
		{
			DWORD flags = 0;
			if (clearcolor) flags |= D3DCLEAR_TARGET;
			if (cleardepth) flags |= D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;

			d->device->Clear(0, 0, flags, (DWORD) clear_color, 1.0f, 0x0);

			if (FAILED(d->device->BeginScene()))
			{
				CLAW_ERROR("Could not begin scene");
			}
		}

		void end(data *d)
		{
			if (FAILED(d->device->EndScene()))
			{
				CLAW_ERROR("Could not end scene");
			}
		}

		void present(data *d)
		{
			if (FAILED(d->device->Present(0, 0, 0, 0)))
			{
				CLAW_ERROR("Could not end scene");
			}
		}

	}
}