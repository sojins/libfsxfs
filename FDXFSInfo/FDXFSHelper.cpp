#include "FDXFSHelper.h"

#include <system_string.h>
#include <libfsxfs.h>
#include <sstream>

FDXFSHelper* FDXFSHelper::m_pInstance = NULL;

FDXFSHelper::FDXFSHelper()
{
	m_program = "FDXFSHelper";
	m_source = L"";

	m_pError = NULL;
	m_pXfsInfoHandle = NULL;
}

FDXFSHelper::~FDXFSHelper()
{
	if (m_pXfsInfoHandle != NULL)
		delete m_pXfsInfoHandle;

	for (XFSBlockDevInfo::iterator it = m_block_dev.begin(); it != m_block_dev.end(); it++) {
		xfs_fs* b = it->second;
		if (b) {
			//FD_EXT4::umount(it->first.c_str());
		}
	}
	m_block_dev.clear();
}

BOOL FDXFSHelper::Mount(CVirtualDrive* pVdrive, const char* mount_point)
{
	const char* mp = m_mountpoint.c_str();
	if (mount_point)
		mp = mount_point;

	std::stringstream ss;
	ss << mount_point;
	if (m_block_dev.size() > 0) {
		XFSBlockDevInfo::iterator it = m_block_dev.find(mount_point);
		if (it != m_block_dev.end()) {
			ss << "#";
			ss << m_block_dev.size();

			//char* pVolumeLabel = pVdrive->partitionInfo.volume_label;
			//sprintf(pVolumeLabel, ss.str().c_str());
		}
	}
	m_mountpoint = "/" + ss.str() + "/";

	//struct ext4_blockdev* bdev = FD_EXT4::open_blockdev(ss.str().c_str(), pVdrive);
	//if (FD_EXT4::mount(bdev, &m_bc, m_mountpoint.c_str())) {
	//	xfs_fs* fs = new xfs_fs;
	//	ext4_get_fs(m_mountpoint.c_str(), fs);
	//	m_block_dev.insert(std::make_pair(ss.str(), fs));
	//	return TRUE;
	//}
	return FALSE;
}

BOOL FDXFSHelper::IsMounted()
{
	return m_block_dev.size() > 0;
}

BOOL FDXFSHelper::Initialize()
{
	size_t string_length = 0;
	m_pXfsInfoHandle = new FDXFSInfoHandle();

	libfsxfs_notify_set_stream(stderr, NULL);
	libfsxfs_notify_set_verbose(0);
	try {
		if (m_pXfsInfoHandle->Initialize() != 1)
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "Unable to initialize info handle.\n");

		// for Debug
		info_handle_t* pInfoHandle = m_pXfsInfoHandle->GetInfoHandle();
		if (pInfoHandle == NULL)
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "invalid info handle.\n");

		return TRUE;
	}
	catch (FDXFSException& e) {
		if (m_pError != NULL)
		{
			//libcnotify_print_error_backtrace(
			//	m_pError);
			//libcerror_error_free(
			//	&m_pError);
		}
		if (m_pXfsInfoHandle != NULL)
		{
			m_pXfsInfoHandle->Finalize();
		}
	}
	return FALSE;
}

BOOL FDXFSHelper::Open(const TCHAR* source)
{
	if (NULL == source) return FALSE;
	if (!m_source.empty()) {
		return TRUE;
	}

	if (m_pXfsInfoHandle->Open(source) != 1) {
		fprintf(
			stderr,
			"Unable to open: %" PRIs_SYSTEM ".\n",
			m_source.c_str());

		return FALSE;
	}

	if (m_source.empty()) {
		m_source = std::wstring(source);
	}

	return TRUE;
}

void FDXFSHelper::Close()
{
	if (m_pXfsInfoHandle && !m_source.empty()) {
		m_pXfsInfoHandle->Close();
	}
}

int FDXFSHelper::TestVolume()
{
	info_handle_t* info_handle = m_pXfsInfoHandle->GetInfoHandle();
	if (info_handle == NULL)
		throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle.");
	m_pXfsInfoHandle->Volume_fprint();
	return 1;
}

int FDXFSHelper::DirTest()
{
	libfsxfs_file_entry_t* file_entry = NULL;
	static const char* function = "FDXFSHelper::Dir()";
	int result = 0;

	info_handle_t* info_handle = m_pXfsInfoHandle->GetInfoHandle();
	libfsxfs_error_t** error = &m_pError;

	try {
		if (info_handle == NULL)
			throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle.");

		if (info_handle->bodyfile_stream == NULL)
		{
			fprintf(info_handle->notify_stream, "X File System information:\n\n");
			fprintf(info_handle->notify_stream, "File system hierarchy:\n");
		}

		file_entry = m_pXfsInfoHandle->FindFirst();

		if (result == -1 || file_entry == NULL)
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "unable to retrieve root directory file entry.");


		if (m_pXfsInfoHandle->FindNext(file_entry, _SYSTEM_STRING("/"), 1) != 1)
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_PRINT_FAILED, "unable to print root directory file entry information.");

		if (libfsxfs_file_entry_free(&file_entry, error) != 1)
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED, "unable to free file entry.");

		if (info_handle->bodyfile_stream == NULL)
			fprintf(info_handle->notify_stream, "\n");
		return(1);
	}
	catch (FDXFSException& e) {
		LIBCERROR_ERROR_DOMAINS err_domain = LIBCERROR_ERROR_DOMAIN_ARGUMENTS;
		if (typeid(e) == typeid(FDRuntimeException)) {
			err_domain = LIBCERROR_ERROR_DOMAIN_RUNTIME;
		}
		else if (typeid(e) == typeid(FDMemoryException)) {
			err_domain = LIBCERROR_ERROR_DOMAIN_MEMORY;
		}
		else if (typeid(e) == typeid(FDConversionException)) {
			err_domain = LIBCERROR_ERROR_DOMAIN_CONVERSION;
		}

		libcerror_error_set(
			error,
			err_domain,
			e.code(),
			"%s: %s",
			function, e.what());
	}

	if (file_entry != NULL)
	{
		libfsxfs_file_entry_free(
			&file_entry,
			NULL);
	}
	return(-1);
}

FDXFSHelper* GetXFSHelper()
{
	return FDXFSHelper::getInstance();
}
