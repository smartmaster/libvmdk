/*
 * Offset table functions
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
#include <byte_stream.h>
#include <memory.h>
#include <types.h>

#include "libvmdk_definitions.h"
#include "libvmdk_grain_offset.h"
#include "libvmdk_libbfio.h"
#include "libvmdk_libcerror.h"
#include "libvmdk_libcnotify.h"
#include "libvmdk_offset_table.h"

/* Initialize the offset table
 * Returns 1 if successful or -1 on error
 */
int libvmdk_offset_table_initialize(
     libvmdk_offset_table_t **offset_table,
     uint32_t number_of_grain_offsets,
     libcerror_error_t **error )
{
	static char *function    = "libvmdk_offset_table_initialize";
	size_t grain_offset_size = 0;

	if( offset_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid offset table.",
		 function );

		return( -1 );
	}
	if( *offset_table != NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_ALREADY_SET,
		 "%s: invalid offset table value already set.",
		 function );

		return( -1 );
	}
	*offset_table = memory_allocate_structure(
	                 libvmdk_offset_table_t );

	if( *offset_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
		 "%s: unable to create offset table.",
		 function );

		goto on_error;
	}
	if( memory_set(
	     *offset_table,
	     0,
	     sizeof( libvmdk_offset_table_t ) ) == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_MEMORY,
		 LIBCERROR_MEMORY_ERROR_SET_FAILED,
		 "%s: unable to clear offset table.",
		 function );

		memory_free(
		 *offset_table );

		*offset_table = NULL;

		return( -1 );
	}
	if( number_of_grain_offsets > 0 )
	{
		grain_offset_size = sizeof( libvmdk_grain_offset_t ) * number_of_grain_offsets;

		if( grain_offset_size > (size_t) SSIZE_MAX )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
			 "%s: invalid grain offset size value exceeds maximum.",
			 function );

			goto on_error;
		}
		( *offset_table )->grain_offset = (libvmdk_grain_offset_t *) memory_allocate(
									      grain_offset_size );

		if( ( *offset_table )->grain_offset == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create grain offsets.",
			 function );

			goto on_error;
		}
		if( memory_set(
		     ( *offset_table )->grain_offset,
		     0,
		     grain_offset_size ) == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear grain offsets.",
			 function );

			goto on_error;
		}
	}
	( *offset_table )->number_of_grain_offsets = number_of_grain_offsets;

	return( 1 );

on_error:
	if( *offset_table != NULL )
	{
		if( ( *offset_table )->grain_offset != NULL )
		{
			memory_free(
			 ( *offset_table )->grain_offset );
		}
		memory_free(
		 *offset_table );

		*offset_table = NULL;
	}
	return( -1 );
}

/* Frees the offset table including elements
 * Returns 1 if successful or -1 on error
 */
int libvmdk_offset_table_free(
     libvmdk_offset_table_t **offset_table,
     libcerror_error_t **error )
{
	static char *function = "libvmdk_offset_table_free";

	if( offset_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid offset table.",
		 function );

		return( -1 );
	}
	if( *offset_table != NULL )
	{
		/* The segment file references are freed in the segment table
		 */
		if( ( *offset_table )->grain_offset != NULL )
		{
			memory_free(
			 ( *offset_table )->grain_offset );
		}
		memory_free(
		 *offset_table );

		*offset_table = NULL;
	}
	return( 1 );
}

/* Resizes the offset table
 * Returns 1 if successful or -1 on error
 */
int libvmdk_offset_table_resize(
     libvmdk_offset_table_t *offset_table,
     uint32_t number_of_grain_offsets,
     libcerror_error_t **error )
{
	void *reallocation       = NULL;
	static char *function    = "libvmdk_offset_table_resize";
	size_t grain_offset_size = 0;

	if( offset_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid offset_table.",
		 function );

		return( -1 );
	}
	if( offset_table->number_of_grain_offsets < number_of_grain_offsets )
	{
		grain_offset_size = sizeof( libvmdk_grain_offset_t ) * number_of_grain_offsets;

		if( grain_offset_size > (size_t) SSIZE_MAX )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
			 "%s: invalid grain offset size value exceeds maximum.",
			 function );

			return( -1 );
		}
		reallocation = memory_reallocate(
				offset_table->grain_offset,
				grain_offset_size );

		if( reallocation == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to resize grain offsets.",
			 function );

			return( -1 );
		}
		offset_table->grain_offset = (libvmdk_grain_offset_t *) reallocation;

		if( memory_set(
		     &( offset_table->grain_offset[ offset_table->number_of_grain_offsets ] ),
		     0,
		     sizeof( libvmdk_grain_offset_t ) * ( number_of_grain_offsets - offset_table->number_of_grain_offsets ) ) == NULL )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_MEMORY,
			 LIBCERROR_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear grain offsets.",
			 function );

			return( -1 );
		}
		offset_table->number_of_grain_offsets = number_of_grain_offsets;
	}
	return( 1 );
}

