/*
 * Handle functions
 *
 * Copyright (c) 2009-2012, Joachim Metz <joachim.metz@gmail.com>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include "libvmdk_debug.h"
#include "libvmdk_definitions.h"
#include "libvmdk_descriptor_file.h"
#include "libvmdk_extent_file.h"
#include "libvmdk_handle.h"
#include "libvmdk_io_handle.h"
#include "libvmdk_libbfio.h"
#include "libvmdk_libcerror.h"
#include "libvmdk_libcnotify.h"
#include "libvmdk_libcstring.h"
#include "libvmdk_offset_table.h"

#include "libvmdk_segment_table.h"

/* Initialize a handle
 * Make sure the value handle is pointing to is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libvmdk_handle_initialize(
     libvmdk_handle_t **handle,
     libcerror_error_t **error )
{
	libvmdk_internal_handle_t *internal_handle = NULL;
	static char *function                      = "libvmdk_handle_initialize";

	if( handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.",
		 function );

		return( -1 );
	}
	if( *handle != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid handle value already set.",
		 function );

		return( -1 );
	}
	internal_handle = memory_allocate_structure(
	                   libvmdk_internal_handle_t );

	if( internal_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create handle.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     internal_handle,
	     0,
	     sizeof( libvmdk_internal_handle_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear handle.",
		 function );

		memory_free(
		 internal_handle );

		return( -1 );
	}
	/* The segment table is initially filled with a single entry
	 */
	if( libvmdk_segment_table_initialize(
	     &( internal_handle->segment_table ),
	     1,
	     LIBVMDK_DEFAULT_SEGMENT_FILE_SIZE,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create segment table.",
		 function );

		goto on_error;
	}
	if( libvmdk_offset_table_initialize(
	     &( internal_handle->offset_table ),
	     0,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create offset table.",
		 function );

		goto on_error;
	}
	if( libvmdk_io_handle_initialize(
	     &( internal_handle->io_handle ),
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create io handle.",
		 function );

		goto on_error;
	}
	internal_handle->maximum_number_of_open_handles = LIBBFIO_POOL_UNLIMITED_NUMBER_OF_OPEN_HANDLES;

	*handle = (libvmdk_handle_t *) internal_handle;

	return( 1 );

on_error:
	if( internal_handle != NULL )
	{
		if( internal_handle->offset_table != NULL )
		{
			libvmdk_offset_table_free(
			 &( internal_handle->offset_table ),
			 NULL );
		}
		if( internal_handle->segment_table != NULL )
		{
			libvmdk_segment_table_free(
			 &( internal_handle->segment_table ),
			 NULL );
		}
		memory_free(
		 internal_handle );
	}
	return( -1 );
}

/* Frees an exisisting handle
 * Returns 1 if successful or -1 on error
 */
int libvmdk_handle_free(
     libvmdk_handle_t **handle,
     libcerror_error_t **error )
{
	libvmdk_internal_handle_t *internal_handle = NULL;
	static char *function                      = "libvmdk_handle_free";
	int result                                 = 1;

	if( handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.",
		 function );

		return( -1 );
	}
	if( *handle != NULL )
	{
		internal_handle = (libvmdk_internal_handle_t *) *handle;
		*handle         = NULL;

		if( internal_handle->io_handle != NULL )
		{
			if( libvmdk_io_handle_free(
			     &( internal_handle->io_handle ),
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free io handle.",
				 function );

				result = -1;
			}
		}
		if( internal_handle->file_io_pool_created_in_library != 0 )
		{
			if( internal_handle->file_io_pool != NULL )
			{
				if( libbfio_pool_free(
				     &( internal_handle->file_io_pool ),
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
					 "%s: unable to free file IO pool.",
					 function );

					result = -1;
				}
			}
		}
		if( libvmdk_segment_table_free(
		     &( internal_handle->segment_table ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free segment table.",
			 function );

			result = -1;
		}
		if( libvmdk_offset_table_free(
		     &( internal_handle->offset_table ),
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free offset table.",
			 function );

			result = -1;
		}
		memory_free(
		 internal_handle );
	}
	return( result );
}

