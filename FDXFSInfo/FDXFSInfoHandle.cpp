/*
 * Info handle
 *
 * Copyright (C) 2020-2023, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "FDXFSInfoHandle.h"

#include <memory.h>
#include <system_string.h>

#include <libfsxfs.h>
#include <libcerror_error.h>

#include <libhmac_definitions.h>
#include <libuna_utf8_string.h>
#include <libuna_unicode_character.h>
#include "../libfsxfs/libfsxfs_volume.h"

#include <sstream>
#include "FDXFSFileDev.h"

#define DIGEST_HASH_STRING_SIZE_MD5	33
#define INFO_HANDLE_NOTIFY_STREAM	stdout
void verifyHandle(xfsinfo_handle_t* info_handle, const system_character_t* path);
void verifyHandle(xfsinfo_handle_t* info_handle);

FDXFSInfoHandle::FDXFSInfoHandle()
{
	m_pError = NULL;
	m_pInfo = NULL;
}

FDXFSInfoHandle::~FDXFSInfoHandle()
{
}

#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
#define copyToUtf16 libuna_unicode_character_copy_to_utf16
#define copyFromUtf16 libuna_unicode_character_copy_from_utf16
#define XFSGetName libfsxfs_file_entry_get_utf16_name
#define XFSGetNameLength libfsxfs_file_entry_get_utf16_name_size
#define copyPosixTimeToUtf16 libfdatetime_posix_time_copy_to_utf16_string
#define getSymbolicLinkTarget libfsxfs_file_entry_get_utf16_symbolic_link_target
#define getSymbolicLinkTargetSize libfsxfs_file_entry_get_utf16_symbolic_link_target_size
#define getAttributeNameSize libfsxfs_extended_attribute_get_utf16_name_size
#define getAttributeName libfsxfs_extended_attribute_get_utf16_name
//#define getFileEntryPath libfsxfs_volume_get_file_entry_by_utf16_path
#define getLabelSize libfsxfs_volume_get_utf16_label_size
#define getLabel libfsxfs_volume_get_utf16_label
#else
#define copyToUtf16 libuna_unicode_character_copy_to_utf8
#define copyFromUtf16 libuna_unicode_character_copy_from_utf8
#define XFSGetName libfsxfs_file_entry_get_utf8_name
#define XFSGetNameLength libfsxfs_file_entry_get_utf8_name_size
#define copyPosixTimeToUtf16 libfdatetime_posix_time_copy_to_utf8_string
#define getSymbolicLinkTarget libfsxfs_file_entry_get_utf8_symbolic_link_target
#define getSymbolicLinkTargetSize libfsxfs_file_entry_get_utf8_symbolic_link_target_size
#define getAttributeNameSize libfsxfs_extended_attribute_get_utf8_name_size
#define getAttributeName libfsxfs_extended_attribute_get_utf8_name
//#define getFileEntryPath libfsxfs_volume_get_file_entry_by_utf8_path
#define getLabelSize libfsxfs_volume_get_utf8_label_size
#define getLabel libfsxfs_volume_get_utf8_label
#endif

/* Creates an info handle
 * Make sure the value info_handle is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int FDXFSInfoHandle::Initialize()
{
	static const char* function = "info_handle_initialize";

	xfsinfo_handle_t** info_handle = &m_pInfo;
	uint8_t calculate_md5 = 0;
	libcerror_error_t** error = &m_pError;

	try {
		if (info_handle == NULL)
			throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle.");

		if (*info_handle != NULL)
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET, "invalid info handle value already set.");

		*info_handle = memory_allocate_structure(xfsinfo_handle_t);

		if (*info_handle == NULL)
			throw FDMemoryException(LIBCERROR_MEMORY_ERROR_INSUFFICIENT, "unable to create info handle.");

		if (memory_set(*info_handle, 0, sizeof(xfsinfo_handle_t)) == NULL)
			throw FDMemoryException(LIBCERROR_MEMORY_ERROR_SET_FAILED, "unable to clear info handle.");

		//xfs_blockdev* bd = FD_XFS::open_blockdev("xfs_mp", NULL);
		//if (bd) {
		//	(*info_handle)->input_file_io_handle = bd->handle;
		//}


		(*info_handle)->calculate_md5 = calculate_md5;
		(*info_handle)->notify_stream = INFO_HANDLE_NOTIFY_STREAM;

		return(1);
	}
	catch (FDXFSException& e)
	{
		LIBCERROR_ERROR_DOMAINS err_domain = LIBCERROR_ERROR_DOMAIN_ARGUMENTS;
		if (typeid(e) == typeid(FDRuntimeException)) {
			err_domain = LIBCERROR_ERROR_DOMAIN_RUNTIME;
		}
		else if (typeid(e) == typeid(FDMemoryException)) {
			err_domain = LIBCERROR_ERROR_DOMAIN_MEMORY;
		}

		libcerror_error_set(
			error,
			err_domain,
			e.code(),
			"%s: %s",
			function, e.what());

		if (*info_handle != NULL)
		{
			if ((*info_handle)->input_file_io_handle != NULL)
			{
				//libbfio_handle_free(
				//	&((*info_handle)->input_file_io_handle),
				//	NULL);
			}
			SAFE_FREE(*info_handle);
		}
		return(-1);
	}
}

/* Frees an info handle
 * Returns 1 if successful or -1 on error
 */
int FDXFSInfoHandle::Finalize()
{
	static const char* function = "info_handle_free";
	int result = 1;
	xfsinfo_handle_t** info_handle = &m_pInfo;
	libcerror_error_t** error = &m_pError;
	try {
		if (info_handle == NULL)
			throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle.");

		if (*info_handle != NULL)
		{
			if ((*info_handle)->bodyfile_stream != NULL)
			{
				if (file_stream_close((*info_handle)->bodyfile_stream) != 0)
					throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED, "unable to close bodyfile stream.");

				(*info_handle)->bodyfile_stream = NULL;
			}

			if ((*info_handle)->input_volume != NULL)
			{
				if (libfsxfs_volume_free(&((*info_handle)->input_volume), error) != 1)
					throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED, "unable to free input volume.");
			}

			//if (libbfio_handle_free(&((*info_handle)->input_file_io_handle), error) != 1)
			//	throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED, "unable to free input file IO handle.");

			SAFE_FREE(info_handle);
		}
		return(result);
	}
	catch (FDRuntimeException& e) {
		result = -1;
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			e.code(),
			"%s: %s",
			function, e.what());

		SAFE_FREE(info_handle);
	}
	catch (FDArgumentException& e) {
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			e.code(),
			"%s: %s",
			function, e.what());
	}

	return result;
}

