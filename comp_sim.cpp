#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <string>
#include <stdint.h>
#include <fstream>

#include <vector>

#include "comp.h"

#define OPT_ERROR { HowtoUse(argv[0]); return 0; }

typedef enum {
	SNAPPY,
	ZLIB,
	ZIP2
} COMP_TYPE;

typedef enum {
	CAFTL,
	PROP
} ALIGN_TYPE;

void HowtoUse(char* bin_name)
{
    printf("Usage: %s [options]", bin_name);
    printf("\n");
    printf("  --in_file,-i file_name\n");
    printf("     set input file\n");
    printf("  --out_file,-o out_file_name\n");
    printf("     set output file, it can be optional\n");
    printf("  --alignment, -a num\n");
    printf("     set alignment number, effect on padding frequency\n");
	printf("  --block_size, -b num\n");
	printf("     set compression block size, effect on comp ratio\n");
	printf("  --help,-h \n");
	printf("     print this message\n");
    printf("\n");
}

bool CheckAdditionalOpt(int i, int argc, char* argv[])
{
    if( i >= argc || argv == NULL) {
        printf("Option Error\nThis option requires an additional argument -- %s\n", argv[i-1]);
        return false;
    }
    return true;
}

size_t Compress(std::istream *input_stream, std::ostream *output_stream, COMP_TYPE type, uint32_t block_size ,
		ALIGN_TYPE a_type, uint32_t padding_align_size)
{
	Compression* cmp_eng = NULL;
	std::vector<char> input_buf;
	size_t	output_size = block_size*2;
	char	*output_buf = new char[output_size];
	size_t	result_size = 0;
	size_t  pad_size = 0;
	size_t  ttl_size = 0;
	size_t  ttl_pad_size = 0;
	size_t  buf_size = 0;

	switch(type) {
		case SNAPPY:
			cmp_eng = new Snappy();
			break;
		case ZLIB:
			cmp_eng = new Zlib();
			break;
		case ZIP2:
			//cmp_eng = new Zip2();
			break;
		default:
			break;
	}

	while(1) {
		input_buf.resize(block_size);
		input_stream->read(&*input_buf.begin(), input_buf.size());

		if (input_stream->fail() && !input_stream->eof()) {
			ERROR("failed to read data from file");
		}
		input_buf.resize(input_stream->gcount());
		if (input_buf.empty()) {
			break;
		}
		cmp_eng->Compress( input_buf.data(), input_buf.size(), output_buf, output_size, &result_size);

		if( a_type == CAFTL ) {
			// add padding to aling data size as padding_align_size x N
			pad_size = (result_size % padding_align_size) == 0 ? 0 : (padding_align_size - result_size % padding_align_size);
			result_size += pad_size;

			ttl_size += result_size;
			ttl_pad_size += pad_size;

		}else if( a_type == PROP ) {
			if( buf_size + result_size > padding_align_size ) {
				pad_size = padding_align_size - buf_size;

				ttl_size += buf_size + pad_size;
				ttl_pad_size += pad_size;

				buf_size = result_size;
			}else {
				buf_size += result_size;
			}
		}else
			ERROR("invalid type\n");
	}

	if( a_type == PROP && buf_size != 0 ) {
		pad_size = padding_align_size - buf_size;

		ttl_size += buf_size + pad_size;
		ttl_pad_size += pad_size;
	}

	// cmp_eng->Compress(input_stream, output_stream, block_size, &ttl_size);

	return ttl_size;
}

int main(int argc, char* argv[])
{
	std::string in_file_name = "";
	std::string out_file_name = "";
	COMP_TYPE	mode = SNAPPY;
	ALIGN_TYPE	a_mode = CAFTL;

	uint32_t  block_size          = 8096;
	uint32_t  padding_align_size  = 4 * block_size;

    { // analyze input argument
        if( argc == 1 ) {
			OPT_ERROR;
		} else
        {
            for( int i = 1; i < argc; i++ )
            {
                if( strcmp(argv[i], "--in_file") == 0 || strcmp(argv[i], "-f") ==0 )
                {
                    if( !CheckAdditionalOpt(++i, argc, argv) ) { OPT_ERROR;}
                    else in_file_name = argv[i];
                }
                else if( strcmp(argv[i], "--out_file") == 0 || strcmp(argv[i], "-o") ==0 )
                {
                    if( !CheckAdditionalOpt(++i, argc, argv) ) { OPT_ERROR;}
                    else out_file_name = argv[i];
                }
                else if( strcmp(argv[i], "--alignment") == 0 || strcmp(argv[i], "-a") ==0 )
                {
                    if( !CheckAdditionalOpt(++i, argc, argv) ) { OPT_ERROR;}
                    else padding_align_size = atoi(argv[i]);
                }
                else if( strcmp(argv[i], "--align_mode") == 0 || strcmp(argv[i], "-l") ==0 )
                {
                    if( !CheckAdditionalOpt(++i, argc, argv) ) { OPT_ERROR;}
                    else block_size = atoi(argv[i]);
                }
                else if( strcmp(argv[i], "--block_size") == 0 || strcmp(argv[i], "-b") ==0 )
                {
                    if( !CheckAdditionalOpt(++i, argc, argv) ) { OPT_ERROR;}
                    else block_size = atoi(argv[i]);
                }
				else if( strcmp(argv[i], "--mode") == 0 || strcmp(argv[i], "-m") ==0 )
                {
                    if( !CheckAdditionalOpt(++i, argc, argv) ) { OPT_ERROR;}
					else if( *argv[i] == 's' ) mode = SNAPPY;
					else if( *argv[i] == 'z' ) mode = ZLIB;
					else if( *argv[i] == '2' ) mode = ZIP2;
					else { OPT_ERROR;}
                }
				else if( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 )
                {
                    HowtoUse(argv[0]);
                    return 0;
                }
                else
                {
                    printf("Invalid option -- %s\n", argv[i]);
					OPT_ERROR;
                }
            }
        }
    }

	std::ofstream output_file;
	std::ostream* output_stream = NULL;

	if( out_file_name != "" ) {
		output_file.open(out_file_name.c_str(), std::ios::binary);
		if (!output_file) {
			ERROR("%s", out_file_name.c_str());
		}
		output_stream = &output_file;
	}

	std::ifstream input_file(in_file_name.c_str(), std::ios::binary);
	if (!input_file) {
		ERROR("%s", in_file_name.c_str());
	}

	size_t org_size = (size_t)input_file.seekg(0, std::ios::end).tellg();
	input_file.seekg(0, std::ios::beg);

	size_t cmp_size = Compress(&input_file, output_stream, mode, block_size, a_mode, padding_align_size );

	double cmp_ratio = (double)cmp_size / org_size;

	printf("mode = %d, bs = %d, align = %d, org size = %ld, cmp size = %ld, cmp_ratio = %.5f\n",
			mode, block_size, padding_align_size, org_size, cmp_size, cmp_ratio);

	return 0;
}
