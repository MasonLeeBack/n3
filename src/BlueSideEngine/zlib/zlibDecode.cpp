//--------------------------------------------------------------------------------------
// zlibDecode.cpp
//
// Overlapped I/O decompression of zlib compressed files
//
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "stdafx.h"
#include "zlibDecode.h"
#include "assert.h"
#include "zlib.h"
#include "BsFileIO.h"

DWORD ZLIBDecode::InitFileDecompress(BYTE * pBuff,  UINT uiBytes, const HANDLE HWFile, OVERLAPPED *pOVLw, const HANDLE HRFile, OVERLAPPED *pOVLr, DWORD dwFileSize, bool bBlockOnRead)
{
	assert(pOVLw);
	assert(pOVLr);
	assert(HWFile != INVALID_HANDLE_VALUE);
	assert(HRFile != INVALID_HANDLE_VALUE);

	m_uiReadSize = uiBytes / 6;
	m_uiWriteSize = m_uiReadSize * 2;
	
	assert((m_uiReadSize % _DVD_SECTOR_SIZE) == 0);
	assert((m_uiWriteSize % _HD_SECTOR_SIZE) == 0);

	m_pRead		= pBuff;
	m_pDecInp	= m_pRead + m_uiReadSize;
	m_pDecOut	= m_pDecInp + m_uiReadSize;
	m_pWrite	= m_pDecOut + m_uiWriteSize;
	m_pOVLr = pOVLr;
	m_pOVLw = pOVLw;
	m_HRFile = HRFile;
	m_HWFile = HWFile;
	m_dwBytesRemaining = dwFileSize;
	m_bWriteOPFile = true;
	m_BlockOnRead = bBlockOnRead;

	return ERROR_SUCCESS;
}

DWORD ZLIBDecode::InitMmDecompress(BYTE * pIPBuff,  BYTE * pOPBuff,  UINT uiIPBytes, UINT uiOPBytes, const HANDLE HRFile, OVERLAPPED *pOVLr, DWORD dwFileSize, bool bBlockOnRead)
{
	assert(pIPBuff);
	assert(pOPBuff);
	assert(pOVLr);
	assert(HRFile != INVALID_HANDLE_VALUE);

	m_uiReadSize = uiIPBytes / 2;
	m_uiWriteSize = uiOPBytes;

	assert((m_uiReadSize % _DVD_SECTOR_SIZE) == 0);

	m_pRead		= pIPBuff;
	m_pDecInp	= m_pRead + m_uiReadSize;
	m_pDecOut	= pOPBuff;
	m_pOVLr = pOVLr;
	m_HRFile = HRFile;
	m_dwBytesRemaining = dwFileSize;
	m_bWriteOPFile = false;
	m_BlockOnRead = bBlockOnRead;

	return ERROR_SUCCESS;

}

HRESULT ZLIBDecode::DecompressStreamToFile()
{
	// Kickoff the first data read
	if(ReadCompressedFile() != ERROR_SUCCESS)
	{
		return E_FAIL;
	}

	// Start next read - will wait for first read to finish
	if(ReadCompressedFile() != ERROR_SUCCESS)
	{
		return E_FAIL;
	}

	return DecodeStreamToFile();
}


HRESULT ZLIBDecode::DecompressStreamToMemory()
{
	// Kickoff the first data read
	if(ReadCompressedFile() != ERROR_SUCCESS)
	{
		return E_FAIL;
	}

	// Start next read - will wait for first read to finish
	if(ReadCompressedFile() != ERROR_SUCCESS)
	{
		return E_FAIL;
	}

	return DecodeStreamToMemory();
}


DWORD ZLIBDecode::SleepIfWaiting(const HANDLE HFile, OVERLAPPED *pOVR, DWORD *pdwBytes)
{
	// Sleep if there is outstanding I/O
	BOOL bOK = false;

	while(!bOK)
	{
		bOK = CBsFileIO::BsGetOverlappedResult(HFile, pOVR, pdwBytes, false);
		if(bOK == 0)
		{
			DWORD dwErr = GetLastError();
			if(dwErr == ERROR_IO_INCOMPLETE)
				Sleep(0);
			else
				return dwErr;
		}
	}

	pOVR->Offset += *pdwBytes;
	return ERROR_SUCCESS;
}