/* Signals the libvmdk handle to abort its current activity
 * Returns 1 if successful or -1 on error
 */
int libvmdk_handle_signal_abort(
     libvmdk_handle_t *handle,
     libcerror_error_t **error )
{
	static char *function = "libvmdk_handle_signal_abort";

	if( handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.",
		 function );

		return( -1 );
	}
	( (libvmdk_internal_handle_t *) handle )->abort = 1;

	return( 1 );
}

/* Opens a set of Mware Virtual Disk file(s)
 * Returns 1 if successful or -1 on error
 */
int libvmdk_handle_open(
     libvmdk_handle_t *handle,
     char * const filenames[],
     int number_of_filenames,
     int flags,
     libcerror_error_t **error )
{
	libbfio_handle_t *file_io_handle           = NULL;
	libbfio_pool_t *file_io_pool               = NULL;
	libvmdk_internal_handle_t *internal_handle = NULL;
	static char *function                      = "libvmdk_handle_open";
	size_t filename_length                     = 0;
	int file_io_pool_entry                     = 0;
	int filename_index                         = 0;

	if( handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.",
		 function );

		return( -1 );
	}
	internal_handle = (libvmdk_internal_handle_t *) handle;

	if( filenames == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filenames.",
		 function );

		return( -1 );
	}
	if( number_of_filenames <= 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_ZERO_OR_LESS,
		 "%s: invalid number of files zero or less.",
		 function );

		return( -1 );
	}
	if( ( flags & LIBVMDK_ACCESS_FLAG_WRITE ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: write access currently not supported.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_initialize(
	     &file_io_pool,
	     0,
	     internal_handle->maximum_number_of_open_handles,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file IO pool.",
		 function );

		goto on_error;
	}
	if( ( flags & LIBVMDK_ACCESS_FLAG_READ ) != 0 )
	{
		for( filename_index = 0;
		     filename_index < number_of_filenames;
		     filename_index++ )
		{
			filename_length = libcstring_narrow_string_length(
					   filenames[ filename_index ] );

			/* Make sure there is more to the filename than the extension
			 */
			if( filename_length <= 4 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
				 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
				 "%s: filename: %s is too small.",
				 function,
				 filenames[ filename_index ] );

				goto on_error;
			}
			if( libbfio_file_initialize(
			     &file_io_handle,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create file IO handle.",
				 function );

				goto on_error;
			}
#if defined( HAVE_DEBUG_OUTPUT )
			if( libbfio_handle_set_track_offsets_read(
			     file_io_handle,
			     1,
			     error ) != 1 )
			{
		                libcerror_error_set(
		                 error,
		                 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		                 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		                 "%s: unable to set track offsets read in file IO handle.",
		                 function );

				goto on_error;
			}
#endif
			if( libbfio_file_set_name(
			     file_io_handle,
			     filenames[ filename_index ],
			     filename_length + 1,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to set name in file IO handle.",
				 function );

				goto on_error;
			}
			if( libbfio_pool_add_handle(
			     file_io_pool,
			     &file_io_pool_entry,
			     file_io_handle,
			     LIBBFIO_OPEN_READ,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to add file IO handle to pool.",
				 function );

				goto on_error;
			}
			file_io_handle = NULL;

#if defined( HAVE_VERBOSE_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: added pool entry: %d with filename: %s.\n",
				 function,
				 file_io_pool_entry,
				 filenames[ filename_index ] );
			}
#endif
		}
	}
	if( libvmdk_handle_open_file_io_pool(
	     handle,
	     file_io_pool,
	     flags,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open handle using a file IO pool.",
		 function );

		goto on_error;
	}
	internal_handle->file_io_pool_created_in_library = 1;

	return( 1 );

on_error:
	if( file_io_handle != NULL )
	{
		libbfio_handle_free(
		 &file_io_handle,
		 NULL );
	}
	if( file_io_pool != NULL )
	{
		libbfio_pool_free(
		 &file_io_pool,
		 NULL );
	}
	return( -1 );
}