/* Signals the info handle to abort
 * Returns 1 if successful or -1 on error
 */
//int FDXFSInfoHandle::SignalAbort()
//{
//	static const char* function = "info_handle_signal_abort";
//	info_handle_t* info_handle = m_pInfoHandle;
//	libcerror_error_t** error = &m_pError;
//	try {
//		if (info_handle == NULL)
//			throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle.");
//
//		info_handle->abort = 1;
//
//		if (info_handle->input_volume != NULL)
//		{
//			if (libfsxfs_volume_signal_abort(info_handle->input_volume, error) != 1)
//				throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_SET_FAILED, "unable to signal input volume to abort.");
//		}
//		return 1;
//	}
//	catch (FDArgumentException& e) {
//		libcerror_error_set(
//			error,
//			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
//			e.code(),
//			"%s: %s",
//			function, e.what());
//	}
//	catch (FDRuntimeException& e) {
//		libcerror_error_set(
//			error,
//			LIBCERROR_ERROR_DOMAIN_RUNTIME,
//			e.code(),
//			"%s: %s",
//			function, e.what());
//	}
//
//	return(-1);
//}

/* Sets the bodyfile
 * Returns 1 if successful or -1 on error
 */
//int FDXFSInfoHandle::SetBodyFile(
//	const system_character_t* filename)
//{
//	static const char* function = "info_handle_set_bodyfile";
//	info_handle_t* info_handle = m_pInfoHandle;
//	libcerror_error_t** error = &m_pError;
//	try {
//		if (info_handle == NULL)
//			throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle.");
//
//		/*if (info_handle == NULL)
//		{
//			libcerror_error_set(
//				error,
//				LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
//				LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
//				"%s: invalid info handle.",
//				function);
//
//			return(-1);
//		}*/
//		if (info_handle->bodyfile_stream != NULL)
//		{
//			//libcerror_error_set(
//			//	error,
//			//	LIBCERROR_ERROR_DOMAIN_RUNTIME,
//			//	LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
//			//	"%s: invalid info handle - bodyfile stream value already set.",
//			//	function);
//
//			//return(-1);
//		}
//		if (filename == NULL)
//		{
//			libcerror_error_set(
//				error,
//				LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
//				LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
//				"%s: invalid filename.",
//				function);
//
//			return(-1);
//		}
//#if defined( HAVE_WIDE_SYSTEM_CHARACTER )
//		info_handle->bodyfile_stream = file_stream_open_wide(
//			filename,
//			L"wb");
//#else
//		info_handle->bodyfile_stream = file_stream_open(
//			filename,
//			"wb");
//#endif
//		if (info_handle->bodyfile_stream == NULL)
//		{
//			libcerror_error_set(
//				error,
//				LIBCERROR_ERROR_DOMAIN_IO,
//				LIBCERROR_IO_ERROR_OPEN_FAILED,
//				"%s: unable to open bodyfile stream.",
//				function);
//
//			return(-1);
//		}
//		return(1);
//	}
//	catch (FDArgumentException& e) {
//		libcerror_error_set(
//			error,
//			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
//			e.code(),
//			"%s: %s",
//			function, e.what());
//	}
//	return -1;
//}

/* Sets the volume offset
 * Returns 1 if successful or -1 on error
 */
//int FDXFSInfoHandle::SetVolumeOffset(const system_character_t* string)
//{
//	static const char* function = "info_handle_set_volume_offset";
//	size_t string_length = 0;
//	uint64_t value_64bit = 0;
//	info_handle_t* info_handle = m_pInfoHandle;
//	libcerror_error_t** error = &m_pError;
//
//	if (info_handle == NULL)
//		throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle.");
//
//	string_length = system_string_length(
//		string);
//
//	if (CopyFrom64bitInDecimal(string, string_length + 1, &value_64bit) != 1)
//		throw FDArgumentException(LIBCERROR_RUNTIME_ERROR_COPY_FAILED, "unable to copy string to 64-bit decimal.");
//
//	info_handle->volume_offset = (off64_t)value_64bit;
//
//	return(1);
//}

/* Opens the input
 * Returns 1 if successful or -1 on error
 */
void FDXFSInfoHandle::Mount(const system_character_t* filename)
{
	xfsinfo_handle_t* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;

	size_t filename_length = system_string_length(filename);

	if (FD_XFS::file_dev_set_name_wide(
		info_handle->input_file_io_handle,
		filename,
		filename_length,
		error) != 1)
	{
		throw FDArgumentException(LIBCERROR_RUNTIME_ERROR_SET_FAILED, "unable to set file name.");
	}

	if (FD_XFS::file_dev_set(
		info_handle->input_file_io_handle,
		info_handle->volume_offset,
		0,
		error) != 1)
	{
		throw FDArgumentException(LIBCERROR_RUNTIME_ERROR_SET_FAILED, "unable to set range.");
	}
}

/* Opens the input
 * Returns 1 if successful or -1 on error
 */
int FDXFSInfoHandle::Open(const system_character_t* filename)
{
	static const char* function = "info_handle_open_input";

	xfsinfo_handle_t* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;

	try {
		if (info_handle == NULL)
			throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle.");

		Mount(filename);
		
		if (libfsxfs_volume_initialize(&(info_handle->input_volume), error) != 1)
			throw FDArgumentException(LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED, "unable to initialize input volume.");

		if (libfsxfs_volume_open_file_io_handle(
			info_handle->input_volume,
			(libbfio_handle_t*)info_handle->input_file_io_handle,
			LIBFSXFS_OPEN_READ,
			error) != 1)
			throw FDIOException(LIBCERROR_IO_ERROR_OPEN_FAILED, "unable to open input volume.");

		return(1);
	}
	catch (FDXFSException& e) {
		LIBCERROR_ERROR_DOMAINS err_domain = LIBCERROR_ERROR_DOMAIN_ARGUMENTS;
		if (typeid(e) == typeid(FDIOException)) {
			err_domain = LIBCERROR_ERROR_DOMAIN_IO;
		}
		libcerror_error_set(
			error,
			err_domain,
			e.code(),
			"%s: %s",
			function, e.what());
		
		if (info_handle && info_handle->input_volume != NULL)
		{
			libfsxfs_volume_free(
				&(info_handle->input_volume),
				NULL);
		}
	}

	return(-1);
}

