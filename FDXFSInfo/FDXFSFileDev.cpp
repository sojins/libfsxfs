/*
 * File range functions
 *
 * Copyright (C) 2009-2022, Joachim Metz <joachim.metz@gmail.com>
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

#include "FDXFSFileDev.h"

#include <memory.h>
#include <system_string.h>

#include <libfsxfs.h>
#include <libcerror_error.h>

#include <libbfio_definitions.h>
#include <libbfio_file_io_handle.h>

#include <sstream>
#include <map>
#ifdef __cplusplus
extern "C" {
#endif
	namespace FD_XFS {
		static const char* fname = "xfs";
		static DWORDLONG dwlStartPageOfPartition = 0; // 파티션 데이터의 시작 위치 (1 Page = BytesPerSector bytes)
		typedef std::map<std::string, struct xfs_blockdev*> XfsBlockDev;
		typedef std::map<std::string, std::string> XfsDevName;
		static XfsBlockDev file_devs;
		static XfsDevName dev_names;

		namespace DeviceIO {
			/**********************BLOCKDEV INTERFACE**************************************/
			static int Open(file_dev_handle_t* file_range_io_handle, int access_flags, libcerror_error_t** error);
			static ssize_t Read(file_dev_handle_t* file_range_io_handle, uint8_t* buffer, size_t size, libcerror_error_t** error);
			static ssize_t Write(file_dev_handle_t* file_range_io_handle, const uint8_t* buffer, size_t size, libcerror_error_t** error);
			static int Close(file_dev_handle_t* file_range_io_handle, libcerror_error_t** error);
			static int Clone(file_dev_handle_t** destination_file_range_io_handle, file_dev_handle_t* source_file_range_io_handle, libcerror_error_t** error);
			static int Free(file_dev_handle_t** file_range_io_handle, libcerror_error_t** error);
			static off64_t Seek(file_dev_handle_t* file_range_io_handle, off64_t offset, int whence, libcerror_error_t** error);
			static int Exists(file_dev_handle_t* file_range_io_handle, libcerror_error_t** error);
			static int IsOpen(file_dev_handle_t* file_range_io_handle, libcerror_error_t** error);
			static int GetSize(file_dev_handle_t* file_range_io_handle, size64_t* size, libcerror_error_t** error);
			/******************************************************************************/

			static int Initialize(file_dev_handle_t** file_range_io_handle, libcerror_error_t** error);
			static int GetNameSize(file_dev_handle_t* file_range_io_handle, size_t* name_size, libcerror_error_t** error);
			static int GetNameSizeWide(file_dev_handle_t* file_range_io_handle, size_t* name_size, libcerror_error_t** error);
			static int GetName(file_dev_handle_t* file_range_io_handle, char* name, size_t name_size, libcerror_error_t** error);
			static int GetNameWide(file_dev_handle_t* file_range_io_handle, wchar_t* name, size_t name_size, libcerror_error_t** error);
			static int SetNameWide(file_dev_handle_t* file_range_io_handle, const wchar_t* name, size_t name_length, libcerror_error_t** error);
			static int Set(file_dev_handle_t* file_range_io_handle, off64_t range_offset, size64_t range_size, libcerror_error_t** error);
			static int Get(file_dev_handle_t* file_range_io_handle, off64_t* range_offset, size64_t* range_size, libcerror_error_t** error);
			static int SetName(file_dev_handle_t* file_range_io_handle, const char* name, size_t name_length, libcerror_error_t** error);

			/* Creates a file range IO handle
			* Make sure the value file_range_io_handle is referencing, is set to NULL
			* Returns 1 if successful or -1 on error
			*/
			int Initialize(file_dev_handle_t** file_range_io_handle, libcerror_error_t** error)
				{
					static const char* function = "initialize";

					if (file_range_io_handle == NULL)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
							LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
							"%s: invalid file range IO handle.",
							function);

						return(-1);
					}
					if (*file_range_io_handle != NULL)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_RUNTIME,
							LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
							"%s: invalid file range IO handle value already set.",
							function);

						return(-1);
					}
					*file_range_io_handle = memory_allocate_structure(
						file_dev_handle_t);

					if (*file_range_io_handle == NULL)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_MEMORY,
							LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
							"%s: unable to create file range IO handle.",
							function);

						goto on_error;
					}
					if (memory_set(
						*file_range_io_handle,
						0,
						sizeof(file_dev_handle_t)) == NULL)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_MEMORY,
							LIBCERROR_MEMORY_ERROR_SET_FAILED,
							"%s: unable to clear file range IO handle.",
							function);

						goto on_error;
					}
					if (libbfio_file_io_handle_initialize(
						&((*file_range_io_handle)->file_io_handle),
						error) != 1)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_RUNTIME,
							LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
							"%s: unable to initialize file IO handle.",
							function);

						goto on_error;
					}
					return(1);

				on_error:
					if (*file_range_io_handle != NULL)
					{
						memory_free(
							*file_range_io_handle);

						*file_range_io_handle = NULL;
					}
					return(-1);
				}

			/* Frees a file range IO handle
			 * Returns 1 if succesful or -1 on error
			 */
			int Free(file_dev_handle_t** file_range_io_handle, libcerror_error_t** error)
			{
				static const char* function = "free";
				int result = 1;

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (*file_range_io_handle != NULL)
				{
					if (libbfio_file_io_handle_free(
						&((*file_range_io_handle)->file_io_handle),
						error) != 1)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_RUNTIME,
							LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
							"%s: unable to free file IO handle.",
							function);

						result = -1;
					}
					memory_free(
						*file_range_io_handle);

					*file_range_io_handle = NULL;
				}
				return(result);
			}

			/* Clones (duplicates) the file range IO handle and its attributes
			 * Returns 1 if succesful or -1 on error
			 */
			int Clone(file_dev_handle_t** destination_file_range_io_handle, file_dev_handle_t* source_file_range_io_handle, libcerror_error_t** error)
			{
				static const char* function = "clone";

				if (destination_file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid destination file range IO handle.",
						function);

					return(-1);
				}
				if (*destination_file_range_io_handle != NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
						"%s: destination file range IO handle already set.",
						function);

					return(-1);
				}
				if (source_file_range_io_handle == NULL)
				{
					*destination_file_range_io_handle = NULL;

					return(1);
				}
				*destination_file_range_io_handle = memory_allocate_structure(
					file_dev_handle_t);

				if (*destination_file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_MEMORY,
						LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
						"%s: unable to create destination file range IO handle.",
						function);

					goto on_error;
				}
				if (memory_set(
					*destination_file_range_io_handle,
					0,
					sizeof(file_dev_handle_t)) == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_MEMORY,
						LIBCERROR_MEMORY_ERROR_SET_FAILED,
						"%s: unable to clear destination file range IO handle.",
						function);

					memory_free(
						*destination_file_range_io_handle);

					*destination_file_range_io_handle = NULL;

					return(-1);
				}
				if (libbfio_file_io_handle_clone(
					&((*destination_file_range_io_handle)->file_io_handle),
					source_file_range_io_handle->file_io_handle,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
						"%s: unable to create file IO handle.",
						function);

					goto on_error;
				}
				(*destination_file_range_io_handle)->range_offset = source_file_range_io_handle->range_offset;
				(*destination_file_range_io_handle)->range_size = source_file_range_io_handle->range_size;

				return(1);

			on_error:
				if (*destination_file_range_io_handle != NULL)
				{
					Free(
						destination_file_range_io_handle,
						NULL);
				}
				return(-1);
			}

			/* Retrieves the name size of the file range IO handle
			 * The name size includes the end of string character
			 * Returns 1 if succesful or -1 on error
			 */
			int GetNameSize(file_dev_handle_t* file_range_io_handle, size_t* name_size, libcerror_error_t** error)
			{
				static const char* function = "get_name_size";

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (libbfio_file_io_handle_get_name_size(
					file_range_io_handle->file_io_handle,
					name_size,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve name size from file IO handle.",
						function);

					return(-1);
				}
				
				return(1);
			}

			/* Retrieves the name of the file range IO handle
			 * The name size should include the end of string character
			 * Returns 1 if succesful or -1 on error
			 */
			int GetName(file_dev_handle_t* file_range_io_handle, char* name, size_t name_size, libcerror_error_t** error)
			{
				static const char* function = "get_name";

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (libbfio_file_io_handle_get_name(
					file_range_io_handle->file_io_handle,
					name,
					name_size,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve name from file IO handle.",
						function);

					return(-1);
				}
				return(1);
			}

			/* Retrieves the name size of the file range IO handle
			 * The name size includes the end of string character
			 * Returns 1 if succesful or -1 on error
			 */
			int GetNameSizeWide(file_dev_handle_t* file_range_io_handle, size_t* name_size, libcerror_error_t** error)
			{
				static const char* function = "get_name_size_wide";

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (libbfio_file_io_handle_get_name_size_wide(
					file_range_io_handle->file_io_handle,
					name_size,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve name size from file IO handle.",
						function);

					return(-1);
				}
				return(1);
			}

			/* Retrieves the name of the file range IO handle
			 * The name size should include the end of string character
			 * Returns 1 if succesful or -1 on error
			 */
			int GetNameWide(file_dev_handle_t* file_range_io_handle, wchar_t* name, size_t name_size, libcerror_error_t** error)
			{
				static const char* function = "get_name_wide";

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (libbfio_file_io_handle_get_name_wide(
					file_range_io_handle->file_io_handle,
					name,
					name_size,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve name from file IO handle.",
						function);

					return(-1);
				}
				return(1);
			}

			/* Sets the name for the file range IO handle
			 * Returns 1 if succesful or -1 on error
			 */
			int SetNameWide(file_dev_handle_t* file_range_io_handle, const wchar_t* name, size_t name_length, libcerror_error_t** error)
			{
				static const char* function = "set_name_wide";

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (libbfio_file_io_handle_set_name_wide(
					file_range_io_handle->file_io_handle,
					name,
					name_length,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_SET_FAILED,
						"%s: unable to set name in file IO handle.",
						function);

					return(-1);
				}
				return(1);
			}

			/* Retrieves the range of the file range IO handle
			 * Returns 1 if succesful or -1 on error
			 */
			int Get(file_dev_handle_t* file_range_io_handle, off64_t* range_offset, size64_t* range_size, libcerror_error_t** error)
			{
				static const char* function = "get";

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (range_offset == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid range offset.",
						function);

					return(-1);
				}
				if (range_size == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid range size.",
						function);

					return(-1);
				}
				*range_offset = file_range_io_handle->range_offset;
				*range_size = file_range_io_handle->range_size;

				return(1);
			}

			/* Sets the range of the file range IO handle
			 * A range size of 0 represents that the range continues until the end of the file
			 * Returns 1 if succesful or -1 on error
			 */
			int Set(file_dev_handle_t* file_range_io_handle, off64_t range_offset, size64_t range_size, libcerror_error_t** error)
			{
				static const char* function = "set";

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (range_offset < 0)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_VALUE_LESS_THAN_ZERO,
						"%s: invalid range offset value less than zero.",
						function);

					return(-1);
				}
				if (range_size > (size64_t)INT64_MAX)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
						"%s: invalid range size value exceeds maximum.",
						function);

					return(-1);
				}
				file_range_io_handle->range_offset = range_offset;
				file_range_io_handle->range_size = range_size;

				return(1);
			}

			/* Opens the file range IO handle
			 * Returns 1 if successful or -1 on error
			 */
			int Open(file_dev_handle_t* file_range_io_handle, int access_flags, libcerror_error_t** error)
			{
				static const char* function = "open";
				size64_t file_size = 0;

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (libbfio_file_io_handle_open(
					file_range_io_handle->file_io_handle,
					access_flags,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_IO,
						LIBCERROR_IO_ERROR_OPEN_FAILED,
						"%s: unable to open file IO handle.",
						function);

					return(-1);
				}
				if (libbfio_file_io_handle_get_size(
					file_range_io_handle->file_io_handle,
					&file_size,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve size from file IO handle.",
						function);

					return(-1);
				}
				if (file_range_io_handle->range_offset >= (off64_t)file_size)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
						"%s: invalid range offset value exceeds file size.",
						function);

					return(-1);
				}
				file_size -= file_range_io_handle->range_offset;

				if (file_range_io_handle->range_size > file_size)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
						"%s: invalid range size value exceeds file size.",
						function);

					return(-1);
				}
				if (libbfio_file_io_handle_seek_offset(
					file_range_io_handle->file_io_handle,
					file_range_io_handle->range_offset,
					SEEK_SET,
					error) == -1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_IO,
						LIBCERROR_IO_ERROR_SEEK_FAILED,
						"%s: unable to seek range offset in file IO handle.",
						function);

					return(-1);
				}
				return(1);
			}

			/* Closes the file range IO handle
			 * Returns 0 if successful or -1 on error
			 */
			int Close(file_dev_handle_t* file_range_io_handle, libcerror_error_t** error)
			{
				static const char* function = "close";

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (libbfio_file_io_handle_close(
					file_range_io_handle->file_io_handle,
					error) != 0)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_IO,
						LIBCERROR_IO_ERROR_CLOSE_FAILED,
						"%s: unable to close file IO handle.",
						function);

					return(-1);
				}
				return(0);
			}

			/* Reads a buffer from the file range IO handle
			 * Returns the number of bytes read if successful, or -1 on error
			 */
			ssize_t Read(file_dev_handle_t* file_range_io_handle, uint8_t* buffer, size_t size, libcerror_error_t** error)
			{
				static const char* function = "bread";
				off64_t file_offset = 0;
				ssize_t read_count = 0;

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				file_offset = libbfio_file_io_handle_seek_offset(
					file_range_io_handle->file_io_handle,
					0,
					SEEK_CUR,
					error);

				if (file_offset == -1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve current offset from file IO handle.",
						function);

					return(-1);
				}
				if (file_offset < file_range_io_handle->range_offset)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
						"%s: invalid file offset value out of bounds.",
						function);

					return(-1);
				}
				if (file_range_io_handle->range_size != 0)
				{
					if ((size64_t)file_offset >= file_range_io_handle->range_size)
					{
						return(0);
					}
					if ((size64_t)(file_offset + size) >= file_range_io_handle->range_size)
					{
						size = (size_t)(file_range_io_handle->range_offset - file_offset);
					}
				}
				read_count = libbfio_file_io_handle_read_buffer(
					file_range_io_handle->file_io_handle,
					buffer,
					size,
					error);

				if (read_count == -1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_IO,
						LIBCERROR_IO_ERROR_READ_FAILED,
						"%s: unable to read from file IO handle.",
						function);

					return(-1);
				}
				return(read_count);
			}

			/* Writes a buffer to the file range IO handle
			 * Returns the number of bytes written if successful, or -1 on error
			 */
			ssize_t Write(file_dev_handle_t* file_range_io_handle, const uint8_t* buffer, size_t size, libcerror_error_t** error)
			{
				static const char* function = "bwrite";
				off64_t file_offset = 0;
				ssize_t write_count = 0;

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				file_offset = libbfio_file_io_handle_seek_offset(
					file_range_io_handle->file_io_handle,
					0,
					SEEK_CUR,
					error);

				if (file_offset == -1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_GET_FAILED,
						"%s: unable to retrieve current offset from file IO handle.",
						function);

					return(-1);
				}
				if (file_offset < file_range_io_handle->range_offset)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
						"%s: invalid file offset value out of bounds.",
						function);

					return(-1);
				}
				if (file_range_io_handle->range_size != 0)
				{
					if ((size64_t)file_offset >= file_range_io_handle->range_size)
					{
						return(0);
					}
					if ((size64_t)(file_offset + size) >= file_range_io_handle->range_size)
					{
						size = (size_t)(file_range_io_handle->range_offset - file_offset);
					}
				}
				write_count = libbfio_file_io_handle_write_buffer(
					file_range_io_handle->file_io_handle,
					buffer,
					size,
					error);

				if (write_count == -1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_IO,
						LIBCERROR_IO_ERROR_READ_FAILED,
						"%s: unable to write to file IO handle.",
						function);

					return(-1);
				}
				return(write_count);
			}

			/* Seeks a certain offset within the file range IO handle
			 * Returns the offset if the seek is successful or -1 on error
			 */
			off64_t Seek(file_dev_handle_t* file_range_io_handle, off64_t offset, int whence, libcerror_error_t** error)
			{
				static const char* function = "seek_offset";
				off64_t file_offset = 0;
				off64_t seek_offset = 0;

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if ((whence != SEEK_CUR)
					&& (whence != SEEK_END)
					&& (whence != SEEK_SET))
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
						"%s: unsupported whence.",
						function);

					return(-1);
				}
				if (whence == SEEK_CUR)
				{
					file_offset = libbfio_file_io_handle_seek_offset(
						file_range_io_handle->file_io_handle,
						0,
						SEEK_CUR,
						error);

					if (file_offset == -1)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_RUNTIME,
							LIBCERROR_RUNTIME_ERROR_GET_FAILED,
							"%s: unable to retrieve current offset from file IO handle.",
							function);

						return(-1);
					}
					offset += file_offset;
					whence = SEEK_SET;
				}
				else if (whence == SEEK_END)
				{
					if (file_range_io_handle->range_size != 0)
					{
						offset += file_range_io_handle->range_size;
						whence = SEEK_SET;
					}
				}
				else if (whence == SEEK_SET)
				{
					offset += file_range_io_handle->range_offset;
				}
				if (whence == SEEK_SET)
				{
					if (offset < file_range_io_handle->range_offset)
					{
						libcerror_error_set(
							error,
							LIBCERROR_ERROR_DOMAIN_RUNTIME,
							LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
							"%s: invalid offset value out of bounds.",
							function);

						return(-1);
					}
				}
				seek_offset = libbfio_file_io_handle_seek_offset(
					file_range_io_handle->file_io_handle,
					offset,
					whence,
					error);

				if (seek_offset == -1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_IO,
						LIBCERROR_IO_ERROR_SEEK_FAILED,
						"%s: unable to seek offset: %" PRIi64 " in file IO handle.",
						function,
						offset);

					return(-1);
				}
				if (seek_offset < file_range_io_handle->range_offset)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
						"%s: invalid offset: %" PRIi64 " value out of bounds.",
						function,
						seek_offset);

					return(-1);
				}
				seek_offset -= file_range_io_handle->range_offset;

				return(seek_offset);
			}

			/* Function to determine if a file range exists
			 * Returns 1 if file exists, 0 if not or -1 on error
			 */
			int Exists(file_dev_handle_t* file_range_io_handle, libcerror_error_t** error)
			{
				static const char* function = "exists";
				int result = 0;

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				result = libbfio_file_io_handle_exists(
					file_range_io_handle->file_io_handle,
					error);

				if (result == -1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_IO,
						LIBCERROR_IO_ERROR_GENERIC,
						"%s: unable to determine if file exists.",
						function);

					return(-1);
				}
				return(result);
			}

			/* Check if the file range IO handle is open
			 * Returns 1 if open, 0 if not or -1 on error
			 */
			int IsOpen(file_dev_handle_t* file_range_io_handle, libcerror_error_t** error)
			{
				static const char* function = "is_open";
				int result = 0;

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				result = libbfio_file_io_handle_is_open(
					file_range_io_handle->file_io_handle,
					error);

				if (result == -1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_IO,
						LIBCERROR_IO_ERROR_GENERIC,
						"%s: unable to determine if file is open.",
						function);

					return(-1);
				}
				return(result);
			}

			/* Retrieves the file range size
			 * Returns 1 if successful or -1 on error
			 */
			int GetSize(file_dev_handle_t* file_range_io_handle, size64_t* size, libcerror_error_t** error)
			{
				static const char* function = "get_size";

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (size == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid size.",
						function);

					return(-1);
				}
				if (file_range_io_handle->range_size == 0)
				{
					//if (libbfio_file_io_handle_get_size(
					//	file_range_io_handle->file_io_handle,
					//	size,
					//	error) != 1)
					//{
					//	libcerror_error_set(
					//		error,
					//		LIBCERROR_ERROR_DOMAIN_RUNTIME,
					//		LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					//		"%s: unable to determine size.",
					//		function);

					//	return(-1);
					//}
					*size -= file_range_io_handle->range_offset;
				}
				else
				{
					*size = file_range_io_handle->range_size;
				}
				return(1);
			}

			/* Sets the name for the file range IO handle
			* Returns 1 if succesful or -1 on error
			*/
			int SetName(file_dev_handle_t* file_range_io_handle, const char* name, size_t name_length, libcerror_error_t** error)
			{
				static const char* function = "set_name";

				if (file_range_io_handle == NULL)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
						LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
						"%s: invalid file range IO handle.",
						function);

					return(-1);
				}
				if (libbfio_file_io_handle_set_name(
					file_range_io_handle->file_io_handle,
					name,
					name_length,
					error) != 1)
				{
					libcerror_error_set(
						error,
						LIBCERROR_ERROR_DOMAIN_RUNTIME,
						LIBCERROR_RUNTIME_ERROR_SET_FAILED,
						"%s: unable to set name in file IO handle.",
						function);

					return(-1);
				}
				return(1);
			}
		};

		/***************************************************************************/
		/* FINALDATA; FILE RANGE HELPER */
		/***************************************************************************/
		/* Creates a file range handle
		* Make sure the value handle is referencing, is set to NULL
		* Returns 1 if successful or -1 on error
		*/
		int file_dev_initialize(libbfio_handle_t** handle, libcerror_error_t** error)
		{
			file_dev_handle_t* file_range_io_handle = NULL;
			static const char* function = "file_dev_initialize";

			if (handle == NULL)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
					LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
					"%s: invalid handle.",
					function);

				return(-1);
			}
			if (*handle != NULL)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
					"%s: invalid handle value already set.",
					function);

				return(-1);
			}

			if (DeviceIO::Initialize(
				&file_range_io_handle,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					"%s: unable to create file range IO handle.",
					function);

				goto on_error;
			}

			if (libbfio_handle_initialize(
				(libbfio_handle_t**)handle,
				(intptr_t*)file_range_io_handle,
				(int (*)(intptr_t**, libcerror_error_t**)) DeviceIO::Free,
				(int (*)(intptr_t**, intptr_t*, libcerror_error_t**)) DeviceIO::Clone,
				(int (*)(intptr_t*, int, libcerror_error_t**)) DeviceIO::Open,
				(int (*)(intptr_t*, libcerror_error_t**)) DeviceIO::Close,
				(ssize_t(*)(intptr_t*, uint8_t*, size_t, libcerror_error_t**)) DeviceIO::Read,
				(ssize_t(*)(intptr_t*, const uint8_t*, size_t, libcerror_error_t**)) DeviceIO::Write,
				(off64_t(*)(intptr_t*, off64_t, int, libcerror_error_t**)) DeviceIO::Seek,
				(int (*)(intptr_t*, libcerror_error_t**)) DeviceIO::Exists,
				(int (*)(intptr_t*, libcerror_error_t**)) DeviceIO::IsOpen,
				(int (*)(intptr_t*, size64_t*, libcerror_error_t**)) DeviceIO::GetSize,
				LIBBFIO_FLAG_IO_HANDLE_MANAGED | LIBBFIO_FLAG_IO_HANDLE_CLONE_BY_FUNCTION,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					"%s: unable to create handle.",
					function);

				goto on_error;
			}
			return(1);

		on_error:
			if (file_range_io_handle != NULL)
			{
				//libbfio_file_range_io_handle_free(
				DeviceIO::Free(
					&file_range_io_handle,
					NULL);
			}
			return(-1);
		}

		/* Retrieves the name of the file range handle
		* The name size should include the end of string character
		* Returns 1 if succesful or -1 on error
		*/
		int file_dev_get_name(libbfio_handle_t* handle, char* name, size_t name_size, libcerror_error_t** error)
		{
			libbfio_internal_handle_t* internal_handle = NULL;
			static const char* function = "file_dev_get_name";

			if (handle == NULL)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
					LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
					"%s: invalid handle.",
					function);

				return(-1);
			}
			internal_handle = (libbfio_internal_handle_t*)handle;

			if (DeviceIO::GetName(
				(file_dev_handle_t*)internal_handle->io_handle,
				name,
				name_size,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					"%s: unable to retrieve name from file IO handle.",
					function);

				return(-1);
			}
			return(1);
		}
		
		/* Retrieves the name size of the file range handle
		* The name size includes the end of string character
		* Returns 1 if succesful or -1 on error
		*/
		int file_dev_get_name_size_wide(libbfio_handle_t* handle, size_t* name_size, libcerror_error_t** error)
		{
			libbfio_internal_handle_t* internal_handle = NULL;
			static const char* function = "file_dev_get_name_size_wide";

			if (handle == NULL)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
					LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
					"%s: invalid handle.",
					function);

				return(-1);
			}
			internal_handle = (libbfio_internal_handle_t*)handle;

			if (DeviceIO::GetNameSizeWide(
				(file_dev_handle_t*)internal_handle->io_handle,
				name_size,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					"%s: unable to retrieve name size from file IO handle.",
					function);

				return(-1);
			}
			return(1);
		}


		/* Retrieves the range of the file range handle
		 * Returns 1 if succesful or -1 on error
		 */
		int file_dev_get(libbfio_handle_t* handle, off64_t* range_offset, size64_t* range_size, libcerror_error_t** error)
		{
			libbfio_internal_handle_t* internal_handle = NULL;
			static const char* function = "file_dev_get";

			if (handle == NULL)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
					LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
					"%s: invalid handle.",
					function);

				return(-1);
			}
			internal_handle = (libbfio_internal_handle_t*)handle;

			if (DeviceIO::Get(
				(file_dev_handle_t*)internal_handle->io_handle,
				range_offset,
				range_size,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					"%s: unable to retrieve range from file IO handle.",
					function);

				return(-1);
			}
			return(1);
		}

		/* Sets the range of the file range handle
		 * A range size of 0 represents that the range continues until the end of the file
		 * Returns 1 if succesful or -1 on error
		 */
		int file_dev_set(libbfio_handle_t* handle, off64_t range_offset, size64_t range_size, libcerror_error_t** error)
		{
			libbfio_internal_handle_t* internal_handle = NULL;
			static const char* function = "file_dev_set";

			if (handle == NULL)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
					LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
					"%s: invalid handle.",
					function);

				return(-1);
			}
			internal_handle = (libbfio_internal_handle_t*)handle;

			if (DeviceIO::Set(
				(file_dev_handle_t*)internal_handle->io_handle,
				range_offset,
				range_size,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_SET_FAILED,
					"%s: unable to set range in file IO handle.",
					function);

				return(-1);
			}
			return(1);
		}

		/* Retrieves the name of the file range handle
		 * The name size should include the end of string character
		 * Returns 1 if succesful or -1 on error
		 */
		int file_dev_get_name_wide(libbfio_handle_t* handle, wchar_t* name, size_t name_size, libcerror_error_t** error)
		{
			libbfio_internal_handle_t* internal_handle = NULL;
			static const char* function = "file_dev_get_name_wide";

			if (handle == NULL)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
					LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
					"%s: invalid handle.",
					function);

				return(-1);
			}
			internal_handle = (libbfio_internal_handle_t*)handle;

			if (DeviceIO::GetNameWide(
				(file_dev_handle_t*)internal_handle->io_handle,
				name,
				name_size,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					"%s: unable to retrieve name from file IO handle.",
					function);

				return(-1);
			}
			return(1);
		}

		/* Retrieves the name size of the file range handle
		 * The name size includes the end of string character
		 * Returns 1 if succesful or -1 on error
		 */
		int file_dev_get_name_size(libbfio_handle_t* handle, size_t* name_size, libcerror_error_t** error)
		{
			libbfio_internal_handle_t* internal_handle = NULL;
			static const char* function = "file_dev_get_name_size";

			if (handle == NULL)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
					LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
					"%s: invalid handle.",
					function);

				return(-1);
			}
			internal_handle = (libbfio_internal_handle_t*)handle;

			if (DeviceIO::GetNameSize(
				(file_dev_handle_t*)internal_handle->io_handle,
				name_size,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_GET_FAILED,
					"%s: unable to retrieve name size from file IO handle.",
					function);

				return(-1);
			}
			return(1);
		}
		
		/* Sets the name for the file range handle
		 * Returns 1 if succesful or -1 on error
		 */
		int file_dev_set_name(libbfio_handle_t* handle, const char* name, size_t name_length, libcerror_error_t** error)
		{
			libbfio_internal_handle_t* internal_handle = NULL;
			static const char* function = "file_dev_set_name";

			if (handle == NULL)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
					LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
					"%s: invalid handle.",
					function);

				return(-1);
			}
			internal_handle = (libbfio_internal_handle_t*)handle;

			if (DeviceIO::SetName(
				(file_dev_handle_t*)internal_handle->io_handle,
				name,
				name_length,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_SET_FAILED,
					"%s: unable to set name in file IO handle.",
					function);

				return(-1);
			}
			return(1);
		}