#if defined( HAVE_WIDE_CHARACTER_TYPE )

/* Opens a set of VMware Virtual Disk file(s)
 * Returns 1 if successful or -1 on error
 */
int libvmdk_handle_open_wide(
     libvmdk_handle_t *file,
     const wchar_t *filename,
     int flags,
     libcerror_error_t **error )
{
	libbfio_handle_t *file_io_handle           = NULL;
	libbfio_pool_t *file_io_pool               = NULL;
	libvmdk_internal_handle_t *internal_handle = NULL;
	static char *function                      = "libvmdk_handle_open_wide";
	size_t filename_length                     = 0;
	int file_io_pool_entry                     = 0;
	int filename_index                         = 0;

	if( handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.",
		 function );

		return( -1 );
	}
	internal_handle = (libvmdk_internal_handle_t *) handle;

	if( filenames == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid filenames.",
		 function );

		return( -1 );
	}
	if( number_of_filenames <= 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_ZERO_OR_LESS,
		 "%s: invalid number of files zero or less.",
		 function );

		return( -1 );
	}
	if( ( flags & LIBVMDK_ACCESS_FLAG_WRITE ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: write access currently not supported.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_initialize(
	     &file_io_pool,
	     0,
	     internal_handle->maximum_number_of_open_handles,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create file IO pool.",
		 function );

		goto on_error;
	}
	if( ( flags & LIBVMDK_ACCESS_FLAG_READ ) != 0 )
	{
		for( filename_index = 0;
		     filename_index < number_of_filenames;
		     filename_index++ )
		{
			filename_length = libcstring_wide_string_length(
					   filenames[ filename_index ] );

			/* Make sure there is more to the filename than the extension
			 */
			if( filename_length <= 4 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
				 LIBCERROR_ARGUMENT_ERROR_VALUE_TOO_SMALL,
				 "%s: filename: %ls is too small.",
				 function,
				 filenames[ filename_index ] );

				goto on_error;
			}
			if( libbfio_file_initialize(
			     &file_io_handle,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create file IO handle.",
				 function );

				goto on_error;
			}
#if defined( HAVE_DEBUG_OUTPUT )
			if( libbfio_handle_set_track_offsets_read(
			     file_io_handle,
			     1,
			     error ) != 1 )
			{
		                libcerror_error_set(
		                 error,
		                 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		                 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
		                 "%s: unable to set track offsets read in file IO handle.",
		                 function );

				goto on_error;
			}
#endif
			if( libbfio_file_set_name_wide(
			     file_io_handle,
			     filenames[ filename_index ],
			     filename_length + 1,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
				 "%s: unable to set name in file IO handle.",
				 function );

				goto on_error;
			}
			if( libbfio_pool_add_handle(
			     file_io_pool,
			     &file_io_pool_entry,
			     file_io_handle,
			     LIBBFIO_OPEN_READ,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to add file IO handle to pool.",
				 function );

				goto on_error;
			}
			file_io_handle = NULL;

#if defined( HAVE_VERBOSE_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: added pool entry: %d with filename: %ls.\n",
				 function,
				 file_io_pool_entry,
				 filenames[ filename_index ] );
			}
#endif
		}
	}
	if( libvmdk_handle_open_file_io_pool(
	     handle,
	     file_io_pool,
	     flags,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_OPEN_FAILED,
		 "%s: unable to open handle using a file IO pool.",
		 function );

		goto on_error;
	}
	internal_handle->file_io_pool_created_in_library = 1;

	return( 1 );

on_error:
	if( file_io_handle != NULL )
	{
		libbfio_handle_free(
		 &file_io_handle,
		 NULL );
	}
	if( file_io_pool != NULL )
	{
		libbfio_pool_free(
		 &file_io_pool,
		 NULL );
	}
	return( -1 );
}

#endif

/* Opens a set of VMware Virtual Disk file(s) using a Basic File IO (bfio) pool
 * Returns 1 if successful or -1 on error
 */