/* Fills the offset table from the table offsets
 * Returns 1 if successful or -1 on error
 */
int libvmdk_offset_table_fill(
     libvmdk_offset_table_t *offset_table,
     uint8_t *grain_table,
     size_t grain_table_size,
     uint32_t number_of_grain_table_entries,
     size_t grain_size,
     libcerror_error_t **error )
{
	libvmdk_grain_offset_t *grain_offset = NULL;
	static char *function                = "libvmdk_offset_table_fill";
	off64_t current_offset               = 0;
	size_t current_size                  = 0;
	uint32_t grain_table_entry_iterator  = 0;

	if( offset_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid offset table.",
		 function );

		return( -1 );
	}
	if( grain_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid grain table.",
		 function );

		return( -1 );
	}
	if( grain_table_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid grain table size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( ( grain_table_size % 4 ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: invalid grain table size not a multitude of 4.",
		 function );

		return( -1 );
	}
	if( number_of_grain_table_entries != ( grain_table_size / 4 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
		 "%s: invalid number of grain table entries size mismatch with calculated number.",
		 function );

		return( -1 );
	}
	if( grain_size == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_ZERO_OR_LESS,
		 "%s: invalid grain size value zero or less.",
		 function );

		return( -1 );
	}
	if( grain_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid grain size value exceeds maximum.",
		 function );

		return( -1 );
	}
	/* Allocate additional entries in the offset table if needed
	 * - a single reallocation saves processing time
	 */
	if( offset_table->number_of_grain_offsets < ( offset_table->last_grain_offset_filled + number_of_grain_table_entries ) )
	{
		if( libvmdk_offset_table_resize(
		     offset_table,
		     offset_table->last_grain_offset_filled + number_of_grain_table_entries,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_RESIZE_FAILED,
			 "%s: unable to resize offset table.",
			 function );

			return( -1 );
		}
	}
	for( grain_table_entry_iterator = 0;
	     grain_table_entry_iterator < number_of_grain_table_entries;
	     grain_table_entry_iterator++ )
	{
		byte_stream_copy_to_uint32_little_endian(
		 grain_table,
		 current_offset );

		grain_table += sizeof( uint32_t );

		current_offset *= LIBVMDK_SECTOR_SIZE;

		if( current_offset == 0 )
		{
			current_size = 0;
		}
		else
		{
			current_size = grain_size;
		}
#if defined( HAVE_VERBOSE_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			libcnotify_printf(
			 "%s: grain %" PRIu32 " read with offset 0x%08" PRIx64 " (%" PRIu64 ") and size %" PRIu32 ".\n",
			 function,
			 offset_table->last_grain_offset_filled,
			 current_offset,
			 current_offset,
			 current_size );
		}
#endif
		grain_offset = &( offset_table->grain_offset[ offset_table->last_grain_offset_filled ] );

		grain_offset->file_offset = current_offset;
		grain_offset->size        = current_size;

		offset_table->last_grain_offset_filled++;
	}
	return( 1 );
}

/* Compares the offsets with the ones in the offset table and makes corrections if necessary
 * Returns 1 if successful or -1 on error
 */