#if defined( HAVE_WIDE_CHARACTER_TYPE )
		/* Sets the name for the file range handle
		* Returns 1 if succesful or -1 on error
		*/
		int file_dev_set_name_wide(libbfio_handle_t* handle, const wchar_t* name, size_t name_length, libcerror_error_t** error)
		{
			libbfio_internal_handle_t* internal_handle = NULL;
			static const char* function = "file_dev_set_name_wide";

			if (handle == NULL)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
					LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
					"%s: invalid handle.",
					function);

				return(-1);
			}
			internal_handle = (libbfio_internal_handle_t*)handle;

			if (DeviceIO::SetNameWide(
				(file_dev_handle_t*)internal_handle->io_handle,
				name,
				name_length,
				error) != 1)
			{
				libcerror_error_set(
					error,
					LIBCERROR_ERROR_DOMAIN_RUNTIME,
					LIBCERROR_RUNTIME_ERROR_SET_FAILED,
					"%s: unable to set name in file IO handle.",
					function);

				return(-1);
			}
			return(1);
		}

#endif /* defined( HAVE_WIDE_CHARACTER_TYPE ) */

		xfs_blockdev* open_blockdev(const char* label, CVirtualDrive* pVdrive)
		{
			try {
				//std::string deviceName = label;
				//struct ext4_blockdev* bd = file_dev_get(label);
				XfsBlockDev::iterator it = file_devs.find(label);
				xfs_blockdev* bd = NULL;
				if (it != file_devs.end())
				{
					bd = it->second;
				}
				else
				{
					bd = new xfs_blockdev;
					//if (bd) {
					//	bd->handle = (libbfio_handle_t*)memory_allocate_structure(libbfio_internal_handle_t);
					//}
					if (memory_set(bd, 0, sizeof(xfs_blockdev)) == NULL)
						throw FDMemoryException(LIBCERROR_MEMORY_ERROR_SET_FAILED, "unable to clear info handle.");
					file_devs.insert(std::make_pair(label, bd));
				}

				 
				libbfio_handle_t** handle = &bd->handle;
				if (handle == NULL)
					throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED, "unable to initialize input file IO handle.");

				if (file_dev_initialize(handle, NULL) != 1)
					throw FDRuntimeException(LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED, "unable to initialize input file IO handle.");

				size_t filename_length = strlen(label);
				libcerror_error_t** error = NULL;
				off64_t range_offset = 0;

				if (file_dev_set_name(*handle, label, filename_length, error) != 1)
					throw FDArgumentException(LIBCERROR_RUNTIME_ERROR_SET_FAILED, "unable to set file name.");

				if (FD_XFS::file_dev_set(*handle, range_offset, 0, error) != 1)
					throw FDArgumentException(LIBCERROR_RUNTIME_ERROR_SET_FAILED, "unable to set range.");

				if (bd) {
					bd->handle = *handle;
					bd->error = NULL;
				}
				return bd;
			}
			catch (FDXFSException& e) {
				//libcerror_error_set(
				//	error,
				//	LIBCERROR_ERROR_DOMAIN_RUNTIME,
				//	LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				//	"%s: unable to set name in file IO handle.",
				//	function);
				return nullptr;
			}
		}

		BOOL mount(xfs_blockdev* bdev, const char* mp_name)
		{
			//int file_dev_initialize(
			//	libbfio_handle_t * *handle,
			//	libcerror_error_t * *error);


			//int file_dev_set_name_wide(
			//	libbfio_handle_t * file_range_io_handle,
			//	const wchar_t* name,
			//	size_t name_length,
			//	libcerror_error_t * *error);

			return 0;
		}

		BOOL umount(const char* mp_name)
		{
			return 0;
		}
	};

#ifdef __cplusplus
}
#endif