int libvmdk_handle_open_file_io_pool(
     libvmdk_handle_t *handle,
     libbfio_handle_t *file_io_pool,
     int flags,
     libcerror_error_t **error )
{
	libbfio_handle_t *file_io_handle           = NULL;
	libvmdk_internal_handle_t *internal_handle = NULL;
	libvmdk_extent_file_t *extent_file         = NULL;
	static char *function                      = "libvmdk_handle_open_file_io_pool";
	uint8_t file_type                          = 0;
	int number_of_file_io_handles              = 0;
	int file_io_pool_entry                     = 0;
	int result                                 = 0;

	if( handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.",
		 function );

		return( -1 );
	}
	internal_handle = (libvmdk_internal_handle_t *) handle;

	if( internal_handle->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid handle - missing io handle.",
		 function );

		return( -1 );
	}
	if( file_io_pool == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file IO pool.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_get_number_of_handles(
	     file_io_pool,
	     &number_of_file_io_handles,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve the number of handles in the file IO pool.",
		 function );

		return( -1 );
	}
	if( ( ( flags & LIBVMDK_ACCESS_FLAG_READ ) == 0 )
	 && ( ( flags & LIBVMDK_ACCESS_FLAG_WRITE ) == 0 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported flags.",
		 function );

		goto on_error;
	}
	internal_handle->io_handle->flags = flags;
	internal_handle->file_io_pool     = file_io_pool;

	if( ( flags & LIBVMDK_ACCESS_FLAG_READ ) != 0 )
	{
		for( file_io_pool_entry = 0;
		     file_io_pool_entry < number_of_file_io_handles;
		     file_io_pool_entry++ )
		{
			file_io_handle = NULL;

			if( libbfio_pool_get_handle(
			     file_io_pool,
			     file_io_pool_entry,
			     &file_io_handle,
			     error ) != 1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_RUNTIME,
				 LIBCERROR_RUNTIME_ERROR_APPEND_FAILED,
				 "%s: unable to retrieve file IO handle from pool entry: %d.",
				 function,
				 file_io_pool_entry );

				goto on_error;
			}
#if defined( HAVE_VERBOSE_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: processing pool entry: %d.\n",
				 function,
				 file_io_pool_entry );
			}
#endif
			result = libvmdk_handle_open_read_signature(
			          file_io_pool,
			          file_io_pool_entry,
			          &file_type,
			          error );

			if( result == -1 )
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_IO,
				 LIBCERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read signature of pool entry: %d.",
				 function,
				 file_io_pool_entry );

				goto on_error;
			}
			if( file_type == LIBVMDK_FILE_TYPE_DESCRIPTOR_FILE )
			{
/* TODO check if already set */
			}
			else if( file_type == LIBVMDK_FILE_TYPE_RAW_DATA )
			{
			}
			else if( ( file_type == LIBVMDK_FILE_TYPE_COWD_SPARSE_DATA )
			      || ( file_type == LIBVMDK_FILE_TYPE_VMDK_SPARSE_DATA ) )
			{
				if( libvmdk_extent_file_initialize(
				     &extent_file,
				     file_io_pool_entry,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					 "%s: unable to create extent file.",
					 function );

					goto on_error;
				}
				if( libvmdk_extent_file_read_header(
				     extent_file,
				     file_io_pool,
				     error ) <= -1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_IO,
					 LIBCERROR_IO_ERROR_READ_FAILED,
					 "%s: unable to read extent file header.",
					 function );

					goto on_error;
				}
				if( libvmdk_segment_table_set_handle(
				     internal_handle->segment_table,
				     /* TODO */ 0,
				     extent_file,
				     error ) != 1 )
				{
					libcerror_error_set(
					 error,
					 LIBCERROR_ERROR_DOMAIN_RUNTIME,
					 LIBCERROR_RUNTIME_ERROR_SET_FAILED,
					 "%s: unable to set handle in segment table.",
					 function );

					goto on_error;
				}
				extent_file = NULL;
			}
			else
			{
				libcerror_error_set(
				 error,
				 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
				 LIBCERROR_ARGUMENT_ERROR_UNSUPPORTED_VALUE,
				 "%s: unsupported file type.",
				 function );

				goto on_error;
			}
		}
		result = libvmdk_handle_open_read(
		          internal_handle,
		          internal_handle->segment_table,
		          error );

		if( result != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read segment table.",
			 function );

			goto on_error;
		}
	}
	return( 1 );

