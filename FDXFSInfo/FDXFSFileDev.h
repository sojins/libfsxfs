#if !defined( _FDXFS_FILE_DEV_H )
#define _FDXFS_FILE_DEV_H

#include <common.h>
#include <types.h>

#include <file_stream.h>
#include <libcerror_types.h>

#if defined( HAVE_LOCAL_LIBBFIO )

#include <libbfio_definitions.h>
#include <libbfio_file.h>
#include <libbfio_file_pool.h>
#include <libbfio_file_io_handle.h>
#include <libbfio_handle.h>
#include <libbfio_memory_range.h>
#include <libbfio_pool.h>
#include <libbfio_types.h>

#endif

#include "FDXFSException.h"
class CVirtualDrive;
#ifdef __cplusplus
extern "C" {
#endif
	typedef struct file_dev_handle file_dev_handle_t;

	struct file_dev_handle
	{
		libbfio_file_io_handle_t * file_io_handle;	/* The file IO handle */
		off64_t range_offset;						/* The range (start) offset */
		size64_t range_size;						/* The range size */
	};

	typedef struct xfs_blockdev xfs_blockdev_t;
	struct xfs_blockdev
	{
		libbfio_handle_t*   handle;
		libcerror_error_t*  error;
	};

	namespace FD_XFS {

		xfs_blockdev* open_blockdev(const char* label, CVirtualDrive* pVdrive);
		BOOL mount(xfs_blockdev* bdev, const char* mp_name = NULL);
		BOOL umount(const char* mp_name = NULL);

		//int file_dev_initialize(
		//	libbfio_handle_t** handle,
		//	libcerror_error_t** error);
		//
		//
		int file_dev_set_name_wide(
			libbfio_handle_t* file_range_io_handle,
			const wchar_t* name,
			size_t name_length,
			libcerror_error_t** error);
		
		//int file_dev_get(
		//	libbfio_handle_t* handle,
		//	off64_t* range_offset,
		//	size64_t* range_size,
		//	libcerror_error_t** error);

		int file_dev_set(
			libbfio_handle_t* handle,
			off64_t range_offset,
			size64_t range_size,
			libcerror_error_t** error);
	};

#ifdef __cplusplus
}
#endif

#endif /* !defined( _FDXFS_FILE_DEV_H ) */
