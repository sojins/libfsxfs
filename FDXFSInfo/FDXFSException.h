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

#if !defined( _FDXFS_EXCEPTION_H )
#define _FDXFS_EXCEPTION_H 

#include <exception>
#include <string>
#include <typeinfo>

#include <common.h>
#include <types.h>

#include <file_stream.h>
#include <libcerror_types.h>


class FDXFSException : public std::exception {
	std::string message;
	int _err;
public:
	FDXFSException(int err, std::string _m) : message(_m) { _err = err; }
	virtual const char* what() const throw() {
		return message.c_str();
	}
	const int code() { return _err; }
};

//LIBCERROR_ERROR_DOMAIN_ARGUMENTS = (int)'a',
class FDArgumentException : public FDXFSException {
public:
	FDArgumentException(int err, std::string _m) : FDXFSException(err, _m) {}
};
//LIBCERROR_ERROR_DOMAIN_CONVERSION = (int)'c',
class FDConversionException : public FDXFSException {
public:
	FDConversionException(int err, std::string _m) : FDXFSException(err, _m) {}
};
//LIBCERROR_ERROR_DOMAIN_COMPRESSION = (int)'C',
class FDCompressionException : public FDXFSException {
public:
	FDCompressionException(int err, std::string _m) : FDXFSException(err, _m) {}
};
//LIBCERROR_ERROR_DOMAIN_ENCRYPTION = (int)'E',
class FDEncryptionException : public FDXFSException {
public:
	FDEncryptionException(int err, std::string _m) : FDXFSException(err, _m) {}
};
//LIBCERROR_ERROR_DOMAIN_IO = (int)'I',
class FDIOException : public FDXFSException {
public:
	FDIOException(int err, std::string _m) : FDXFSException(err, _m) {}
};
//LIBCERROR_ERROR_DOMAIN_INPUT = (int)'i',
class FDInputException : public FDXFSException {
public:
	FDInputException(int err, std::string _m) : FDXFSException(err, _m) {}
};
//LIBCERROR_ERROR_DOMAIN_MEMORY = (int)'m',
class FDMemoryException : public FDXFSException {
public:
	FDMemoryException(int err, std::string _m) : FDXFSException(err, _m) {}
};
//LIBCERROR_ERROR_DOMAIN_OUTPUT = (int)'o',
class FDOutputException : public FDXFSException {
public:
	FDOutputException(int err, std::string _m) : FDXFSException(err, _m) {}
};
//LIBCERROR_ERROR_DOMAIN_RUNTIME = (int)'r',
class FDRuntimeException : public FDXFSException {
public:
	FDRuntimeException(int err, std::string _m) : FDXFSException(err, _m) {}
};

#endif /* !defined( _FDXFS_EXCEPTION_H ) */
