#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include <claw/log.h>

#include <cassert>
#include <map>
#include <string>
#include <vector>
#include <cassert>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include "render.h"

namespace claw
{
	namespace render
	{
#pragma pack(push, 1)
		struct SolidVert
		{
			float x, y, z;
			DWORD color;
		};
#pragma pack(pop)

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

			D3DXMATRIX mtx;
			D3DXMatrixOrthoLH(&mtx, 800, 600, 0.1f, 100.0f);
			
			D3DXMATRIX sc;
			D3DXMatrixScaling(&sc, 1, -1, 1);
			D3DXMATRIX ofs;
			D3DXMatrixTranslation(&ofs, -400, -300, 0);

			mtx = ofs * sc * mtx;
			d->device->SetTransform(D3DTS_PROJECTION, &mtx);

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

		void solid_rect(data *d, float x0, float y0, float x1, float y1, unsigned int color)
		{
			SolidVert v[4];
			v[0].x = x0;
			v[0].y = y0;
			v[0].z = 1.0f;
			v[0].color = color;

			v[1].x = x1;
			v[1].y = y0;
			v[1].z = 1.0f;
			v[1].color = color;

			v[2].x = x0;
			v[2].y = y1;
			v[2].z = 1.0f;
			v[2].color = color;

			v[3].x = x1;
			v[3].y = y1;
			v[3].z = 1.0f;
			v[3].color = color;

			d->device->SetRenderState(D3DRS_LIGHTING, false);
			d->device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
			
			//d->device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			//d->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			d->device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(SolidVert));
		}

	}
}