on_error:
	if( extent_file != NULL )
	{
		libvmdk_extent_file_free(
		 &extent_file,
		 NULL );
	}
	internal_handle->file_io_pool = NULL;

	return( -1 );
}

/* Closes a VMware Virtual Disk file(s)
 * Returns 0 if successful or -1 on error
 */
int libvmdk_handle_close(
     libvmdk_handle_t *handle,
     libcerror_error_t **error )
{
	libvmdk_internal_handle_t *internal_handle = NULL;
	static char *function                      = "libvmdk_handle_close";

	if( handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid handle.",
		 function );

		return( -1 );
	}
	internal_handle = (libvmdk_internal_handle_t *) handle;

	if( internal_handle->file_io_pool_created_in_library != 0 )
	{
		if( libbfio_pool_close_all(
		     internal_handle->file_io_pool,
		     error ) != 0 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_CLOSE_FAILED,
			 "%s: unable to close all segment files.",
			 function );

			return( -1 );
		}
	}
	return( 0 );
}

/* Opens VMware Virtual Disk file(s) for reading
 * Returns 1 if successful or -1 on error
 */
int libvmdk_handle_open_read(
     libvmdk_internal_handle_t *internal_handle,
     libvmdk_segment_table_t *segment_table,
     libcerror_error_t **error )
{
	libvmdk_extent_file_t *extent_file = NULL;
	static char *function              = "libvmdk_handle_open_read";
	int number_of_extent_files         = 0;
	int segment_number                 = 0;

	if( internal_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid internal handle.",
		 function );

		return( -1 );
	}
	if( libvmdk_segment_table_get_number_of_handles(
	     segment_table,
	     &number_of_extent_files,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve number of handles in segment table.",
		 function );

		return( -1 );
	}
	/* Read the segment and offset table from the segment file(s)
	 */
	for( segment_number = 1;
	     segment_number < number_of_extent_files;
	     segment_number++ )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: reading section list for segment number: %d.\n",
			 function,
			 segment_number );
		}
#endif
		extent_file = NULL;

		if( libvmdk_segment_table_get_handle(
		     segment_table,
		     segment_number,
		     &extent_file,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve handle: %d from segment table.",
			 function,
			 segment_number );

			return( -1 );
		}
/* TODO */

		if( internal_handle->abort == 1 )
		{
			return( -1 );
		}
	}
	return( 1 );
}

/* Reads the file signature and tries to determine the file type
 * Returns 1 if successful, 0 if no file type could be determined or -1 on error
 */
int libvmdk_handle_open_read_signature(
     libbfio_pool_t *file_io_pool,
     int file_io_pool_entry,
     uint8_t *file_type,
     libcerror_error_t **error )
{
	uint8_t signature[ 32 ];

	static char *function = "libvmdk_handle_open_read_signature";
	ssize_t read_count    = 0;
	int result            = 0;

	if( file_type == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid file type.",
		 function );

		return( -1 );
	}
	if( libbfio_pool_seek_offset(
	     file_io_pool,
	     file_io_pool_entry,
	     0,
	     SEEK_SET,
	     error ) == -1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_SEEK_FAILED,
		 "%s: unable to seek offset: 0 in file IO pool entry: %d.",
		 function,
		 file_io_pool_entry );

		goto on_error;
	}
	read_count = libbfio_pool_read(
	              file_io_pool,
	              file_io_pool_entry,
	              signature,
	              32,
	              error );

	if( read_count != (ssize_t) 32 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read signature of file IO pool entry: %d.",
		 function,
		 file_io_pool_entry );

		return( -1 );
	}
	if( memory_compare(
	     signature,
	     cowd_sparse_file_signature,
	     4 ) == 0 )
	{
		*file_type = LIBVMDK_FILE_TYPE_COWD_SPARSE_DATA;
		result     = 1;
	}
	else if( memory_compare(
	          signature,
	          vmdk_sparse_file_signature,
	          4 ) == 0 )
	{
		*file_type = LIBVMDK_FILE_TYPE_VMDK_SPARSE_DATA;
		result     = 1;
	}
	else if( memory_compare(
	          signature,
	          vmdk_descriptor_file_signature,
	          21 ) == 0 )
	{
		if( signature[ 21 ] == (uint8_t) '\n' )
		{
			*file_type = LIBVMDK_FILE_TYPE_DESCRIPTOR_FILE;
			result     = 1;
		}
	}
	return( result );
}

