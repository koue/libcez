#
# Copyright (c) 2018-2019 Nikola Kolev <koue@chaosophia.net>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# FindLibcez
# ----------
#
# Find libcez headers and libraries
#
# ::
#
#  LIBCEZ_FOSSIL_BASE_INCLUDE_DIR - libcez fossil base include directory
#  LIBCEZ_FOSSIL_CSON_INCLUDE_DIR - libcez fossil cson include directory
#  LIBCEZ_FOSSIL_DB_INCLUDE_DIR - libcez fossil db include directory
#  LIBCEZ_KV_INCLUDE_DIR - libcez key-value include directory
#  LIBCEZ_MISC_INCLUDE_DIR - libcez misc include directory
#  LIBCEZ_PRAYER_INCLUDE_DIR - libcez prayer include directory
#  LIBCEZ_QUEUE_INCLUDE_DIR - libcez queue include directory
#
#  LIBCEZ_FOSSIL_BASE_LIBRARIES - libcez fossil base library
#  LIBCEZ_FOSSIL_CSON_LIBRARIES - libcez fossil cson library
#  LIBCEZ_FOSSIL_DB_LIBRARIES - libcez fossil db library
#  LIBCEZ_KV_LIBRARIES - libcez key-value library
#  LIBCEZ_MISC_LIBRARIES - libcez misc library
#  LIBCEZ_PRAYER_LIBRARIES - libcez prayer library
#  LIBCEZ_QUEUE_LIBRARIES - libcez queue library
#
#  LIBCEZ_FOUND - libcez found

# Look for libcez fossil base header file.
find_path(LIBCEZ_FOSSIL_BASE_INCLUDE_DIR NAMES cez_fossil_base.h)
# Look for libcez fossil cson header file.
find_path(LIBCEZ_FOSSIL_CSON_INCLUDE_DIR NAMES cez_fossil_cson.h)
# Look for libcez fossil db header file.
find_path(LIBCEZ_FOSSIL_db_INCLUDE_DIR NAMES cez_fossil_db.h)
# Look for libcez key-value header file.
find_path(LIBCEZ_KV_INCLUDE_DIR NAMES cez_kv.h)
# Look for libcez misc header file.
find_path(LIBCEZ_MISC_INCLUDE_DIR NAMES cez_misc.h)
# Look for libcez prayer header file.
find_path(LIBCEZ_PRAYER_INCLUDE_DIR NAMES cez_prayer.h)
# Look for libcez queue header file.
find_path(LIBCEZ_QUEUE_INCLUDE_DIR NAMES cez_queue.h)

# Look for libcez fossil base library
find_library(LIBCEZ_FOSSIL_BASE_LIBRARIES NAMES cezfossilbase)
# Look for libcez fossil cson library
find_library(LIBCEZ_FOSSIL_CSON_LIBRARIES NAMES cezfossilcson)
# Look for libcez fossil db library
find_library(LIBCEZ_FOSSIL_DB_LIBRARIES NAMES cezfossildb)
# Look for libcez key-value library
find_library(LIBCEZ_KV_LIBRARIES NAMES cezkv)
# Look for libcez misc library
find_library(LIBCEZ_MISC_LIBRARIES NAMES cezmisc)
# Look for libcez prayer library
find_library(LIBCEZ_PRAYER_LIBRARIES NAMES cezprayer)
# Look for libcez queue library
find_library(LIBCEZ_QUEUE_LIBRARIES NAMES cezqueue)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Libcez DEFAULT_MSG
			LIBCEZ_FOSSIL_BASE_INCLUDE_DIR LIBCEZ_FOSSIL_BASE_LIBRARIES
			LIBCEZ_FOSSIL_CSON_INCLUDE_DIR LIBCEZ_FOSSIL_CSON_LIBRARIES
			LIBCEZ_FOSSIL_DB_INCLUDE_DIR LIBCEZ_FOSSIL_DB_LIBRARIES
			LIBCEZ_KV_INCLUDE_DIR LIBCEZ_KV_LIBRARIES
			LIBCEZ_MISC_INCLUDE_DIR LIBCEZ_MISC_LIBRARIES
			LIBCEZ_PRAYER_INCLUDE_DIR LIBCEZ_PRAYER_LIBRARIES
			LIBCEZ_QUEUE_INCLUDE_DIR LIBCEZ_QUEUE_LIBRARIES
)

mark_as_advanced(LIBCEZ_FOSSIL_BASE_INCLUDE_DIR LIBCEZ_FOSSIL_BASE_LIBRARIES
		LIBCEZ_FOSSIL_CSON_INCLUDE_DIR LIBCEZ_FOSSIL_CSON_LIBRARIES
		LIBCEZ_FOSSIL_DB_INCLUDE_DIR LIBCEZ_FOSSIL_DB_LIBRARIES
		LIBCEZ_KV_INCLUDE_DIR LIBCEZ_KV_LIBRARIES
		LIBCEZ_MISC_INCLUDE_DIR LIBCEZ_MISC_LIBRARIES
		LIBCEZ_PRAYER_INCLUDE_DIR LIBCEZ_PRAYER_LIBRARIES
		LIBCEZ_QUEUE_INCLUDE_DIR LIBCEZ_QUEUE_LIBRARIES
)