DWORD ZLIBDecode::BlockOnFileIO(const HANDLE HFile, OVERLAPPED *pOVR, DWORD *pdwBytes)
{
	DWORD dwErr = ERROR_SUCCESS;
	BOOL bOK = 0;

	do
	{
		bOK = CBsFileIO::BsGetOverlappedResult(HFile, pOVR, pdwBytes, false);
		if(bOK == 0)
		{
			dwErr = GetLastError();
			if(dwErr != ERROR_IO_INCOMPLETE)
				dwErr = E_FAIL;
		}
	}while(bOK == 0 && dwErr != E_FAIL);

	if(bOK)
	{
		dwErr = ERROR_SUCCESS;
		pOVR->Offset += *pdwBytes;
	}
	
	return dwErr;

}

HRESULT ZLIBDecode::WriteDecompressedFile(UINT uiCount)
{
	BOOL bOK;
	DWORD dwBytesWritten;
	BYTE * pTmp;
	DWORD dwErr = ERROR_SUCCESS;

	// Wait for any preceeding I/O
	if(m_bWriteWaiting)
	{
		dwErr = SleepIfWaiting(m_HWFile, m_pOVLw, &dwBytesWritten);
		m_bWriteWaiting = false;
		if(dwErr != ERROR_SUCCESS)
		{
			return dwErr;
		}
	}

	if(uiCount)
	{
		//Swap decode and write buffer pointers
		pTmp = m_pWrite; m_pWrite = m_pDecOut; m_pDecOut = pTmp;

		bOK = CBsFileIO::BsWriteFile( m_HWFile, m_pWrite, uiCount, &dwBytesWritten, m_pOVLw );
		if( bOK && dwBytesWritten != uiCount )
		{
			return E_FAIL;
		}

		if(bOK == 0)
			m_bWriteWaiting = true;
		else
			m_pOVLw->Offset += dwBytesWritten;
	}
	return ERROR_SUCCESS;
}


HRESULT ZLIBDecode::ReadCompressedFile()
{
	BOOL bOK;
	DWORD dwErr = ERROR_SUCCESS;
	BYTE * pTmp;

	if(m_bReadWaiting)
	{
		if(m_BlockOnRead)
			dwErr = BlockOnFileIO(m_HRFile, m_pOVLr, &m_dwBytesRead);
		else
			dwErr = SleepIfWaiting(m_HRFile, m_pOVLr, &m_dwBytesRead);
		m_bReadWaiting = false;
		if(dwErr != ERROR_SUCCESS)
		{
			return dwErr;
		}
		m_dwBytesRemaining -= m_dwBytesRead;
	}

	// Swap decode and read buffer pointers
	pTmp = m_pRead; m_pRead = m_pDecInp; m_pDecInp = pTmp;
	m_dwReadCount = m_dwBytesRead;

	if(m_dwBytesRemaining)
	{
		DWORD dwReadSize = m_dwBytesRemaining > m_uiReadSize ? m_uiReadSize : m_dwBytesRemaining;
		bOK = CBsFileIO::BsReadFile( m_HRFile, m_pRead, dwReadSize, &m_dwBytesRead, m_pOVLr );
		if(( bOK && m_dwBytesRead != dwReadSize ))
		{
			return E_FAIL;
		}

		if(bOK == 0)
			m_bReadWaiting = true;
		else
		{
			m_pOVLr->Offset += m_dwBytesRead;
			m_dwBytesRemaining -= m_dwBytesRead;
		}
	}

	return dwErr;
}


HRESULT ZLIBDecode::DecodeImmediate(BYTE * pIPBuff,  BYTE * pOPBuff,  UINT uiIPBytes, UINT uiOPBytes)
{
	z_stream zcpr;
	int ret=Z_OK;
	HRESULT hr = S_OK;
	memset(&zcpr,0,sizeof(z_stream));
	inflateInit(&zcpr);

	zcpr.avail_out = uiOPBytes;
	zcpr.next_out = pOPBuff;

#ifdef _XBOX
	const DWORD dwPrefetchDistance = 1024;
	const DWORD dwPPCCacheLine = 128;

	for( DWORD i = 0; i < dwPrefetchDistance; i += dwPPCCacheLine )
		__dcbt( i, pIPBuff );

	while(ret == Z_OK)
	{
		__dcbt( dwPrefetchDistance , pIPBuff );

		zcpr.avail_in = dwPPCCacheLine;
		zcpr.next_in = pIPBuff;
		ret=inflate(&zcpr,Z_SYNC_FLUSH);
		pIPBuff += dwPPCCacheLine;
	}
#else
	zcpr.avail_in = uiIPBytes;
	zcpr.next_in = pIPBuff;
	ret=inflate(&zcpr,Z_FINISH);
#endif

	inflateEnd(&zcpr);

	if(ret==Z_STREAM_END)
		return S_OK;
	else
		return E_FAIL;
}