#ifdef TODO

/* Opens a VMware Virtual Disk file(s) for reading
 * Returns 1 if successful or -1 on error
 */
int libvmdk_handle_open_read(
     libvmdk_internal_handle_t *internal_handle,
     libcerror_error_t **error )
{
	static char *function                      = "libvmdk_handle_open_read";
	off64_t descriptor_offset                  = 0;
	off64_t grain_directory_offset             = 0;
	off64_t secondary_grain_directory_offset   = 0;
	size64_t descriptor_size                   = 0;
	size64_t grain_size                        = 0;
	uint32_t number_of_grain_directory_entries = 0;
	uint32_t number_of_grain_table_entries     = 0;

	if( internal_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid internal file.",
		 function );

		return( -1 );
	}
	if( internal_handle->io_handle == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid internal file - missing io handle.",
		 function );

		return( -1 );
	}
#if defined( HAVE_VERBOSE_OUTPUT )
	if( libcnotify_verbose != 0 )
	{
		libcnotify_printf(
		 "Reading file header:\n" );
	}
#endif
	if( libvmdk_io_handle_read_file_header(
	     internal_handle->io_handle,
	     internal_handle->file_io_handle,
	     &descriptor_offset,
	     &descriptor_size,
	     &grain_directory_offset,
	     &secondary_grain_directory_offset,
	     &number_of_grain_directory_entries,
	     &number_of_grain_table_entries,
	     &grain_size,
	     error ) != 1 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_IO,
		 LIBCERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read file header.",
		 function );

		return( -1 );
	}
	if( descriptor_offset > 0 )
	{
		if( descriptor_size == 0 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
			 "%s: invalid descriptor size value is out of range.",
			 function );

			return( -1 );
		}
#if defined( HAVE_VERBOSE_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "Reading descriptor:\n" );
		}
#endif

		/* TODO read descriptor */
	}
	if( ( grain_directory_offset == 0 )
	 && ( secondary_grain_directory_offset == 0 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
		 "%s: invalid primary and secondary grain directory offset value out of range.",
		 function );

		return( -1 );
	}
	if( grain_directory_offset > 0 )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "Reading (primary) grain directory:\n" );
		}
#endif

		if( libvmdk_io_handle_read_grain_directory(
		     internal_handle->io_handle,
		     internal_handle->file_io_handle,
		     internal_handle->offset_table,
		     grain_directory_offset,
		     number_of_grain_directory_entries,
		     number_of_grain_table_entries,
		     grain_size,
		     0,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read (primary) grain directory.",
			 function );

			return( -1 );
		}
	}
	if( secondary_grain_directory_offset > 0 )
	{
#if defined( HAVE_VERBOSE_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "Reading secondary grain directory:\n" );
		}
#endif

		if( libvmdk_io_handle_read_grain_directory(
		     internal_handle->io_handle,
		     internal_handle->file_io_handle,
		     internal_handle->offset_table,
		     secondary_grain_directory_offset,
		     number_of_grain_directory_entries,
		     number_of_grain_table_entries,
		     grain_size,
		     1,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_IO,
			 LIBCERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read secondary grain directory.",
			 function );

			return( -1 );
		}
	}
	return( 1 );
}

#endif

