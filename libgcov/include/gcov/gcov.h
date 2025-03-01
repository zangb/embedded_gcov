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

#ifndef LIB_GCOV_H
#define LIB_GCOV_H

#include <stddef.h>

#if !defined(__GNUC__) || (__GNUC__ != 14)
#error "This library only works with GCC 14, for other version checks the structs and defines"
#endif

struct gcov_info;

typedef unsigned gcov_unsigned_t;
typedef long long gcov_type;

// This is dependent on the gcc/gcov-counter.def and correponds to the number of counters
// listed in that file, see also gcc/gcov-io.h
#define GCOV_COUNTERS                9
#define GCOV_DATA_MAGIC              ((gcov_unsigned_t) 0x67636461)    // "gcda"
#define GCOV_TAG_FUNCTION            ((gcov_unsigned_t) 0x01000000)
#define GCOV_TAG_COUNTER_BASE        ((gcov_unsigned_t) 0x01a10000)
#define GCOV_TAG_FOR_COUNTER(COUNT)  (GCOV_TAG_COUNTER_BASE + ((gcov_unsigned_t) (COUNT) << 17))
#define GCOV_WORD_SIZE               4
#define GCOV_TAG_FUNCTION_LENGTH     (3 * GCOV_WORD_SIZE)
#define GCOV_TAG_COUNTER_LENGTH(NUM) ((NUM) * 2 * GCOV_WORD_SIZE)

/*! \brief Converts the internal gcov data tree into the gcda output format
 *
 *  \param[in]  buffer  The buffer to store the data in, NULL if no data should be stored
 *  \param[in]  info    The pointer to the gcov coverage data
 *  \return             The number of bytes the were/would have been stored in the buffer
 *
 *  Converts the internal gcov data tree into the gcda output format. If this
 *  function is called with a nullptr for \a buffer, the number of bytes needed can be
 *  determined. Compare to libgcc/libgcov-driver.c function write_one_data()
 */
extern size_t gcov_convert_to_gcda(gcov_unsigned_t* buffer, struct gcov_info* info);

/*! \brief Called for each object file
 *
 *  \param[in]  info    The info object associated with the object file
 *
 *  Called for each object file to init the counters.
 */
extern void __gcov_init(struct gcov_info* info);

/*! \brief Called for each object file to summarize coverage data
 *
 *  Called for each object file to summarize coverage data
 */
extern void __gcov_exit(void);

/*! \brief Function must not be called but needs to be defined
 *
 *  \param[in]   counters   Not specified in gcc documentation / source code
 *  \param[in]   n_counters Not specified in gcc documentation / source code
 *
 *  Function must not be called but needs to be defined
 */
extern void __gcov_merge_add(gcov_type* counters, gcov_unsigned_t n_counters);

/*! \brief Function must not be called but needs to be defined when compiling with MC/DC
 *
 *  \param[in]  counters    Not specified in gcc documentation / source code
 *  \param[in]  n_counters  Not specified in gcc documentation / source code
 *
 *  Function must not be called but needs to be defined when compiling with MC/DC coverage
 */
extern void __gcov_merge_ior(gcov_type* counters, gcov_unsigned_t n_counters);

#endif