/* Closes the input
 * Returns the 0 if succesful or -1 on error
 */
int FDXFSInfoHandle::Close()
{
	static const char* function = "FDXFSInfoHandle::Close()";
	xfsinfo_handle_t* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;
	try {
		if (info_handle == NULL)
			throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle.");

		if (info_handle->input_volume != NULL)
		{
			if (libfsxfs_volume_close(info_handle->input_volume, error) != 0)
				throw FDIOException(LIBCERROR_IO_ERROR_CLOSE_FAILED, "invalid info handle.");
		}
		return 0;
	}
	catch (FDArgumentException& e) {
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			e.code(),
			"%s: %s",
			function, e.what());
	} 
	catch (FDIOException& e) {
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_IO,
			e.code(),
			"%s: %s",
			function, e.what());
	}
	return(-1);
}

///* Calculates the MD5 of the contents of a file entry
// * Returns 1 if successful or -1 on error
// */
//int FDXFSInfoHandle::info_handle_file_entry_calculate_md5(
//     info_handle_t *info_handle,
//     libfsxfs_file_entry_t *file_entry,
//     char *md5_string,
//     size_t md5_string_size,
//     libcerror_error_t **error )
//{
//	uint8_t md5_hash[ LIBHMAC_MD5_HASH_SIZE ];
//	uint8_t read_buffer[ 4096 ];
//
//	libhmac_md5_context_t *md5_context = NULL;
//	static const char *function              = "info_handle_file_entry_calculate_md5";
//	size64_t data_size                 = 0;
//	size_t read_size                   = 0;
//	ssize_t read_count                 = 0;
//
//	if( info_handle == NULL )
//	{
//		libcerror_error_set(
//		 error,
//		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
//		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
//		 "%s: invalid info handle.",
//		 function );
//
//		return( -1 );
//	}
//	if( libfsxfs_file_entry_get_size(
//	     file_entry,
//	     &data_size,
//	     error ) != 1 )
//	{
//		libcerror_error_set(
//		 error,
//		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
//		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
//		 "%s: unable to retrieve size.",
//		 function );
//
//		goto on_error;
//	}
//	if( libfsxfs_file_entry_seek_offset(
//	     file_entry,
//	     0,
//	     SEEK_SET,
//	     error ) == -1 )
//	{
//		libcerror_error_set(
//		 error,
//		 LIBCERROR_ERROR_DOMAIN_IO,
//		 LIBCERROR_IO_ERROR_SEEK_FAILED,
//		 "%s: unable to seek offset: 0 in file entry.",
//		 function );
//
//		goto on_error;
//	}
//	if( libhmac_md5_initialize(
//	     &md5_context,
//	     error ) != 1 )
//	{
//		libcerror_error_set(
//		 error,
//		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
//		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
//		 "%s: unable to initialize MD5 context.",
//		 function );
//
//		goto on_error;
//	}
//	while( data_size > 0 )
//	{
//		read_size = 4096;
//
//		if( (size64_t) read_size > data_size )
//		{
//			read_size = (size_t) data_size;
//		}
//		read_count = libfsxfs_file_entry_read_buffer(
//		              file_entry,
//		              read_buffer,
//		              read_size,
//		              error );
//
//		if( read_count != (ssize_t) read_size )
//		{
//			libcerror_error_set(
//			 error,
//			 LIBCERROR_ERROR_DOMAIN_IO,
//			 LIBCERROR_IO_ERROR_READ_FAILED,
//			 "%s: unable to read from file entry.",
//			 function );
//
//			goto on_error;
//		}
//		data_size -= read_size;
//
//		if( libhmac_md5_update(
//		     md5_context,
//		     read_buffer,
//		     read_size,
//		     error ) != 1 )
//		{
//			libcerror_error_set(
//			 error,
//			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
//			 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
//			 "%s: unable to update MD5 hash.",
//			 function );
//
//			goto on_error;
//		}
//	}
//	if( libhmac_md5_finalize(
//	     md5_context,
//	     md5_hash,
//	     LIBHMAC_MD5_HASH_SIZE,
//	     error ) != 1 )
//	{
//		libcerror_error_set(
//		 error,
//		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
//		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
//		 "%s: unable to finalize MD5 hash.",
//		 function );
//
//		goto on_error;
//	}
//	if( libhmac_md5_free(
//	     &md5_context,
//	     error ) != 1 )
//	{
//		libcerror_error_set(
//		 error,
//		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
//		 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
//		 "%s: unable to free MD5 context.",
//		 function );
//
//		goto on_error;
//	}
//	if( digest_hash_copy_to_string(
//	     md5_hash,
//	     LIBHMAC_MD5_HASH_SIZE,
//	     md5_string,
//	     md5_string_size,
//	     error ) != 1 )
//	{
//		libcerror_error_set(
//		 error,
//		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
//		 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
//		 "%s: unable to set MD5 hash string.",
//		 function );
//
//		goto on_error;
//	}
//	return( 1 );
//
//on_error:
//	if( md5_context != NULL )
//	{
//		libhmac_md5_free(
//		 &md5_context,
//		 NULL );
//	}
//	return( -1 );
//}


/* Prints a file entry or data stream name
 * Returns 1 if successful or -1 on error
 */
