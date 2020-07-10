#include "D3D11BufferHandler.h"

void SG::D3D11BufferHandler::SwapUpdateBuffer()
{
	std::swap(toUpdate, toUseNext);
}

void SG::D3D11BufferHandler::SwapToWorkWithBuffer()
{
	std::swap(toWorkWith, toUseNext);
}
