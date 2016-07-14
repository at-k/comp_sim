#ifndef COMP_H
#define COMP_H

#include <stdint.h>
#include <fstream>
#include <iostream>
#include <error.h>
#include <errno.h>
#include <zlib.h>

#define ERROR(fmt, ...) \
  error_at_line(-(__LINE__), errno, __FILE__, __LINE__, fmt, ## __VA_ARGS__)


class Compression {
	public:
		Compression(){}
		virtual ~Compression(){}

		virtual bool Compress(std::istream *input, std::ostream* output, uint32_t bs, size_t* result_size) = 0;
		virtual bool DeCompress(std::istream *input, std::ostream* output) = 0;

		virtual bool Compress(const char* input_buf, const size_t input_buf_size,
				char* output_buf, const size_t output_buf_size, size_t* result_size) = 0;
		virtual bool DeCompress(const char* input_buf, uint32_t input_size, char* output_buf, uint32_t* output_size) = 0;

};

class Snappy : public Compression {
	public:
		Snappy() {}
		virtual ~Snappy() {}

		virtual bool Compress(std::istream *input, std::ostream* output, uint32_t bs, size_t* result_size);
		virtual bool DeCompress(std::istream *input, std::ostream* output);

		virtual bool Compress(const char* input_buf, const size_t input_buf_size,
				char* output_buf, const size_t output_buf_size, size_t* result_size) ;

		virtual bool DeCompress(const char* input_buf, uint32_t input_size, char* output_buf, uint32_t* output_size);

};

typedef enum {
	ZLIB_FORMAT,
	GZIP_FORMAT
} DEFLATE_FORMAT;

class Zlib : public Compression {
	public:
		Zlib() { fmt = ZLIB_FORMAT; }
		virtual ~Zlib() {}

		void SetFormat(DEFLATE_FORMAT _fmt) { fmt = _fmt;}

		virtual bool Compress(std::istream *input, std::ostream* output, uint32_t bs, size_t* result_size);
		virtual bool DeCompress(std::istream *input, std::ostream* output);

		virtual bool Compress(const char* input_buf, const size_t input_buf_size,
				char* output_buf, const size_t output_buf_size, size_t* result_size) ;

		virtual bool DeCompress(const char* input_buf, uint32_t input_size, char* output_buf, uint32_t* output_size);


	private:
		DEFLATE_FORMAT fmt;
		int compression_level = Z_DEFAULT_COMPRESSION;

		void DeflateInit(z_stream* stream);
		void DeflateEnd(z_stream* stream);

		void InflateInit(z_stream* stream);
		void InflateEnd(z_stream* stream);
};

class BZip2 : public Compression {
	public:
		BZip2() {}
		~BZip2() {}

		virtual bool Compress(std::istream *input, std::ostream* output, uint32_t bs, size_t* result_size);
		virtual bool DeCompress(std::istream *input, std::ostream* output);
};

#endif