HRESULT ZLIBDecode::DecodeStreamToFile()
{
	z_stream zcpr;
	int ret=Z_OK;
	HRESULT hr = S_OK;
	memset(&zcpr,0,sizeof(z_stream));
	inflateInit(&zcpr);

	zcpr.next_in = m_pDecInp;
	zcpr.avail_in = m_dwReadCount < m_uiReadSize ? m_dwReadCount : m_uiReadSize;
	zcpr.next_out = m_pDecOut;
	zcpr.avail_out = m_uiWriteSize;

	while (ret==Z_OK && hr==S_OK)
	{
		ret=inflate(&zcpr,Z_SYNC_FLUSH);
		if(zcpr.avail_in == 0 && ret==Z_OK)
		{
			hr = ReadCompressedFile();
			zcpr.next_in = m_pDecInp;
			zcpr.avail_in = m_dwReadCount < m_uiReadSize ? m_dwReadCount : m_uiReadSize;
		}
		if(m_bWriteOPFile && zcpr.avail_out == 0 && (ret==Z_OK || ret==Z_STREAM_END))
		{
			hr = WriteDecompressedFile(m_uiWriteSize);
			zcpr.next_out = m_pDecOut;
			zcpr.avail_out = m_uiWriteSize;
		}
	}

	// Flush any remaining write data
	if(ret==Z_STREAM_END && hr==S_OK && m_bWriteOPFile && m_uiWriteSize - zcpr.avail_out != 0)
		hr = WriteDecompressedFile(m_uiWriteSize - zcpr.avail_out);
	
	inflateEnd(&zcpr);

	if(ret==Z_STREAM_END && hr==S_OK)
		return S_OK;
	else
		return E_FAIL;

}

HRESULT ZLIBDecode::DecodeStreamToMemory()
{
	z_stream zcpr;
	int ret=Z_OK;
	HRESULT hr = S_OK;
	memset(&zcpr,0,sizeof(z_stream));
	inflateInit(&zcpr);

	zcpr.avail_out = m_uiWriteSize;
	zcpr.next_out = m_pDecOut;

	while (ret==Z_OK && hr==S_OK)
	{
#ifdef _XBOX

		const DWORD dwPrefetchDistance = 1024;
		const DWORD dwPPCCacheLine = 128;

		BYTE * pIPBuff = m_pDecInp;
		int nChunkSize = (int)(m_dwReadCount < m_uiReadSize ? m_dwReadCount : m_uiReadSize);

		for( DWORD i = 0; i < dwPrefetchDistance; i += dwPPCCacheLine )
			__dcbt( i, pIPBuff );

		while( ret == S_OK && nChunkSize > 0)
		{
			__dcbt( dwPrefetchDistance , pIPBuff );

			zcpr.avail_in = dwPPCCacheLine;
			zcpr.next_in = pIPBuff;
			ret=inflate(&zcpr,Z_SYNC_FLUSH);
			pIPBuff += dwPPCCacheLine;
			nChunkSize -= (int)dwPPCCacheLine;
		}
#else
		zcpr.next_in = m_pDecInp;
		zcpr.avail_in = m_dwReadCount < m_uiReadSize ? m_dwReadCount : m_uiReadSize;

		ret=inflate(&zcpr,Z_SYNC_FLUSH);
#endif
		if(zcpr.avail_in == 0 && ret==Z_OK)
		{
			hr = ReadCompressedFile();
		}
	}


	inflateEnd(&zcpr);

	if(ret==Z_STREAM_END && hr==S_OK)
		return S_OK;
	else
		return E_FAIL;

}