int libvmdk_offset_table_compare(
     libvmdk_offset_table_t *offset_table,
     uint8_t *grain_table,
     size_t grain_table_size,
     uint32_t number_of_grain_table_entries,
     size_t grain_size,
     libcerror_error_t **error )
{
#if defined( HAVE_VERBOSE_OUTPUT )
	char *remarks                        = NULL;
#endif

	libvmdk_grain_offset_t *grain_offset = NULL;
	static char *function                = "libvmdk_offset_table_compare";
	off64_t current_offset               = 0;
	size_t current_size                  = 0;
	uint32_t grain_table_entry_iterator  = 0;
	uint8_t mismatch                     = 0;

	if( offset_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid offset table.",
		 function );

		return( -1 );
	}
	if( grain_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid grain table.",
		 function );

		return( -1 );
	}
	if( grain_table_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid grain table size value exceeds maximum.",
		 function );

		return( -1 );
	}
	if( ( grain_table_size % 4 ) != 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: invalid grain table size not a multitude of 4.",
		 function );

		return( -1 );
	}
	if( number_of_grain_table_entries != ( grain_table_size / 4 ) )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
		 "%s: invalid number of grain table entries size mismatch with calculated number.",
		 function );

		return( -1 );
	}
	if( grain_size == 0 )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_ZERO_OR_LESS,
		 "%s: invalid grain size value zero or less.",
		 function );

		return( -1 );
	}
	if( grain_size > (size_t) SSIZE_MAX )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid grain size value exceeds maximum.",
		 function );

		return( -1 );
	}
	/* Allocate additional entries in the offset table if needed
	 * - a single reallocation saves processing time
	 */
	if( offset_table->number_of_grain_offsets < ( offset_table->last_grain_offset_compared + number_of_grain_table_entries ) )
	{
		if( libvmdk_offset_table_resize(
		     offset_table,
		     offset_table->last_grain_offset_compared + number_of_grain_table_entries,
		     error ) != 1 )
		{
			libcerror_error_set(
			 error,
			 LIBCERROR_ERROR_DOMAIN_RUNTIME,
			 LIBCERROR_RUNTIME_ERROR_RESIZE_FAILED,
			 "%s: unable to resize offset table.",
			 function );

			return( -1 );
		}
	}
	for( grain_table_entry_iterator = 0;
	     grain_table_entry_iterator < number_of_grain_table_entries;
	     grain_table_entry_iterator++ )
	{
		byte_stream_copy_to_uint32_little_endian(
		 grain_table,
		 current_offset );

		grain_table += sizeof( uint32_t );

		current_offset *= LIBVMDK_SECTOR_SIZE;

		if( current_offset == 0 )
		{
			current_size = 0;
		}
		else
		{
			current_size = grain_size;
		}
		grain_offset = &( offset_table->grain_offset[ offset_table->last_grain_offset_compared ] );

		if( grain_offset->file_offset != current_offset )
		{
#if defined( HAVE_VERBOSE_OUTPUT )
			if( libcnotify_verbose != 0 )
			{
				libcnotify_printf(
				 "%s: file offset mismatch for grain offset: %" PRIu32 ".\n",
				 function,
				 offset_table->last_grain_offset_compared );
			}
#endif

			mismatch = 1;
		}
		else
		{
			mismatch = 0;
		}
#if defined( HAVE_VERBOSE_OUTPUT )
		if( libcnotify_verbose != 0 )
		{
			if( mismatch == 1 )
			{
				remarks = " corrupted";
			}
			else
			{
				remarks = "";
			}
			libcnotify_printf(
			 "%s: grain %" PRIu32 " read with offset 0x%08" PRIu64 " (%" PRIu64 ") and size %" PRIu32 "%s.\n",
			 function,
			 offset_table->last_grain_offset_compared,
			 current_offset,
			 current_offset,
			 grain_size,
			 remarks );
		}
#endif
		if( mismatch == 1 )
		{
			grain_offset->flags &= ~ ( LIBVMDK_GRAIN_OFFSET_FLAGS_CORRUPTED );
		}
		offset_table->last_grain_offset_compared++;
	}
	return( 1 );
}

/* Seeks a certain grain offset within the offset table
 * Returns the grain segment file offset if the seek is successful or -1 on error
 */
off64_t libvmdk_offset_table_seek_grain_offset(
         libvmdk_offset_table_t *offset_table,
         uint32_t grain,
         libcerror_error_t **error )
{
	static char *function = "libvmdk_segment_table_seek_grain_offset";

	if( offset_table == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid offset table.",
		 function );

		return( -1 );
	}
	if( offset_table->grain_offset == NULL )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_RUNTIME,
		 LIBCERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid offset table - missing grain offsets.",
		 function );

		return( -1 );
	}
	if( grain >= offset_table->number_of_grain_offsets )
	{
		libcerror_error_set(
		 error,
		 LIBCERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBCERROR_ARGUMENT_ERROR_VALUE_OUT_OF_RANGE,
		 "%s: grain: %" PRIu32 " out of range [0,%" PRIu32 "].",
		 function,
		 grain,
		 offset_table->number_of_grain_offsets - 1 );

		return( -1 );
	}
/* TODO */

	return( offset_table->grain_offset[ grain ].file_offset );
}

