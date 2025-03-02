/**********************************************************************/
/** @addtogroup embedded_gcov
 * @{
 * @file
 * @version $Id: $
 *
 * @author 2021-08-31 kjpeters  Working and cleaned up version.
 * @author 2022-01-07 kjpeters  Adjust gcc releases for GCOV_COUNTERS.
 *
 * @note Based on GCOV-related code of the Linux kernel,
 * as described online by Thanassis Tsiodras (April 2016)
 * https://www.thanassis.space/coverage.html
 * and by Alexander Tarasikov
 * http://allsoftwaresucks.blogspot.com/2015/05/gcov-is-amazing-yet-undocumented.html
 * with additional investigation, updating, cleanup, and portability
 * by Ken Peters.
 *
 * @brief Private header file for embedded gcov.
 *
 **********************************************************************/
/*
 * This code is based on the GCOV-related code of the Linux kernel (kept under
 * "kernel/gcov"). It basically uses the convert_to_gcda function to generate
 * the .gcda files information upon application completion, and dump it on the
 * host filesystem via GDB scripting.
 *
 * Original Linux banner follows below - but note that the Linux guys have
 * nothing to do with these modifications, so blame me (and contact me)
 * if something goes wrong.
 *
 * Thanassis Tsiodras
 * Real-time Embedded Software Engineer
 * System, Software and Technology Department
 * European Space Agency
 *
 * e-mail: ttsiodras@gmail.com / Thanassis.Tsiodras@esa.int (work)
 *
 *  This file is based on gcc-internal definitions. Data structures are
 *  defined to be compatible with gcc counterparts. For a better
 *  understanding, refer to gcc source: gcc/gcov-io.h.
 *
 *    Copyright IBM Corp. 2009
 *    Author(s): Peter Oberparleiter <oberpar@linux.vnet.ibm.com>
 *
 *    Uses gcc-internal data definitions.
 */
/* Copyright (c) 2021 California Institute of Technology (“Caltech”).
 * U.S. Government sponsorship acknowledged.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *    Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *    Redistributions in binary form must reproduce the above copyright notice,
 *        this list of conditions and the following disclaimer in the documentation
 *        and/or other materials provided with the distribution.
 *    Neither the name of Caltech nor its operating division, the Jet Propulsion Laboratory,
 *        nor the names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <fcntl.h>    // \TODO: linux only header file to be removed for embedded
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    //\TODO: linux only header file to be removed for embedded

#include "gcov/gcov.h"

#include <sys/stat.h>    // \TODO: linux only header file to be removed for embedded

//! Type of function used to merge counters, compare to libgcc/libgcov.h
typedef void (*gcov_merge_fn)(gcov_type*, gcov_unsigned_t);

typedef struct gcov_info_tag gcov_info_tag;

/*! \brief A struct with a list of coverage data for each file
 */
struct gcov_info_tag {
    struct gcov_info* info;        //!< The data belonging to the file
    struct gcov_info_tag* next;    //!< Pointer to the info of the next file
};

/*! \brief Information about counters of a single function
 *
 *  \details Contains information about counters for a single function.
 *  The info is generated at compile time, only the values in the array
 *  change at run-time.
 *  \note Compare this struct to the one in libgcc/libgcov.h
 */
struct gcov_ctr_info {
    gcov_unsigned_t num;    //!< The number of counter values for this type
    gcov_type* values;      //!< The array of counter values for this type
};

/*! \brief Describes the profiling meta data per function
 *
 *  \details Contains information about a single function. The number of counters
 *  is determined from the merge pointer array in gcov_info. The key is used to
 *  detect which of a set of comdat functions was selected. It points to the gcov_info
 *  object of the object file containing the selected comdat function.
 *  \note Compare this struct to the one in libgcc/libgcov.h
 */
struct gcov_fn_info {
    const struct gcov_info* key;        //!< Comdat key
    gcov_unsigned_t ident;              //!< Unique identifier of the function
    gcov_unsigned_t lineno_checksum;    //!< Functin line number checksum
    gcov_unsigned_t cfg_checksum;       //!< Function configuration checksum
    struct gcov_ctr_info ctrs[1];       //!< Instrumented counters
};

/*! \brief Describes the coverage data and meta data for a single file
 *
 *  The structure describes a file's meta data as well as coverage data
 */
struct gcov_info {
    gcov_unsigned_t version;               //!< Expected version number
    struct gcov_info* next;                //!< link to the next struct
    gcov_unsigned_t stamp;                 //!< Unique timestamp
    gcov_unsigned_t checksum;              //!< Unique object checksum
    const char* filename;                  //!< The output file name
    gcov_merge_fn merge[GCOV_COUNTERS];    //!< merge function to use (null if unused)
    unsigned n_functions;                  //!< The number of functions
    struct gcov_fn_info** functions;       //!< pointer to pointers to function infos
};

//! The head of the list of coverage data for each file
static gcov_info_tag* gcov_head = NULL;

//! The buffer where the coverage data converted to gcda will be stored
static unsigned char* gcov_output_buffer = (unsigned char*) (NULL);
//! The index at which the data will be written
static gcov_unsigned_t gcov_output_index = 0;

