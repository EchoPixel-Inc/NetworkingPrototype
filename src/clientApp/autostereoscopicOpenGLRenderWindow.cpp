#include "clientApp/autostereoscopicOpenGLRenderWindow.h"

#include <vtkRenderer.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <vtkUnsignedCharArray.h>

vtkStandardNewMacro(AutostereoscopicOpenGLRenderWindow);
//=============================================================================
AutostereoscopicOpenGLRenderWindow::AutostereoscopicOpenGLRenderWindow() =
	default;
//=============================================================================

//=============================================================================
void AutostereoscopicOpenGLRenderWindow::AddRenderer(vtkRenderer* ren)
{
	double rendererViewport[4];
	ren->GetViewport(rendererViewport);
	ren->SetViewport(0.0, 0.0, 0.5 * rendererViewport[2], rendererViewport[3]);
	Superclass::AddRenderer(ren);
}
//=============================================================================

//=============================================================================
void AutostereoscopicOpenGLRenderWindow::StereoRenderComplete()
{
	if (this->StereoType == VTK_STEREO_SPLITVIEWPORT_HORIZONTAL) {
		const int* size = this->GetSize();
		this->GetPixelData(0, 0, size[0] - 1, size[1] - 1, !this->DoubleBuffer,
			this->ResultFrame, 0);

		auto rightBufferPtr = this->ResultFrame->GetPointer(0);
		auto leftBufferPtr = this->StereoBuffer->GetPointer(0);

		auto mid = static_cast<int>(size[0] / 2.0);

		for (unsigned int y = 0; y < size[1]; ++y) {
			for (unsigned int x = 0; x < mid; ++x) {
				auto source = rightBufferPtr + (x * 3) + (y * size[0] * 3);
				auto target =
					leftBufferPtr + ((x + mid) * 3) + (y * size[0] * 3);
				*target++ = *source++;
				*target++ = *source++;
				*target++ = *source++;
			}
		}

		std::swap(this->StereoBuffer, this->ResultFrame);
		this->StereoBuffer->Reset();
	}
	else {
		Superclass::StereoRenderComplete();
	}
}
//=============================================================================