//--------------------------------------------------------------------------------------
// zlibDecode.h
//
// Overlapped I/O decompression of zlib compressed files
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


#ifndef _ZLIBDecode


class ZLIBDecode 
{
public:
	ZLIBDecode(){memset(this, 0, sizeof(*this));}
	DWORD	InitFileDecompress(BYTE * pBuff,  UINT uiBytes, const HANDLE HWFile, OVERLAPPED *pOVLw, const HANDLE HRFile, OVERLAPPED *pOVLr, DWORD dwFileSize, bool bBlockonRead);
	DWORD	InitMmDecompress(BYTE * pIPBuff,  BYTE * pOPBuff,  UINT uiIPBytes, UINT uiOPBytes, const HANDLE HRFile, OVERLAPPED *pOVLr, DWORD dwFileSize, bool bBlockonRead);
	HRESULT	DecompressStreamToMemory();
	HRESULT	DecompressStreamToFile();
	HRESULT DecodeImmediate(BYTE * pIPBuff,  BYTE * pOPBuff,  UINT uiIPBytes, UINT uiOPBytes);

protected:

	void ReadWriteByteInc();
	BYTE ReadIPByteInc();
	void WriteOPByteInc(BYTE op);
	HRESULT DecodeStreamToMemory();
	HRESULT DecodeStreamToFile();

	HRESULT WriteDecompressedFile( UINT uiCount );
	HRESULT ReadCompressedFile();
	DWORD SleepIfWaiting(const HANDLE HFile, OVERLAPPED *pOVR, DWORD *pdwBytes);
	DWORD BlockOnFileIO(const HANDLE HFile, OVERLAPPED *pOVR, DWORD *pdwBytes);


	OVERLAPPED *	m_pOVLr;
	OVERLAPPED *	m_pOVLw;
	HANDLE	m_HRFile;
	HANDLE	m_HWFile;
	BYTE *	m_pRead;
	BYTE *	m_pDecInp;
	BYTE *	m_pDecOut;
	BYTE *	m_pWrite;
	UINT	m_uiReadSize;
	UINT	m_uiWriteSize;
	DWORD	m_dwBytesRead;
	DWORD	m_dwReadCount;
	DWORD	m_dwBytesRemaining;
	bool	m_bWriteWaiting : 1;
	bool	m_bReadWaiting : 1;
	bool	m_bWriteOPFile : 1;
	bool	m_BlockOnRead : 1;
};


#endif //_ZLIBDecode