int FDXFSInfoHandle::PrintNameValue(
	const system_character_t* value_string,
	size_t value_string_length)
{
	system_character_t* escaped_value_string = NULL;
	static const char* function = "info_handle_name_value_fprint";
	libuna_unicode_character_t unicode_character = 0;
	size_t escaped_value_string_index = 0;
	size_t escaped_value_string_size = 0;
	size_t value_string_index = 0;
	int print_count = 0;
	int result = 0;
	xfsinfo_handle_t* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;
	try {
		verifyHandle(info_handle, value_string);

		/* To ensure normalization in the escaped string is handled correctly
		 * it stored in a temporary variable. Note that there is a worst-case of
		 * a 1 to 4 ratio for each escaped character.
		 */
		if (value_string_length > (size_t)((SSIZE_MAX - 1) / (sizeof(system_character_t) * 4)))
			throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
				"invalid value string length value exceeds maximum.");

		escaped_value_string_size = (value_string_length * 4) + 1;

		escaped_value_string = (system_character_t*)memory_allocate(
			sizeof(system_character_t) * escaped_value_string_size);

		if (escaped_value_string == NULL)
			throw FDMemoryException(LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
				"unable to create escaped value string.");

		while (value_string_index < value_string_length)
		{
			result = copyFromUtf16(&unicode_character, (libuna_utf16_character_t*)value_string, value_string_length, &value_string_index, error);
			
			if (result != 1)
			{
				throw FDConversionException(LIBCERROR_CONVERSION_ERROR_INPUT_FAILED,
					"%s: unable to copy Unicode character from value string.");
			}
			/* Replace:
			 *   Control characters ([U+0-U+1f, U+7f-U+9f]) by \x##
			 */
			if ((unicode_character <= 0x1f)
				|| ((unicode_character >= 0x7f)
					&& (unicode_character <= 0x9f)))
			{
				print_count = system_string_sprintf(
					&(escaped_value_string[escaped_value_string_index]),
					escaped_value_string_size - escaped_value_string_index,
					L"\\x%02" PRIx32 "",
					unicode_character);

				if (print_count < 0)
					throw FDConversionException(LIBCERROR_CONVERSION_ERROR_INPUT_FAILED,
						"unable to copy escaped Unicode character to escaped value string.");

				escaped_value_string_index += print_count;
			}
			else
			{
				result = copyToUtf16(unicode_character, (libuna_utf16_character_t*)escaped_value_string, escaped_value_string_size, &escaped_value_string_index, error);

				if (result != 1)
					throw FDConversionException(LIBCERROR_CONVERSION_ERROR_INPUT_FAILED, "unable to copy Unicode character to escaped value string.");
			}
		}
		escaped_value_string[escaped_value_string_index] = 0;

		if (info_handle->bodyfile_stream != NULL)
		{
			fprintf(
				info_handle->bodyfile_stream,
				"%" PRIs_SYSTEM "",
				escaped_value_string);
		}
		else
		{
			fprintf(
				info_handle->notify_stream,
				"%" PRIs_SYSTEM "",
				escaped_value_string);
		}
		memory_free(
			escaped_value_string);

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

	if (escaped_value_string != NULL)
	{
		memory_free(
			escaped_value_string);
	}
	return(-1);
}

/* Prints a nano seconds POSIX time value
 * Returns 1 if successful or -1 on error
 */
int FDXFSInfoHandle::PosixTimeInNanoSecondsValue_fprint(
	const char* value_name,
	int64_t value_64bit)
{
	system_character_t date_time_string[32];

	libfdatetime_posix_time_t* posix_time = NULL;
	static const char* function = "info_handle_posix_time_in_nano_seconds_value_fprint";
	int result = 0;

	xfsinfo_handle_t* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;

	if (info_handle == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid info handle.",
			function);

		return(-1);
	}
	if (value_64bit == 0)
	{
		fprintf(
			info_handle->notify_stream,
			"%s: Not set (0)\n",
			value_name);
	}
	else
	{
		if (libfdatetime_posix_time_initialize(
			&posix_time,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				"%s: unable to create POSIX time.",
				function);

			goto on_error;
		}
		if (libfdatetime_posix_time_copy_from_64bit(
			posix_time,
			(uint64_t)value_64bit,
			LIBFDATETIME_POSIX_TIME_VALUE_TYPE_NANO_SECONDS_64BIT_SIGNED,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				"%s: unable to copy POSIX time from 64-bit.",
				function);

			goto on_error;
		}

		result = copyPosixTimeToUtf16(
			posix_time,
			(uint16_t*)date_time_string,
			32,
			LIBFDATETIME_STRING_FORMAT_TYPE_ISO8601 | LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME_NANO_SECONDS,
			error);
		if (result != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				"%s: unable to copy POSIX time to string.",
				function);

			goto on_error;
		}
		fprintf(
			info_handle->notify_stream,
			"%s: %" PRIs_SYSTEM "Z\n",
			value_name,
			date_time_string);

		if (libfdatetime_posix_time_free(
			&posix_time,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				"%s: unable to free POSIX time.",
				function);

			goto on_error;
		}
	}
	return(1);

on_error:
	if (posix_time != NULL)
	{
		libfdatetime_posix_time_free(
			&posix_time,
			NULL);
	}
	return(-1);
}

/* Prints a file entry value with name
 * Returns 1 if successful, 0 if not or -1 on error
 */
