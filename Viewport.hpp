#pragma once

#include <functional>

#include "DX9.hpp"

namespace Viewport
{
	// render new viewport
	inline void Render
	(
		std::function<void(void)> renderProcess,
		DWORD originX,  DWORD originY,
		DWORD width,    DWORD height,
		float minZ = 0, float maxZ = 1
	)
	{
		D3DVIEWPORT9 viewport =
		{
			.X     = originX, .Y      = originY,
			.Width = width,   .Height = height,
			.MinZ  = minZ,    .MaxZ   = maxZ
		};

		// get viewport befor change
		D3DVIEWPORT9 preViewport;
		DX9::instance->pDevice->GetViewport(&preViewport);

		// change viewport
		DX9::instance->pDevice->SetViewport(&viewport);

		// render
		renderProcess();

		// restore viewport
		DX9::instance->pDevice->SetViewport(&preViewport);
	}
};