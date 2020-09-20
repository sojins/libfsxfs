/*
 * B+ tree block functions
 *
 * Copyright (C) 2020, Joachim Metz <joachim.metz@gmail.com>
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

#include <common.h>
#include <byte_stream.h>
#include <memory.h>
#include <narrow_string.h>
#include <system_string.h>
#include <types.h>
#include <wide_string.h>

#include "libfsxfs_btree_block.h"
#include "libfsxfs_io_handle.h"
#include "libfsxfs_libbfio.h"
#include "libfsxfs_libcerror.h"
#include "libfsxfs_libcnotify.h"

#include "fsxfs_btree.h"

/* Creates a btree_block
 * Make sure the value btree_block is referencing, is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_btree_block_initialize(
     libfsxfs_btree_block_t **btree_block,
     libcerror_error_t **error )
{
	static char *function = "libfsxfs_btree_block_initialize";

	if( btree_block == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid B+ tree block.",
		 function );

		return( -1 );
	}
	if( *btree_block != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid B+ tree block value already set.",
		 function );

		return( -1 );
	}
	*btree_block = memory_allocate_structure(
	                 libfsxfs_btree_block_t );

	if( *btree_block == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create B+ tree block.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     *btree_block,
	     0,
	     sizeof( libfsxfs_btree_block_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear B+ tree block.",
		 function );

		goto on_error;
	}
	return( 1 );

on_error:
	if( *btree_block != NULL )
	{
		memory_free(
		 *btree_block );

		*btree_block = NULL;
	}
	return( -1 );
}

/* Frees a btree_block
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_btree_block_free(
     libfsxfs_btree_block_t **btree_block,
     libcerror_error_t **error )
{
	static char *function = "libfsxfs_btree_block_free";
	int result            = 1;

	if( btree_block == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid B+ tree block.",
		 function );

		return( -1 );
	}
	if( *btree_block != NULL )
	{
		if( ( *btree_block )->header != NULL )
		{
			if( libfsxfs_btree_header_free(
			     &( ( *btree_block )->header ),
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free B+ tree header.",
				 function );

				result = -1;
			}
		}
		memory_free(
		 *btree_block );

		*btree_block = NULL;
	}
	return( result );
}

/* Reads the btree_block data
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_btree_block_read_data(
     libfsxfs_btree_block_t *btree_block,
     const uint8_t *data,
     size_t data_size,
     libcerror_error_t **error )
{
	static char *function    = "libfsxfs_btree_block_read_data";
	size_t records_data_size = 0;

	if( btree_block == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid B+ tree block.",
		 function );

		return( -1 );
	}
	if( btree_block->header != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid B+ tree block - header value already set.",
		 function );

		return( -1 );
	}
	if( data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid data.",
		 function );

		return( -1 );
	}
	if( ( data_size < sizeof( fsxfs_btree_header_t ) )
	 || ( data_size > (size_t) SSIZE_MAX ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid data size value out of bounds.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: B+ tree block data:\n",
		 function );
		libcnotify_print_data(
		 data,
		 data_size,
		 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
	}
#endif /* defined( HAVE_DEBUG_OUTPUT ) */

	if( libfsxfs_btree_header_initialize(
	     &( btree_block->header ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create B+ tree header.",
		 function );

		goto on_error;
	}
	if( libfsxfs_btree_header_read_data(
	     btree_block->header,
	     data,
	     sizeof( fsxfs_btree_header_t ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read B+ tree header.",
		 function );

		goto on_error;
	}
	if( (size_t) btree_block->header->number_of_records > ( ( data_size - ( sizeof( fsxfs_btree_header_t ) + 40 ) ) / 16 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid data size value out of bounds.",
		 function );

		goto on_error;
	}
	records_data_size = 16 * btree_block->header->number_of_records;

#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: B+ tree records data:\n",
		 function );
		libcnotify_print_data(
		 &( data[ sizeof( fsxfs_btree_header_t ) + 40 ] ),
		 records_data_size,
		 LIBCNOTIFY_PRINT_DATA_FLAG_GROUP_DATA );
	}
#endif /* defined( HAVE_DEBUG_OUTPUT ) */

/* TODO read records */

	return( 1 );

on_error:
	if( btree_block->header != NULL )
	{
		libfsxfs_btree_header_free(
		 &( btree_block->header ),
		 NULL );
	}
	return( -1 );
}

/* Reads the B+ tree block from a Basic File IO (bfio) handle
 * Returns 1 if successful or -1 on error
 */
int libfsxfs_btree_block_read_file_io_handle(
     libfsxfs_btree_block_t *btree_block,
     libfsxfs_io_handle_t *io_handle,
     libbfio_handle_t *file_io_handle,
     off64_t file_offset,
     libcerror_error_t **error )
{
	uint8_t *block_data   = NULL;
	static char *function = "libfsxfs_btree_block_read_file_io_handle";
	ssize_t read_count    = 0;

	if( io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid IO handle.",
		 function );

		return( -1 );
	}
	if( ( io_handle->block_size == 0 )
	 || ( io_handle->block_size > (size_t) MEMORY_MAXIMUM_ALLOCATION_SIZE ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: invalid IO handle - block size value out of bounds.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "%s: reading B+ tree block at offset: %" PRIi64 " (0x%08" PRIx64 ")\n",
		 function,
		 file_offset,
		 file_offset );
	}
#endif
	if( libbfio_handle_seek_offset(
	     file_io_handle,
	     file_offset,
	     SEEK_SET,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek B+ tree block offset: %" PRIi64 " (0x%08" PRIx64 ").",
		 function,
		 file_offset,
		 file_offset );

		goto on_error;
	}
	block_data = (uint8_t *) memory_allocate(
	                          sizeof( uint8_t ) * io_handle->block_size );

	if( block_data == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create block data.",
		 function );

		goto on_error;
	}
	read_count = libbfio_handle_read_buffer(
	              file_io_handle,
	              block_data,
	              io_handle->block_size,
	              error );

	if( read_count != (ssize_t) io_handle->block_size )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read B+ tree block at offset: %" PRIi64 " (0x%08" PRIx64 ").",
		 function,
		 file_offset,
		 file_offset );

		goto on_error;
	}
	if( libfsxfs_btree_block_read_data(
	     btree_block,
	     block_data,
	     io_handle->block_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read B+ tree block at offset: %" PRIi64 " (0x%08" PRIx64 ").",
		 function,
		 file_offset,
		 file_offset );

		goto on_error;
	}
	memory_free(
	 block_data );

	return( 1 );

on_error:
	if( block_data != NULL )
	{
		memory_free(
		 block_data );
	}
	return( -1 );
}