int FDXFSInfoHandle::FileEntryValueWithName_fprint(
	libfsxfs_file_entry_t* file_entry,
	const system_character_t* path,
	size_t path_length,
	const system_character_t* file_entry_name,
	size_t file_entry_name_length)
{
	char md5_string[DIGEST_HASH_STRING_SIZE_MD5] = {
		'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
		'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
		0 };

	char file_mode_string[11] = { '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', 0 };

	libfsxfs_extended_attribute_t* extended_attribute = NULL;
	system_character_t* extended_attribute_name = NULL;
	system_character_t* symbolic_link_target = NULL;
	static const char* function = "info_handle_file_entry_value_with_name_fprint";
	size64_t size = 0;
	size_t extended_attribute_name_size = 0;
	size_t symbolic_link_target_size = 0;
	uint64_t file_entry_identifier = 0;
	int64_t access_time = 0;
	int64_t creation_time = 0;
	int64_t inode_change_time = 0;
	int64_t modification_time = 0;
	uint32_t group_identifier = 0;
	uint32_t major_device_number = 0;
	uint32_t minor_device_number = 0;
	uint32_t number_of_links = 0;
	uint32_t owner_identifier = 0;
	uint16_t file_mode = 0;
	int extended_attribute_index = 0;
	int has_creation_time = 0;
	int number_of_extended_attributes = 0;
	int result = 0;

	xfsinfo_handle_t* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;

	if (info_handle == NULL)
		throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle.");

	if (libfsxfs_file_entry_get_inode_number(
		file_entry,
		&file_entry_identifier,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve inode number.",
			function);

		goto on_error;
	}
	if (libfsxfs_file_entry_get_modification_time(
		file_entry,
		&modification_time,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve modification time.",
			function);

		goto on_error;
	}
	if (libfsxfs_file_entry_get_inode_change_time(
		file_entry,
		&inode_change_time,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve inode change time.",
			function);

		goto on_error;
	}
	if (libfsxfs_file_entry_get_access_time(
		file_entry,
		&access_time,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve access time.",
			function);

		goto on_error;
	}
	result = libfsxfs_file_entry_get_creation_time(
		file_entry,
		&creation_time,
		error);

	if (result == -1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve creation time.",
			function);

		goto on_error;
	}
	has_creation_time = result;

	if (libfsxfs_file_entry_get_owner_identifier(
		file_entry,
		&owner_identifier,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve owner identifier.",
			function);

		goto on_error;
	}
	if (libfsxfs_file_entry_get_group_identifier(
		file_entry,
		&group_identifier,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve group identifier.",
			function);

		goto on_error;
	}
	if (libfsxfs_file_entry_get_file_mode(
		file_entry,
		&file_mode,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve file mode.",
			function);

		goto on_error;
	}
	/* TODO move into function */
	if ((file_mode & 0x0001) != 0)
	{
		file_mode_string[9] = 'x';
	}
	if ((file_mode & 0x0002) != 0)
	{
		file_mode_string[8] = 'w';
	}
	if ((file_mode & 0x0004) != 0)
	{
		file_mode_string[7] = 'r';
	}
	if ((file_mode & 0x0008) != 0)
	{
		file_mode_string[6] = 'x';
	}
	if ((file_mode & 0x0010) != 0)
	{
		file_mode_string[5] = 'w';
	}
	if ((file_mode & 0x0020) != 0)
	{
		file_mode_string[4] = 'r';
	}
	if ((file_mode & 0x0040) != 0)
	{
		file_mode_string[3] = 'x';
	}
	if ((file_mode & 0x0080) != 0)
	{
		file_mode_string[2] = 'w';
	}
	if ((file_mode & 0x0100) != 0)
	{
		file_mode_string[1] = 'r';
	}
	switch (file_mode & 0xf000)
	{
	case 0x1000:
		file_mode_string[0] = 'p';
		break;

	case 0x2000:
		file_mode_string[0] = 'c';
		break;

	case 0x4000:
		file_mode_string[0] = 'd';
		break;

	case 0x6000:
		file_mode_string[0] = 'b';
		break;

	case 0xa000:
		file_mode_string[0] = 'l';
		break;

	case 0xc000:
		file_mode_string[0] = 's';
		break;

	default:
		break;
	}
	result = getSymbolicLinkTargetSize(
		file_entry,
		&symbolic_link_target_size,
		error);

	if (result == -1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve symbolic link target string size.",
			function);

		goto on_error;
	}
	else if (result != 0)
	{
		symbolic_link_target = system_string_allocate(
			symbolic_link_target_size);

		if (symbolic_link_target == NULL)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_MEMORY,
				LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
				"%s: unable to create symbolic link target string.",
				function);

			goto on_error;
		}
		result = getSymbolicLinkTarget(
			file_entry,
			(uint16_t*)symbolic_link_target,
			symbolic_link_target_size,
			error);
		if (result != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to retrieve symbolic link target string.",
				function);

			goto on_error;
		}
	}
	if (libfsxfs_file_entry_get_size(
		file_entry,
		&size,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve size.",
			function);

		goto on_error;
	}
	if (info_handle->bodyfile_stream != NULL)
	{
		if( info_handle->calculate_md5 == 0 )
		{
			md5_string[ 1 ] = 0;
		}
		//else if( ( file_mode & 0xf000 ) == 0x8000 )
		//{
		//	if( info_handle_file_entry_calculate_md5(
		//	     info_handle,
		//	     file_entry,
		//	     md5_string,
		//	     DIGEST_HASH_STRING_SIZE_MD5,
		//	     error ) != 1 )
		//	{
		//		libcerror_error_set(
		//		 error,
		//		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		//		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		//		 "%s: unable to retreive MD5 string.",
		//		 function );

		//		goto on_error;
		//	}
		//}
		///* Colums in a Sleuthkit 3.x and later bodyfile
		// * MD5|name|inode|mode_as_string|UID|GID|size|atime|mtime|ctime|crtime
		// */
		//fprintf(
		// info_handle->bodyfile_stream,
		// "%s|",
		// md5_string );

		//if( path != NULL )
		//{
		//	if( info_handle_name_value_fprint(
		//	     info_handle,
		//	     path,
		//	     path_length,
		//	     error ) != 1 )
		//	{
		//		libcerror_error_set(
		//		 error,
		//		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		//		 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
		//		 "%s: unable to print path string.",
		//		 function );

		//		goto on_error;
		//	}
		//}
		//if( file_entry_name != NULL )
		//{
		//	if( info_handle_name_value_fprint(
		//	     info_handle,
		//	     file_entry_name,
		//	     file_entry_name_length,
		//	     error ) != 1 )
		//	{
		//		libcerror_error_set(
		//		 error,
		//		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		//		 LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
		//		 "%s: unable to print file entry name string.",
		//		 function );

		//		goto on_error;
		//	}
		//}
		//if( symbolic_link_target != NULL )
		//{
		//	fprintf(
		//	 info_handle->bodyfile_stream,
		//	 " -> %" PRIs_SYSTEM "",
		//	 symbolic_link_target );
		//}
		//fprintf(
		// info_handle->bodyfile_stream,
		// "|%" PRIu64 "|%s|%" PRIu32 "|%" PRIu32 "|%" PRIu64 "|%.9f|%.9f|%.9f|%.9f\n",
		// file_entry_identifier,
		// file_mode_string,
		// owner_identifier,
		// group_identifier,
		// size,
		// (double) access_time / 1000000000,
		// (double) modification_time / 1000000000,
		// (double) inode_change_time / 1000000000,
		// (double) creation_time / 1000000000 );
	}
	else
	{
		fprintf(
			info_handle->notify_stream,
			"\tInode number\t\t: %" PRIu64 "\n",
			file_entry_identifier);

		if (file_entry_name != NULL)
		{
			fprintf(
				info_handle->notify_stream,
				"\tName\t\t\t: ");

			if (path != NULL)
			{
				if (PrintNameValue(
					path,
					path_length) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
						"%s: unable to print path string.",
						function);

					goto on_error;
				}
			}
			if (file_entry_name != NULL)
			{
				if (PrintNameValue(
					file_entry_name,
					file_entry_name_length) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
						"%s: unable to print file entry name string.",
						function);

					goto on_error;
				}
			}
			fprintf(
				info_handle->notify_stream,
				"\n");
		}
		fprintf(
			info_handle->notify_stream,
			"\tSize\t\t\t: %" PRIu64 "\n",
			size);

		if (PosixTimeInNanoSecondsValue_fprint(
			"\tModification time\t",
			modification_time) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				"%s: unable to print POSIX time value.",
				function);

			goto on_error;
		}
		if (PosixTimeInNanoSecondsValue_fprint(
			"\tInode change time\t",
			inode_change_time) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				"%s: unable to print POSIX time value.",
				function);

			goto on_error;
		}
		if (PosixTimeInNanoSecondsValue_fprint(
			"\tAccess time\t\t",
			access_time) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				"%s: unable to print POSIX time value.",
				function);

			goto on_error;
		}
		if (has_creation_time != 0)
		{
			if (PosixTimeInNanoSecondsValue_fprint(
				"\tCreation time\t\t",
				creation_time) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
					"%s: unable to print POSIX time value.",
					function);

				goto on_error;
			}
		}
		if (libfsxfs_file_entry_get_number_of_links(
			file_entry,
			&number_of_links,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to retrieve number of links.",
				function);

			goto on_error;
		}
		fprintf(
			info_handle->notify_stream,
			"\tNumber of links\t\t: %" PRIu32 "\n",
			number_of_links);

		fprintf(
			info_handle->notify_stream,
			"\tOwner identifier\t: %" PRIu32 "\n",
			owner_identifier);

		fprintf(
			info_handle->notify_stream,
			"\tGroup identifier\t: %" PRIu32 "\n",
			group_identifier);

		fprintf(
			info_handle->notify_stream,
			"\tFile mode\t\t: %s (%07" PRIo16 ")\n",
			file_mode_string,
			file_mode);

		result = libfsxfs_file_entry_get_device_number(
			file_entry,
			&major_device_number,
			&minor_device_number,
			error);

		if (result == -1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to retrieve device number.",
				function);

			goto on_error;
		}
		else if (result != 0)
		{
			fprintf(
				info_handle->notify_stream,
				"\tDevice number\t\t: %" PRIu32 ",%" PRIu32 "\n",
				major_device_number,
				minor_device_number);
		}
		if (symbolic_link_target != NULL)
		{
			fprintf(
				info_handle->notify_stream,
				"\tSymbolic link target\t: %" PRIs_SYSTEM "\n",
				symbolic_link_target);
		}
		if (libfsxfs_file_entry_get_number_of_extended_attributes(
			file_entry,
			&number_of_extended_attributes,
			error) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_GET_FAILED,
				"%s: unable to retrieve number of extended attributes.",
				function);

			goto on_error;
		}
		if (number_of_extended_attributes > 0)
		{
			fprintf(
				info_handle->notify_stream,
				"\tExtended attributes:\n");

			for (extended_attribute_index = 0;
				extended_attribute_index < number_of_extended_attributes;
				extended_attribute_index++)
			{
				if (libfsxfs_file_entry_get_extended_attribute_by_index(
					file_entry,
					extended_attribute_index,
					&extended_attribute,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve extended attribute: %d.",
						function,
						extended_attribute_index);

					goto on_error;
				}
				result = getAttributeNameSize(
					extended_attribute,
					&extended_attribute_name_size,
					error);

				if (result == -1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve extended attribute name string size.",
						function);

					goto on_error;
				}
				fprintf(
					info_handle->notify_stream,
					"\t\tAttribute: %d\t: ",
					extended_attribute_index + 1);

				if ((result == 1)
					&& (extended_attribute_name_size > 0))
				{
					extended_attribute_name = system_string_allocate(
						extended_attribute_name_size);

					if (extended_attribute_name == NULL)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_MEMORY,
							LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
							"%s: unable to create extended attribute name string.",
							function);

						goto on_error;
					}
					result = getAttributeName(
						extended_attribute,
						(uint16_t*)extended_attribute_name,
						extended_attribute_name_size,
						error);
					if (result != 1)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_RUNTIME,
							LIBCERROR_RUNTIME_ERROR_GET_FAILED,
							"%s: unable to retrieve extended attribute name string.",
							function);

						goto on_error;
					}
					fprintf(
						info_handle->notify_stream,
						"%" PRIs_SYSTEM "",
						extended_attribute_name);

					memory_free(
						extended_attribute_name);

					extended_attribute_name = NULL;
				}
				fprintf(
					info_handle->notify_stream,
					"\n");

				if (libfsxfs_extended_attribute_free(
					&extended_attribute,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
						"%s: unable to free extended attribute: %d.",
						function,
						extended_attribute_index);

					goto on_error;
				}
			}
		}
	}
	if (symbolic_link_target != NULL)
	{
		memory_free(
			symbolic_link_target);

		symbolic_link_target = NULL;
	}
	return(1);

