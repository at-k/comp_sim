#include "comp.h"
#include <snappy.h>
#include <vector>
#include <iostream>
#include <fstream>


bool Zlib::Compress(std::istream *input, std::ostream* output, uint32_t bs, size_t* result_size)
{
	z_stream stream;
	DeflateInit(&stream);

	int flush = Z_NO_FLUSH;
	int ret = Z_OK;

	char input_buf[bs];
	char output_buf[bs];

	do {
		//input_buf.resize(bs);
		//input->read(&*input_buf.begin(), input_buf.size());
		input->read(input_buf, bs);

		if (input->fail() && !input->eof()) {
			ERROR("failed to read data from file");
		}
		//input_buf.resize(input->gcount());
	//	if (input_buf.empty()) {
	//		break;
	//	}

		stream.avail_in = input->gcount();
		stream.next_in = (Bytef*)input_buf;

		if( input->eof() )
			flush = Z_FINISH;

		do {
			// 出力バッファを再設定して圧縮の続きをおこないます．
			stream.next_out = (Bytef*)output_buf;
			stream.avail_out = sizeof(output_buf);
			//stream.avail_out = bs;
			ret = deflate(&stream, flush);
			if (ret == Z_STREAM_ERROR) {
				ERROR("%s", stream.msg);
			}

			if( result_size != NULL )
				*result_size +=  (sizeof(output_buf) - stream.avail_out);

			if( output != NULL ) {
				if( !output->write( output_buf, sizeof(output_buf) - stream.avail_out ) ) {
					ERROR("failed to write data into file");
				}
			}
			//fwrite(output_buf, sizeof(output_buf) - stream.avail_out, 1, output_file);
			//if (ferror(output_file) != 0) {
			//	ERROR("failed to write into file");
			//}
		} while ((stream.avail_out == 0) && (ret != Z_STREAM_END));

		if (stream.avail_in != 0) {
			ERROR("unexpected bytes in input buffer");
		}
	} while (flush != Z_FINISH);
	if (ret != Z_STREAM_END) {
		ERROR("failed to finish deflate");
	}

	DeflateEnd(&stream);
	//delete[] input_buf;
	//delete[] output_buf;

	return true;
}

bool Zlib::Compress(const char* input_buf, const size_t input_size,
		char* output_buf, const size_t output_size, size_t* result_size)
{
	z_stream stream;
	DeflateInit(&stream);

	int flush = Z_NO_FLUSH;
	int ret = Z_OK;

	//char output_buf[4096];
	size_t cur_pointer = 0;

	stream.avail_in = input_size;
	stream.next_in = (Bytef*)input_buf;
	flush = Z_FINISH;

	do {
		// 出力バッファを再設定して圧縮の続きをおこないます．
		stream.next_out = (Bytef*)(output_buf + cur_pointer);
		//stream.avail_out = sizeof(output_buf) - cur_pointer;
		stream.avail_out = output_size - cur_pointer;
		ret = deflate(&stream, flush);

		if (ret == Z_STREAM_ERROR) {
			ERROR("%s", stream.msg);
		}

		// cur_pointer += sizeof(output_buf) - stream.avail_out;
		cur_pointer += output_size - stream.avail_out;
	//	printf("%ld\n", cur_pointer);


		//if( !output->write( output_buf, sizeof(output_buf) - stream.avail_out ) ) {
		//	ERROR("failed to write data into file");
		//}
		//fwrite(output_buf, sizeof(output_buf) - stream.avail_out, 1, output_file);
		//if (ferror(output_file) != 0) {
		//	ERROR("failed to write into file");
		//}
	} while ((stream.avail_out == 0) && (ret != Z_STREAM_END));

	*result_size = cur_pointer;

	if (stream.avail_in != 0 || ret != Z_STREAM_END ) {
		ERROR("unexpected bytes in input buffer");
	}

	DeflateEnd(&stream);

	return true;
}

bool Zlib::DeCompress(const char* input_buf, uint32_t input_size, char* output_buf, uint32_t* output_size)
{
	return true;
}

bool Zlib::DeCompress(std::istream *input, std::ostream* output)
{
	return true;
}

void Zlib::DeflateInit(z_stream* stream)
{
	// メモリの確保・解放は zlib に任せます．
	stream->zalloc = Z_NULL;
	stream->zfree = Z_NULL;
	stream->opaque = Z_NULL;

	// deflateInit() では zlib 形式になります．deflateInit2() の第 3 引数を
	// 24 以上 31 以下の値にすると gzip 形式になります．deflateInit2() で
	// zlib 形式の圧縮をする場合は，第 3 引数を 8 以上 15 以下にします．
	// deflateInit() の動作は 15 のときと同じです．
	int ret;
	switch ( fmt ) {
		case GZIP_FORMAT:
			ret = deflateInit2(stream, compression_level,
					Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
			break;
		case ZLIB_FORMAT:
			ret = deflateInit(stream, compression_level);
			break;
		default:
			ERROR("invalid format: %d", fmt);
	}

	if (ret != Z_OK) {
		// deflateInit(), deflateInit2() はエラーが起きても .msg を更新しません．
		// エラーメッセージの取得には zError() を利用することになります．
		ERROR("%s", zError(ret));
	}

}

void Zlib::DeflateEnd(z_stream* stream)
{
	int ret = deflateEnd(stream);
	if (ret != Z_OK) {
		// deflateEnd() はエラーが起きても .msg を更新しません．
		// エラーメッセージの取得には zError() を利用することになります．
		ERROR("%s", zError(ret));
	}
}

void Zlib::InflateInit(z_stream* stream)
{
	// メモリの確保・解放は zlib に任せます．
	stream->zalloc = Z_NULL;
	stream->zfree = Z_NULL;
	stream->opaque = Z_NULL;

	// .next_in, .avail_in は inflateInit(), inflateInit2() を呼び出す前に
	// 初期化しておく必要があります．
	stream->next_in = Z_NULL;
	stream->avail_in = 0;

	int ret = inflateInit2(stream, 47);
	if (ret != Z_OK) {
		// inflateInit(), inflateInit2() はエラーが起きても .msg を更新しません．
		// エラーメッセージの取得には zError() を利用することになります．
		ERROR("%s", zError(ret));
	}

}

void Zlib::InflateEnd(z_stream* stream)
{
	int ret = inflateEnd(stream);
	if (ret != Z_OK) {
		// deflateEnd() はエラーが起きても .msg を更新しません．
		// エラーメッセージの取得には zError() を利用することになります．
		ERROR("%s", zError(ret));
	}
}

