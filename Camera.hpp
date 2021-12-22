#pragma once

#include "const.hpp"
#include "utils.hpp"

class Camera
{
public:
	Camera(const HWND* hWnd);
	~Camera();

	void MoveFPS(const HWND* hWnd);

	const D3DXMATRIX* GetViewMatrix() const
	{
		return &this->view;
	}

	const D3DXMATRIX* GetProjectionMatrix() const
	{
		return &this->proj;
	}

private:

	D3DXVECTOR3 eye, at, up;

	D3DXMATRIX view, proj;

	POINT mousePos, preMousePos;

	float cameraSpeed;

};