on_error:
	if (extended_attribute_name != NULL)
	{
		memory_free(
			extended_attribute_name);
	}
	if (extended_attribute != NULL)
	{
		libfsxfs_extended_attribute_free(
			&extended_attribute,
			NULL);
	}
	if (symbolic_link_target != NULL)
	{
		memory_free(
			symbolic_link_target);
	}
	return(-1);
}

void verifyHandle(xfsinfo_handle_t* info_handle)
{
	if (info_handle == NULL)
		throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle");
}

void verifyHandle(xfsinfo_handle_t* info_handle, const system_character_t* path)
{
	if (info_handle == NULL)
		throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid info handle");

	if (path == NULL)
		throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE, "invalid path");
}

void FDXFSInfoHandle::SAFE_FREE(void* pBuffer) {
	if (pBuffer) {
		memory_free(pBuffer); pBuffer = NULL;
	}
}
/* Prints file entry information as part of the file system hierarchy
 * Returns 1 if successful or -1 on error
 */
int FDXFSInfoHandle::FindNext(
	libfsxfs_file_entry_t* file_entry,
	const system_character_t* path,
	size_t path_length)
{
	libfsxfs_file_entry_t* sub_file_entry = NULL;
	system_character_t* file_entry_name = NULL;
	system_character_t* sub_path = NULL;
	
	static const char* function = "FDXFSInfoHandle::FindNext";
	size_t file_entry_name_length = 0;
	size_t file_entry_name_size = 0;
	size_t sub_path_size = 0;
	uint64_t file_entry_identifier = 0;
	int number_of_sub_file_entries = 0;
	int result = 0;
	int sub_file_entry_index = 0;

	xfsinfo_handle_t* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;

	try {
		verifyHandle(info_handle, path);

		if (path_length > (size_t)(SSIZE_MAX - 1))
			throw FDArgumentException(LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM, "invalid path length value exceeds maximum.");

		if (libfsxfs_file_entry_get_inode_number(file_entry, &file_entry_identifier, error) != 1)
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "%s: unable to retrieve inode number.");

		result = XFSGetNameLength(file_entry, &file_entry_name_size, error);

		if (result == -1)
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "unable to retrieve file entry name string size.");

		if ((result == 1) && (file_entry_name_size > 0))
		{
			file_entry_name = system_string_allocate(file_entry_name_size);

			if (file_entry_name == NULL)
			{
				throw FDMemoryException(LIBCERROR_MEMORY_ERROR_INSUFFICIENT, "unable to create file entry name string.");
			}
			result = XFSGetName(file_entry, (uint16_t*)file_entry_name, file_entry_name_size, error);

			if (result != 1)
				throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "unable to retrieve file entry name string.");

			file_entry_name_length = file_entry_name_size - 1;
		}
		if (info_handle->bodyfile_stream != NULL)
		{
			if (FileEntryValueWithName_fprint(file_entry, path, path_length, file_entry_name, file_entry_name_length) != 1)
				throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_PRINT_FAILED, "unable to print file entry.");
		}
		else
		{
			if (FileEntryValueWithName_fprint(file_entry, path, path_length, file_entry_name, file_entry_name_length) != 1)
			{}
			//throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_PRINT_FAILED, "unable to print file entry.");
			//if (PrintNameValue(path, path_length) != 1)
			//	throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_PRINT_FAILED, "unable to print path string.");

			//if (file_entry_name != NULL)
			//	if (PrintNameValue( file_entry_name, file_entry_name_length) != 1)
			//		throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_PRINT_FAILED, "unable to print file entry name string.");

			//fprintf(info_handle->notify_stream,"\n");
		}

		if (libfsxfs_file_entry_get_number_of_sub_file_entries(file_entry, &number_of_sub_file_entries, error) != 1)
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "unable to retrieve number of sub file entries.");

		if (number_of_sub_file_entries > 0)
		{
			sub_path_size = path_length + 1;

			if (file_entry_name != NULL)
				sub_path_size += file_entry_name_size;

			sub_path = system_string_allocate(sub_path_size);

			if (sub_path == NULL)
				throw FDMemoryException(LIBCERROR_MEMORY_ERROR_INSUFFICIENT, "unable to create sub path.");

			if (system_string_copy(sub_path, path, path_length) == NULL)
				throw FDMemoryException(LIBCERROR_MEMORY_ERROR_COPY_FAILED, "unable to copy path to sub path.");
			
			if (file_entry_name != NULL)
			{
				if (system_string_copy( &(sub_path[path_length]), file_entry_name, file_entry_name_size - 1) == NULL)
					throw FDMemoryException(LIBCERROR_MEMORY_ERROR_COPY_FAILED, "unable to copy file entry name to sub path.");

				sub_path[sub_path_size - 2] = (system_character_t)LIBFSXFS_SEPARATOR;
			}
			sub_path[sub_path_size - 1] = (system_character_t)0;

			for (sub_file_entry_index = 0;
				sub_file_entry_index < number_of_sub_file_entries;
				sub_file_entry_index++)
			{
				if (libfsxfs_file_entry_get_sub_file_entry_by_index(file_entry, sub_file_entry_index, &sub_file_entry, error) != 1)
					throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "unable to retrieve sub file entry: %d.");

				if (FindNext( sub_file_entry, sub_path, sub_path_size - 1) != 1) {
					std::stringstream msg;
					msg << "unable to print file entry: ";
					msg << sub_file_entry_index;
					msg << " information.";
					throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_PRINT_FAILED, msg.str());
				}

				if (libfsxfs_file_entry_free( &sub_file_entry, error) != 1) {
					std::stringstream msg;
					msg << "unable to free sub file entry: ";
					msg << sub_file_entry_index;
					throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED, msg.str());
				}
			}
			SAFE_FREE(sub_path);
		}
		SAFE_FREE(file_entry_name);
		return(1);
	}
	catch (FDMemoryException& e) {
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_MEMORY,
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
	}
	catch (FDArgumentException& e) {
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			e.code(),
			"%s: %s",
			function, e.what());
		return -1;
	}

	if (sub_file_entry != NULL)
		libfsxfs_file_entry_free(&sub_file_entry, NULL);

	SAFE_FREE(sub_path);
	SAFE_FREE(file_entry_name);

	return(-1);
}

