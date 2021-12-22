#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#include <Windows.h>

#include "const.hpp"
#include "utils.hpp"

#include "DX9.hpp"
#include "LineSegment.hpp"
#include "Camera.hpp"

#include "Viewport.hpp"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);

		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	//////////////////////////////////////////////////
	// create window

	WNDCLASSEX wcex =
	{
		.cbSize = sizeof(WNDCLASSEX),
		.style = CS_HREDRAW | CS_VREDRAW,
		.lpfnWndProc = WndProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = hInstance,
		.hIcon = nullptr,
		.hCursor = nullptr,
		.hbrBackground = nullptr,
		.lpszMenuName = nullptr,
		.lpszClassName = myApp::WINDOW_NAME,
		.hIconSm = nullptr
	};

	if (!RegisterClassEx(&wcex)) return 0;

	HWND hWnd = {};

	hWnd = CreateWindow
	(
		myApp::WINDOW_NAME,
		myApp::TITLE_NAME,
		WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME),
		320, 180,
		myApp::SCREEN_WIDTH, myApp::SCREEN_HEIGHT,
		nullptr, nullptr, hInstance, nullptr
	);

	if (!hWnd) return 0;

	ShowWindow(hWnd, nCmdShow);

	//////////////////////////////////////////////////
	// init directx9

	if (!(new DX9())->Init(&hWnd))
	{
		return 0;
	}

	//////////////////////////////////////////////////
	// init light

	D3DLIGHT9 light =
	{
		.Type      = D3DLIGHT_DIRECTIONAL,
		.Diffuse   = { 1,1,1 },
		.Ambient   = { 0.35f,0.35f,0.35f },
		.Direction = { -1, -1, -1 },
	};
	DX9::instance->pDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
	DX9::instance->pDevice->SetLight(0, &light);
	DX9::instance->pDevice->LightEnable(0, TRUE);

	//////////////////////////////////////////////////
	// init camera

	// init view & proj
	auto camera = std::make_unique<Camera>(&hWnd);

	//////////////////////////////////////////////////
	// test

	auto axisLine = std::make_unique<LineSegment>();

	ID3DXMesh* teapotMesh;
	HRESULT hr = D3DXCreateTeapot(DX9::instance->pDevice, &teapotMesh, NULL);
	if (FAILED(hr))
	{
		return false;
	}

	D3DMATERIAL9 teapotMat[] =
	{
		{
			.Diffuse = {1,0,0},
			.Ambient = {0.2,0.2,0.2}
		},
		{
			.Diffuse = {0,1,0},
			.Ambient = {0.2,0.2,0.2}
		},
		{
			.Diffuse = {0,0,1},
			.Ambient = {0.2,0.2,0.2}
		},
		{
			.Diffuse = {0.5,0.5,0.5},
			.Ambient = {0.2,0.2,0.2}
		}
	};

	D3DXMATRIX world;
	D3DXMatrixIdentity(&world);
	D3DXMATRIX rotMat;
	D3DXMatrixRotationYawPitchRoll(&rotMat, D3DX_PI / 360, 0, 0);


	D3DXMATRIX tempView;
	D3DXVECTOR3 eye(0, 5, -5), at(0, 0, 0), up(0, 1, 0);
	D3DXMatrixLookAtLH(&tempView, &eye, &at, &up);

	D3DXMATRIX tempProj;

	//////////////////////////////////////////////////
	// main loop

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			DX9::instance->pDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x192f60, 1.0f, 0);
			if (SUCCEEDED(DX9::instance->pDevice->BeginScene()))
			{
				camera->MoveFPS(&hWnd);

				//////////////////////////////////////////////////
				// axes render

				D3DXVECTOR3 start, end;

				start = { -1,0,0 }; end = { 100,0,0 };
				axisLine->SetStartEnd(&start, &end);
				axisLine->Render();
				start = { 0,-1,0 }; end = { 0,100,0 };
				axisLine->SetStartEnd(&start, &end);
				axisLine->Render();
				start = { 0,0,-1 }; end = { 0,0,100 };
				axisLine->SetStartEnd(&start, &end);
				axisLine->Render();

				//////////////////////////////////////////////////
				// main render

				static D3DXMATRIX inv;
				D3DXMatrixInverse(&inv, 0, &world);
				DX9::instance->pDevice->SetTransform(D3DTS_WORLD, &inv);
				DX9::instance->pDevice->SetMaterial(&teapotMat[3]);
				teapotMesh->DrawSubset(0);

				//////////////////////////////////////////////////
				// split render

				// change view
				DX9::instance->pDevice->SetTransform(D3DTS_VIEW, &tempView);

				// change projection
				D3DXMatrixPerspectiveFovLH(&tempProj, D3DX_PI * 0.25f, 300.0f / 200.0f, 0.01f, 1000.0f);
				DX9::instance->pDevice->SetTransform(D3DTS_PROJECTION, &tempProj);

				// render 4 viewports
				for (int i = 0; i < 4; ++i)
				{
					world *= rotMat;
					Viewport::Render
					(
						[&]()
						{
							DX9::instance->pDevice->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x757575, 1.0f, 0);
							DX9::instance->pDevice->SetTransform(D3DTS_WORLD, &world);
							DX9::instance->pDevice->SetMaterial(&teapotMat[i]);
							teapotMesh->DrawSubset(0);
						},								// process
						i * 350,						// x
						myApp::SCREEN_HEIGHT - 200,		// y
						300,							// width
						200								// height
					);
				}

				// restore projection
				DX9::instance->pDevice->SetTransform(D3DTS_PROJECTION, camera->GetProjectionMatrix());

				// restore view
				DX9::instance->pDevice->SetTransform(D3DTS_VIEW, camera->GetViewMatrix());



				DX9::instance->pDevice->EndScene();
			}
			DX9::instance->pDevice->Present(NULL, NULL, NULL, NULL);
		}
	}

	SAFE_RELEASE(teapotMesh);

	OutputDebugFormat("\n{} Finished.\n\n", myApp::TITLE_NAME);

	return 0;
}