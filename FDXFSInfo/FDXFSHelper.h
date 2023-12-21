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
#include <unordered_map>
#include <stack>

class CVirtualDrive;


enum FSXFSINFO_MODES
{
	FSXFSINFO_MODE_FILE_ENTRIES,
	FSXFSINFO_MODE_FILE_ENTRY_BY_IDENTIFIER,
	FSXFSINFO_MODE_FILE_ENTRY_BY_PATH,
	FSXFSINFO_MODE_FILE_SYSTEM_HIERARCHY,
	FSXFSINFO_MODE_VOLUME
};



#define xfs_fs libfsxfs_file_system_t
class FDXFSHelper
{
private:
	FDXFSHelper();
	~FDXFSHelper();
	FDXFSHelper(const FDXFSHelper& inst) {}

public:
	static FDXFSHelper* getInstance(int nRunType = 0)
	{
		if (m_pInstance == nullptr)
		{
			m_pInstance = new FDXFSHelper;
			atexit(destroy);
		}

		return m_pInstance;
	}

	static void destroy()
	{
		if (m_pInstance != nullptr)
		{
			delete m_pInstance;
			m_pInstance = nullptr;
		}
	}

private:
	typedef std::unordered_map<std::string, struct xfs_fs*>	XFSBlockDevInfo;
	XFSBlockDevInfo m_block_dev;
	static FDXFSHelper* m_pInstance;
	friend FDXFSHelper* GetXFSHelper();

	libcerror_error_t* m_pError;

protected:
	const char* m_program;
	std::wstring m_source;
	std::string m_mountpoint;
	std::stack<uint64_t> m_sDeletedInode;

	FDXFSInfoHandle* m_pXfsInfoHandle;

public:
	BOOL Mount(CVirtualDrive* pVdrive, const char* mount_point);
	BOOL IsMounted();

	BOOL Initialize();
	BOOL Open(const TCHAR* source);
	int DirTest();
	int TestVolume();
	void Close();
};
FDXFSHelper* GetXFSHelper();
#endif /* !defined( _FDXFSHELPER_H ) */
