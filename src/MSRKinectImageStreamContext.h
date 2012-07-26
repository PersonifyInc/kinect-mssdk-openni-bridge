//@COPYRIGHT@//
//
// Copyright (c) 2012, Tomoto S. Washio
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//   * Neither the name of the Tomoto S. Washio nor the names of his
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL TOMOTO S. WASHIO BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//@COPYRIGHT@//

#pragma once
#include "base.h"
#include "MSRKinectFrameContext.h"

class MSRKinectImageStreamContext :
	public MSRKinectFrameContext<NUI_IMAGE_FRAME>
{
private:
	typedef MSRKinectFrameContext<NUI_IMAGE_FRAME> SuperClass;

protected:
	HANDLE m_hStreamHandle;
	NUI_IMAGE_TYPE m_eImageType;
	NUI_IMAGE_RESOLUTION m_eImageResolution;

public:
	NUI_IMAGE_RESOLUTION GetImageResolution() const { return m_eImageResolution; }
	NUI_IMAGE_TYPE GetImageType() const { return m_eImageType; }

	int GetMirrorFactor() const
	{
#if KINECTSDK_VER >= 100
		return m_bMirror ? 1 : -1;
#else
		// Weird... Only NUI_IMAGE_TYPE_DEPTH comes with flipped data
		return m_bMirror ^ (m_eImageType == NUI_IMAGE_TYPE_DEPTH) ? 1 : -1;
#endif
	}

protected:
	MSRKinectImageStreamContext(MSRKinectRequirement* pRequirement, HANDLE hNextFrameEvent) :
		SuperClass(pRequirement, hNextFrameEvent),
		m_hStreamHandle(NULL),
		m_eImageResolution(NUI_IMAGE_RESOLUTION_INVALID)
	{
	}

	virtual HRESULT GetNextFrameImpl()
	{
        HRESULT hr;
		hr = m_pRequirement->GetSensor()->NuiImageStreamGetNextFrame(m_hStreamHandle, 100, &m_frame);

        // Do the rectification here -- it takes /forever/ and we don't want to hit it in the app
        if (GetImageType() == NUI_IMAGE_TYPE_DEPTH && IsCalibrateViewPoint()) {
            // put in m_pRectFrame
            // TODO: remove hardcoding of depth frame dims here
            //OutputDebugString(L"pre-nuiimagestreamgetnextframe registration\n");
            LONG* coordinates = new LONG[640*480*2*sizeof(LONG)];
            memset(m_pRectFrame, 0, 640 * 480 * sizeof(XnDepthPixel));

            NUI_LOCKED_RECT lockedRect;
            m_frame.pFrameTexture->LockRect(0, &lockedRect, NULL, 0);
            USHORT* data = (USHORT*)lockedRect.pBits;
            USHORT* sp = NULL;

            // Try full frame registration
            // First, get the mapping
            int step = 1; // correct with our mirroring?
            HRESULT hr = GetSensor()->NuiImageGetColorPixelCoordinateFrameFromDepthPixelFrameAtResolution(
                NUI_IMAGE_RESOLUTION_640x480, NUI_IMAGE_RESOLUTION_640x480,
                640*480, (USHORT*) data,
                640*480*2, coordinates);

            //OutputDebugString(L"nuiimagestreamgetnextframe registration -- get Nui coordinates\n");

            // Then apply it
            LONG ix, iy;
            USHORT d;
            for (XnUInt32 y = 0; y < 480; y++) {
                sp = data + y * 640 + (step < 0 ? 640-1 : 0);
                for (XnUInt32 x = 0; x < 640; x++) {
                    d = (*sp);

                    // Reset unknown values to 0 (implicitly) to ensure that we don't confuse too far and unknown
                    if ((d & ~NUI_IMAGE_PLAYER_INDEX_MASK) != NUI_DEPTH_DEPTH_UNKNOWN_VALUE) {
                        ix = coordinates[(y*640 + x)*2];
                        iy = coordinates[(y*640 + x)*2+1];
                        if (ix >= 0 && ix <= LONG(640-2) && iy >= 0 && iy <= LONG(480-2)) {
                            // Note: not really any faster than process call...
                            *(m_pRectFrame + iy * 640 + ix) = d >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
                        }
                    }
                    sp += step;
                }
            }

            m_frame.pFrameTexture->UnlockRect(0);
            delete [] coordinates;
            //OutputDebugString(L"post-nuiimagestreamgetnextframe registration\n");
        }

        return hr;
	}

	virtual void ReleaseFrameImpl()
	{
		m_pRequirement->GetSensor()->NuiImageStreamReleaseFrame(m_hStreamHandle, &m_frame);
	}

};
