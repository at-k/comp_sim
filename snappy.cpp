#include "comp.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <snappy.h>

bool Snappy::Compress(std::istream *input, std::ostream* output, uint32_t bs, size_t* result_size)
{
	std::vector<char> input_buf;
	std::string output_buf;

	for ( ; ; ) {
		// ブロック単位で読み込みます．
		input_buf.resize(bs);
		input->read(&*input_buf.begin(), input_buf.size());
		if (input->fail() && !input->eof()) {
			ERROR("failed to read data from file");
		}
		input_buf.resize(input->gcount());
		if (input_buf.empty()) {
			break;
		}

		// snappy::Compress() を呼び出すだけで圧縮できます．
		const uint32_t output_size = static_cast<uint32_t>(
				snappy::Compress(&*input_buf.begin(), input_buf.size(), &output_buf));

		if( result_size != NULL )
			*result_size += output_size;

		if( output != NULL ) {
			// 伸長するときに圧縮データのサイズが必要となるため，
			// ヘッダとしてサイズを出力してから圧縮データを出力します．
			if (!output->write(reinterpret_cast<const char *>(&output_size),
						sizeof(output_size))) {
				ERROR("failed to write header into file");
			}
			if (!output->write(&*output_buf.begin(), output_size)) {
				ERROR("failed to write data into file: %u bytes", output_size);
			}
		}
	}
	return true;
}

bool Snappy::Compress(const char* input_buf, const size_t input_size,
		char* output_buf, const size_t output_size, size_t* result_size)
{
	//std::string output_buf_str;
	//(*output_size) = static_cast<uint32_t>(
	//	snappy::Compress(input_buf, input_size, &output_buf_str) );

	//std::copy(output_buf_str.begin(), output_buf_str.end(), output_buf);
	snappy::RawCompress(input_buf, input_size, output_buf, result_size);

	return true;
}

bool Snappy::DeCompress(std::istream *input, std::ostream* output)
{
	std::vector<char> input_buf;
	std::string output_buf;

	for ( ; ; ) {
		// 圧縮データのサイズを読み込み，バッファのサイズを調整した後で，
		// 圧縮データの読み込みをおこないます．
		uint32_t input_size;
		input->read(reinterpret_cast<char *>(&input_size),
				sizeof(input_size));
		if (input->fail() && !input->eof()) {
			ERROR("failed to read header from file");
		} else if (input->gcount() != sizeof(input_size)) {
			if (input->gcount() == 0) {
				break;
			}
			ERROR("invalid format: incomplete header");
		}
		input_buf.resize(input_size);
		if (!input->read(&*input_buf.begin(), input_buf.size())) {
			ERROR("failed to read data from file: %zu bytes", input_buf.size());
		}

		// snappy::Uncompress() を呼び出すだけで伸長できます．
		if (!snappy::Uncompress(&*input_buf.begin(), input_buf.size(),
					&output_buf)) {
			ERROR("failed to uncompress data");
		}

		// 伸長したデータを出力します．
		if (!output->write(&*output_buf.begin(), output_buf.size())) {
			ERROR("failed to write data into file: %zu bytes", output_buf.size());
		}
	}
	return true;
}

bool Snappy::DeCompress(const char* input_buf, uint32_t input_size, char* output_buf, uint32_t* output_size)
{
	return true;
}
