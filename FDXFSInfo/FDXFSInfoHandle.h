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

#if !defined( _INFO_HANDLE_H )
#define _INFO_HANDLE_H

#include <common.h>
#include <types.h>

#include <file_stream.h>
#include <libcerror_types.h>

 /* Define HAVE_LOCAL_LIBFDATETIME for local use of libfdatetime
  */
#if defined( HAVE_LOCAL_LIBFDATETIME )

#include <libfdatetime_date_time_values.h>
#include <libfdatetime_definitions.h>
#include <libfdatetime_fat_date_time.h>
#include <libfdatetime_filetime.h>
#include <libfdatetime_floatingtime.h>
#include <libfdatetime_hfs_time.h>
#include <libfdatetime_nsf_timedate.h>
#include <libfdatetime_posix_time.h>
#include <libfdatetime_systemtime.h>
#include <libfdatetime_types.h>

#endif 
#if defined( HAVE_LOCAL_LIBBFIO )

#include <libbfio_definitions.h>
#include <libbfio_file.h>
#include <libbfio_file_pool.h>
#include <libbfio_file_range.h>
#include <libbfio_handle.h>
#include <libbfio_memory_range.h>
#include <libbfio_pool.h>
#include <libbfio_types.h>

#endif

#include "FDXFSException.h"
#include "FDXFSFileDev.h"

typedef struct xfsinfo_handle xfsinfo_handle_t;

struct xfsinfo_handle
{
	off64_t volume_offset;					/* The volume offset */
	libbfio_handle_t* input_file_io_handle;	/* The libbfio input file IO handle */
	libfsxfs_volume_t* input_volume;		/* The libfsxfs input volume */
	uint8_t calculate_md5;					/* Value to indicate if the MD5 hash should be calculated */
	FILE* bodyfile_stream;					/* The bodyfile output stream */
	FILE* notify_stream;					/* The notification output stream */
	int abort;								/* Value to indicate if abort was signalled */
};

class FDXFSInfoHandle {
public:
	FDXFSInfoHandle();
	~FDXFSInfoHandle();

private:
	xfsinfo_handle_t* m_pInfo;
	libcerror_error_t* m_pError;

public:
	xfsinfo_handle_t* GetInfoHandle() { return m_pInfo; }

	int Initialize();

	int Finalize();

	//int SignalAbort();

	//int SetBodyFile(const system_character_t* filename);

	//int SetVolumeOffset(const system_character_t* string);
	void Mount(const system_character_t* filename);

	int Open(const system_character_t* filename);

	int Close();

	int PrintNameValue(const system_character_t* value_string, size_t value_string_length);

	int PosixTimeInNanoSecondsValue_fprint(const char* value_name, int64_t value_64bit);

	int FileEntryValueWithName_fprint(libfsxfs_file_entry_t* file_entry, const system_character_t* path, size_t path_length, const system_character_t* file_entry_name, size_t file_entry_name_length );

	int FileEntries_fprint();

	int FileEntry_fprint_by_identifier(uint64_t file_entry_identifier);

	//int FileEntry_fprint_by_path(const system_character_t* path);

	int PrintFileSystemHierarchy();

	int PrintVolumeInfo();

	int FindNext(libfsxfs_file_entry_t* file_entry, const system_character_t* path, size_t path_length);
	//int GetEntryName(libfsxfs_file_entry_t* file_entry, system_character_t* file_entry_name, size_t* file_entry_name_size);

	//int GetEntryNameLen(libfsxfs_file_entry_t* file_entry, size_t* file_entry_name_size);

	void SAFE_FREE(void* pBuffer);

	libfsxfs_file_entry_t* FindFirst();
};

#endif /* !defined( _INFO_HANDLE_H ) */