static gcov_unsigned_t gcov_output_buffer_sz = 0U;

//! One Entry for each file that was compiled with coverage info
static gcov_info_tag gcov_info_file_buf[100];
//! The number of files __gcov_init() will be called for
static gcov_unsigned_t gcov_info_file_idx = 0;

//! Pointer to memory where the gcda data of a single file will be temporarily stored
static gcov_unsigned_t* gcov_gcda_buffer = NULL;
//! Space needs to be enough for the largest single file coverage data
//! The size used depends on the size and complexity of the source code that is
//! compiled for coverage
static gcov_unsigned_t gcov_gcda_buffer_sz = 0U;

void set_gcov_buffer(unsigned char* start_address, const gcov_unsigned_t size) {
    gcov_output_buffer = start_address;
    gcov_output_buffer_sz = size;
}

void set_gcov_gcda_buffer(gcov_unsigned_t* start_address, const gcov_unsigned_t size) {
    gcov_gcda_buffer = start_address;
    gcov_gcda_buffer_sz = size;
}

/*! \brief Saves the gcov data to a file
 *
 *  \param[in]  filename    The name of the file the data will be stored in
 *  \param[in]  data        The pointer to the data to store
 *  \param[in]  size        The size of the data to store in bytes
 *
 *  Saves the gcda to a file. \\ TODO: Remove this function for embedded
 */
void save_file(const char* filename, unsigned char* data, uint32_t size) {
    if((data == NULL) || (size == 0))
        return;

    int f = open(filename, (O_CREAT | O_WRONLY), (S_IRWXU | S_IRWXG | S_IRWXO));
    if(f < 0) {
        return;
    }
    int result = write(f, data, size);
    close(f);
    return;
}

/*! \brief Stores a uint32 value to the buffer
 *
 *  \param buffer   The buffer in which to store the values
 *  \param offset   The offset in the buffer at which to place the values
 *  \param value    The value to store
 *  \return         Returns the number of uint32 words stored.
 *
 *  Stores a uint32 value in the provided buffer.
 */
static size_t store_gcov_unsigned(gcov_unsigned_t* buffer, size_t offset, gcov_unsigned_t value) {
    if(buffer) {
        gcov_unsigned_t* data = buffer + offset;
        *data = value;
    }
    return sizeof(value) / sizeof(*buffer);
}

/*! \brief Stores a GCOV Tag with its length in the gcov format to the buffer
 *
 *  \param buffer   The buffer in which to store the values
 *  \param offset   The offset in the buffer at which to place the values
 *  \param tag      The tag to store
 *  \param length   The length to store
 *  \return         Returns the number of uint32 words stored.
 *
 *  Stores a gcov tag and length in the provided buffer.
 */
static size_t store_gcov_tag_length(gcov_unsigned_t* buffer,
                                    const size_t offset,
                                    const gcov_unsigned_t tag,
                                    const gcov_unsigned_t length) {
    if(buffer) {
        (void) store_gcov_unsigned(buffer, offset, tag);
        (void) store_gcov_unsigned(buffer, offset + sizeof(tag) / sizeof(*buffer), length);
    }
    return (sizeof(tag) + sizeof(length)) / sizeof(*buffer);
}

/*! \brief Stores a gcov counter which is a uint64 value to the provided buffer
 *
 *  \param[in]  buffer  The buffer in which to store the value
 *  \param[in]  offset The offset in the buffer at which to place the value
 *  \param[in]  value  The value to store in the buffer
 *  \return Returns the number of uint32 words stored
 *
 *  Stores a gcov counter which is a uint64 value to the provided buffer. If the buffer
 *  is a nullptr, nothing is stored.
 *  In GCOV 64 bit numbers are stored as two 32 bit numbers, the low part first.
 */
