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
#include "MSRKinectGeometricFrameContextBase.h"

template <class FrameClass>
class MSRKinectFrameContext :
	public MSRKinectGeometricFrameContextBase
{
private:
	typedef MSRKinectGeometricFrameContextBase SuperClass;

protected:
	FrameClass m_frame;
	USHORT* m_pRectFrame;
	BOOL m_hasFrame;

public:
	MSRKinectFrameContext(MSRKinectRequirement* pRequirement, HANDLE hNextFrameEvent) : SuperClass(pRequirement, hNextFrameEvent), m_hasFrame(FALSE)
	{
		// TODO: don't hardcode size here
		m_pRectFrame = new USHORT[640*480];
	}
	
	virtual ~MSRKinectFrameContext()
	{
		if (m_pRectFrame) {
			delete [] m_pRectFrame;
			m_pRectFrame = NULL;
		}
	}

	HRESULT GetNextFrame()
	{
		LockFrame();
		ReleaseFrame();
		HRESULT hr = GetNextFrameImpl();
		if (SUCCEEDED(hr)) {
			m_nFrameID++;
			m_lTimestamp = m_frame.liTimeStamp.QuadPart * 1000;
			m_hasFrame = TRUE;
		}
		UnlockFrame();
		return hr;
	}

	void ReleaseFrame()
	{
		LockFrame();
		if (m_hasFrame) {
			ReleaseFrameImpl();
			m_hasFrame = FALSE;
		}
		UnlockFrame();
	}

	FrameClass* GetFrame()
	{
		return &m_frame;
	}

	const USHORT* LockRectFrame()
	{
		LockFrame();
		return m_pRectFrame;
	}

protected:
	virtual HRESULT GetNextFrameImpl() = 0;
	virtual void ReleaseFrameImpl() = 0;
};