/* Prints the file entries information
 * Returns 1 if successful or -1 on error
 */
int FDXFSInfoHandle::FileEntries_fprint()
{
	static const char* function = "info_handle_file_entries_fprint";
	uint64_t file_entry_identifier = 0;
	uint32_t number_of_file_entries = 0;
	xfsinfo_handle* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;
#ifdef TODO
	if (libfsxfs_volume_get_number_of_file_entries(
		info_handle->input_volume,
		&number_of_file_entries,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to retrieve number of file entries.",
			function);

		return(-1);
	}
#endif
	for (file_entry_identifier = 0;
		file_entry_identifier < (uint64_t)number_of_file_entries;
	file_entry_identifier++)
	{
		if (FileEntry_fprint_by_identifier(
			file_entry_identifier) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				"%s: unable to print file entry: %" PRIu64 " information.",
				function,
				file_entry_identifier);

			return(-1);
		}
	}
	return(1);
}

/* Prints the file entry information for a specific identifier
 * Returns 1 if successful, 0 if not or -1 on error
 */
int FDXFSInfoHandle::FileEntry_fprint_by_identifier(
	uint64_t file_entry_identifier)
{
	libfsxfs_file_entry_t* file_entry = NULL;
	static const char* function = "info_handle_file_entry_fprint_by_identifier";

#ifdef TODO
	int is_empty = 0;
#endif
	xfsinfo_handle_t* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;

	if (info_handle == NULL)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
			LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
			"%s: invalid info handle.",
			function);

		return(-1);
	}
	if (libfsxfs_volume_get_file_entry_by_inode(
		info_handle->input_volume,
		file_entry_identifier,
		&file_entry,
		error) != 1)
	{
		if ((error != NULL)
			&& (*error != NULL))
		{
			//libcnotify_print_error_backtrace(
			// *error );
		}
		libcerror_error_free(
			error);

		fprintf(
			info_handle->notify_stream,
			"Error reading file entry: %" PRIu64 "\n\n",
			file_entry_identifier);

		return(0);
	}
	fprintf(
		info_handle->notify_stream,
		"File entry: %" PRIu64 " information:\n",
		file_entry_identifier);