static size_t store_gcov_counter(gcov_unsigned_t* buffer, size_t offset, gcov_type value) {
    if(buffer) {
        (void) store_gcov_unsigned(buffer, offset, value & 0xFFFF'FFFFUL);
        (void) store_gcov_unsigned(
            buffer, offset + sizeof(*buffer) / sizeof(*buffer), value >> 32U);
    }
    return sizeof(value) / sizeof(*buffer);
}

size_t gcov_convert_to_gcda(gcov_unsigned_t* buffer, struct gcov_info* info) {
    size_t buffer_pos = 0U;
    buffer_pos += store_gcov_tag_length(buffer, buffer_pos, GCOV_DATA_MAGIC, info->version);
    buffer_pos += store_gcov_unsigned(buffer, buffer_pos, info->stamp);
    buffer_pos += store_gcov_unsigned(buffer, buffer_pos, info->checksum);

    for(size_t function_idx = 0U; function_idx < info->n_functions; ++function_idx) {
        const struct gcov_fn_info* function = info->functions[function_idx];

        buffer_pos +=
            store_gcov_tag_length(buffer, buffer_pos, GCOV_TAG_FUNCTION, GCOV_TAG_FUNCTION_LENGTH);
        buffer_pos += store_gcov_unsigned(buffer, buffer_pos, function->ident);
        buffer_pos += store_gcov_unsigned(buffer, buffer_pos, function->lineno_checksum);
        buffer_pos += store_gcov_unsigned(buffer, buffer_pos, function->cfg_checksum);

        const struct gcov_ctr_info* counters = function->ctrs;
        for(size_t counter_idx = 0U; counter_idx < GCOV_COUNTERS; ++counter_idx) {
            if(!info->merge[counter_idx])
                continue;    // unused counter
            // counter record
            buffer_pos += store_gcov_tag_length(buffer,
                                                buffer_pos,
                                                GCOV_TAG_FOR_COUNTER(counter_idx),
                                                GCOV_TAG_COUNTER_LENGTH(counters->num));
            for(size_t counter_value_idx = 0U; counter_value_idx < counters->num;
                ++counter_value_idx) {
                buffer_pos +=
                    store_gcov_counter(buffer, buffer_pos, counters->values[counter_value_idx]);
            }
            ++counters;
        }
    }
    return buffer_pos * sizeof(*buffer);
}

void __gcov_init(struct gcov_info* info) {
    gcov_info_tag* new_head = NULL;

    // check that we aren't over the maximum allowed number of files
    if(gcov_info_file_idx >= (sizeof(gcov_info_file_buf) / sizeof(gcov_info_file_buf[0])))
        new_head = NULL;
    else
        new_head = gcov_info_file_buf + gcov_info_file_idx;

    new_head->info = info;
    new_head->next = gcov_head;
    gcov_head = new_head;
    ++gcov_info_file_idx;
}

void __gcov_exit(void) {
    gcov_output_index = 0;

    gcov_info_tag* list_ptr = gcov_head;
    //!< \TODO: replace with embedded allocation to static memory

    // Add checks that the output buffer pointer does not exceed the limits of the buffer
    while(list_ptr) {
        const uint32_t bytes_needed = gcov_convert_to_gcda(NULL, list_ptr->info);

        gcov_unsigned_t* buffer = NULL;
        if(bytes_needed > (gcov_gcda_buffer_sz * sizeof(gcov_unsigned_t))) {
            buffer = (gcov_unsigned_t*) NULL;
        } else {
            // buffer = gcov_buffer; // TODO: use something like this for embedded
            // clear buffer before use
            memset(gcov_gcda_buffer, 0U, gcov_gcda_buffer_sz * sizeof(gcov_gcda_buffer_sz));
            buffer = gcov_gcda_buffer;    // TODO: Replace with embedded allocation
        }
        // + 6 because of the end string "GCOV End\n" we need to add
        if(bytes_needed
           > ((gcov_output_buffer_sz - gcov_output_index) * sizeof(gcov_unsigned_t) + 9U))
            return;

        if(!buffer)    //! not enough memory reserved
            return;
        // convert the binary data to gcda and store it in a local buffer
        (void) gcov_convert_to_gcda(buffer, list_ptr->info);

        // copy the filename to the output buffer
        for(const char* filename = list_ptr->info->filename; filename && (*filename); ++filename) {
            gcov_output_buffer[gcov_output_index++] = (*filename);
        }
        // trailing null char for string completion
        gcov_output_buffer[gcov_output_index++] = '\0';
        // store data byte count with MSB first
        gcov_output_buffer[gcov_output_index++] = (unsigned char) ((bytes_needed >> 24U) & 0xFFFF);
        gcov_output_buffer[gcov_output_index++] = (unsigned char) ((bytes_needed >> 16U) & 0xFFFF);
        gcov_output_buffer[gcov_output_index++] = (unsigned char) ((bytes_needed >> 8U) & 0xFFFF);
        gcov_output_buffer[gcov_output_index++] = (unsigned char) (bytes_needed & 0xFFFF);

        // copy converted gcda data to output location
        for(size_t i = 0U; i < bytes_needed; ++i) {
            gcov_output_buffer[gcov_output_index++] =
                (unsigned char) (((unsigned char*) buffer)[i]);
        }
        // free(buffer);    //!< TODO: Replace with embedded allocation maybe
        list_ptr = list_ptr->next;
    }

    gcov_output_buffer[gcov_output_index++] = 'G';
    gcov_output_buffer[gcov_output_index++] = 'c';
    gcov_output_buffer[gcov_output_index++] = 'o';
    gcov_output_buffer[gcov_output_index++] = 'v';
    gcov_output_buffer[gcov_output_index++] = ' ';
    gcov_output_buffer[gcov_output_index++] = 'E';
    gcov_output_buffer[gcov_output_index++] = 'n';
    gcov_output_buffer[gcov_output_index++] = 'd';
    gcov_output_buffer[gcov_output_index++] = '\0';

    save_file("../output/gcov_output.bin", gcov_output_buffer, gcov_output_index);
    free(gcov_gcda_buffer);
    free(gcov_output_buffer);
    return;
}

void __gcov_merge_add(gcov_type* counters, gcov_unsigned_t n_counters) {
    (void) counters;
    (void) n_counters;
    return;
}

void __gcov_merge_ior(gcov_type* counters, gcov_unsigned_t n_counters) {
    (void) counters;
    (void) n_counters;
    return;
}
