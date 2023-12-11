#if !defined( _FDXFSHELPER_H )
#define _FDXFSHELPER_H 

#pragma once
#if defined( _MSC_VER )
#include <crtdbg.h>
#endif
#include <common.h>
#include <types.h>
#include <string>

#include "FDXFSInfoHandle.h"

enum FSXFSINFO_MODES
{
	FSXFSINFO_MODE_FILE_ENTRIES,
	FSXFSINFO_MODE_FILE_ENTRY_BY_IDENTIFIER,
	FSXFSINFO_MODE_FILE_ENTRY_BY_PATH,
	FSXFSINFO_MODE_FILE_SYSTEM_HIERARCHY,
	FSXFSINFO_MODE_VOLUME
};

class FDXFSHelper
{
public:
	FDXFSHelper();
	~FDXFSHelper();

private:
	libcerror_error_t* m_pError;

protected:
	const char* m_program;
	std::wstring m_source;
	FDXFSInfoHandle* m_pXfsInfoHandle;

public:
	BOOL Initialize();
	BOOL Open(const TCHAR* source);
	int DirTest();

	void Close();
};

#endif /* !defined( _FDXFSHELPER_H ) */
