#include "FDXFSHelper.h"

#include <system_string.h>
#include <libfsxfs.h>

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
}

BOOL FDXFSHelper::Initialize()
{
	size_t string_length = 0;
	m_pXfsInfoHandle = new FDXFSInfoHandle();

	libfsxfs_notify_set_stream(stderr,NULL);
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
		LIBCERROR_ERROR_DOMAINS err = LIBCERROR_ERROR_DOMAIN_ARGUMENTS;
		if (typeid(e) == typeid(FDRuntimeException)) {
			err = LIBCERROR_ERROR_DOMAIN_RUNTIME;
		}
		else if (typeid(e) == typeid(FDMemoryException)) {
			err = LIBCERROR_ERROR_DOMAIN_MEMORY;
		}
		else if (typeid(e) == typeid(FDConversionException)) {
			err = LIBCERROR_ERROR_DOMAIN_CONVERSION;
		}

		libcerror_error_set(
			error,
			err,
			e.code(),
			"%s: %s",
			function, e.what());
	}
	/*catch (FDArgumentException& e) {
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			e.code(),
			"%s: %s",
			function, e.what());
	}
	catch (FDRuntimeException& e) {
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			e.code(),
			"%s: %s",
			function, e.what());
	}*/

	if (file_entry != NULL)
	{
		libfsxfs_file_entry_free(
			&file_entry,
			NULL);
	}
	return(-1);
}