#ifdef TODO
	is_empty = libfsxfs_file_entry_is_empty(
		file_entry,
		error);

	if (is_empty == -1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			"%s: unable to determine if file entry is empty.",
			function);

		goto on_error;
	}
	else if (is_empty != 0)
	{
		fprintf(
			info_handle->notify_stream,
			"\tIs empty\n");
	}
	else
#endif /* TODO */
	{
		/* TODO implement is allocated */
		if (FileEntryValueWithName_fprint(
			file_entry,
			NULL,
			0,
			NULL,
			0) != 1)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_RUNTIME,
				LIBCERROR_RUNTIME_ERROR_PRINT_FAILED,
				"%s: unable to print file entry.",
				function);

			goto on_error;
		}
}
	if (libfsxfs_file_entry_free(
		&file_entry,
		error) != 1)
	{
		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			"%s: unable to free file entry.",
			function);

		goto on_error;
	}
	fprintf(
		info_handle->notify_stream,
		"\n");

	return(1);

on_error:
	if (file_entry != NULL)
	{
		libfsxfs_file_entry_free(
			&file_entry,
			NULL);
	}
	return(-1);
}

libfsxfs_file_entry_t* FDXFSInfoHandle::FindFirst()
{
	libfsxfs_file_entry_t* file_entry = NULL;
	static const char* function = "FDXFSInfoHandle::FindFirst()";
	int result = 0;

	xfsinfo_handle_t* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;

	if (info_handle == NULL)
		return NULL;

	result = libfsxfs_volume_get_root_directory(
		info_handle->input_volume,
		&file_entry,
		error);

	return file_entry;
}

/* Prints the file system hierarchy information
 * Returns 1 if successful or -1 on error
 */
int FDXFSInfoHandle::PrintFileSystemHierarchy()
{
	libfsxfs_file_entry_t* file_entry = NULL;
	static const char* function = "info_handle_file_system_hierarchy_fprint";
	int result = 0;

	xfsinfo_handle_t* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;
	try {
		verifyHandle(info_handle);

		if (info_handle->bodyfile_stream == NULL)
		{
			fprintf(info_handle->notify_stream, "X File System information:\n\n");
			fprintf(info_handle->notify_stream, "File system hierarchy:\n");
		}
		result = libfsxfs_volume_get_root_directory(
			info_handle->input_volume,
			&file_entry,
			error);

		if (result == -1)
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "unable to retrieve root directory file entry.");

		else if (result != 0)
		{
			if (FindNext(file_entry, _SYSTEM_STRING("/"), 1) != 1)
				throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_PRINT_FAILED, "unable to print root directory file entry information.");

			if (libfsxfs_file_entry_free(&file_entry, error) != 1)
				throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED, "unable to free file entry.");
		}
		if (info_handle->bodyfile_stream == NULL)
			fprintf(info_handle->notify_stream, "\n");

		return(1);
	}
	catch (FDRuntimeException& e)
	{
		if (file_entry != NULL)
			libfsxfs_file_entry_free(&file_entry, NULL);

		libcerror_error_set(
			error,
			LIBCERROR_ERROR_DOMAIN_RUNTIME,
			e.code(),
			"%s: %s",
			function, e.what());
	}

	return(-1);
}


/* Prints the volume information
 * Returns 1 if successful or -1 on error
 */
int FDXFSInfoHandle::PrintVolumeInfo()
{
	//system_character_t* value_string = NULL;
	static const char* function = "info_handle_volume_fprint";
	size_t value_string_size = 0;
	uint8_t format_version = 0;
	int result = 0;

	xfsinfo_handle_t* info_handle = GetInfoHandle();
	libcerror_error_t** error = &m_pError;
	try {
		verifyHandle(info_handle);
		fprintf(info_handle->notify_stream, "X File System information:\n\n");
		fprintf(info_handle->notify_stream, "Volume information:\n");

		if (libfsxfs_volume_get_format_version(
			info_handle->input_volume,
			&format_version,
			error) != 1)
		{
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "unable to retrieve format version.");
		}
		fprintf(
			info_handle->notify_stream,
			"\tFormat version\t\t\t: %" PRIu8 "\n",
			format_version);

		fprintf(info_handle->notify_stream, "\tLabel\t\t\t\t: ");

		result = getLabelSize(info_handle->input_volume, &value_string_size, error);

		if (result != 1)
			throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "unable to retrieve volume label size.");

		if (value_string_size > 0)
		{
			//value_string = system_string_allocate(value_string_size);
			std::wstring value;
			//value_string = (system_character_t*) value.data();
			//if (value_string == NULL)
			//	throw FDMemoryException(LIBCERROR_MEMORY_ERROR_INSUFFICIENT, "unable to create volume label string.");

			result = getLabel(info_handle->input_volume, (uint16_t*)value.data()/*value_string*/, value_string_size, error);
			if (result != 1)
				throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_GET_FAILED, "unable to retrieve volume label.");

			fprintf(info_handle->notify_stream, "%" PRIs_SYSTEM "", /*value_string*/value.c_str());

			//memory_free(value_string);
			//value_string = NULL;
		}
		fprintf(info_handle->notify_stream, "\n");

		/* TODO print more info */

		fprintf(info_handle->notify_stream, "\n");

		return(1);
	}
	catch (FDXFSException& e)
	{
		//if (value_string != NULL)
		//	memory_free(value_string);

		if (info_handle == NULL)
		{
			libcerror_error_set(
				error,
				LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
				e.code(),
				"%s: %s",
				function, e.what());

			return(-1);
		}
		return(-1);
	}
}

