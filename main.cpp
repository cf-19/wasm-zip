#include <stdio.h>

#ifdef EM_BUILD
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <emscripten/wire.h>
#include <zip.h>
#else
#include "./emsdk/emscripten.h"
#include "./emsdk/emscripten/bind.h"
#include "./emsdk/emscripten/val.h"
#include "./emsdk/emscripten/wire.h"
#include "./include/zip.h"
#endif

using namespace emscripten;

struct entry
{
	std::string filename;
	std::string buf;
};

struct entry_out
{
	std::string filename;
	val buf;
	entry_out() : filename(NULL), buf(NULL) {}
};

val zip_files(std::vector<entry> files)
{
	char* zip_buf = NULL;
	size_t outbufsize;

	struct zip_t* zip = zip_stream_open(NULL, 0, ZIP_DEFAULT_COMPRESSION_LEVEL, 'w');

	for (auto& file : files)
	{
		zip_entry_open(zip, file.filename.c_str());
		{
			zip_entry_write(zip, file.buf.c_str(), file.buf.size());
		}
		zip_entry_close(zip);
	}

	zip_stream_copy(zip, (void**)&zip_buf, &outbufsize);
	zip_stream_close(zip);

	return val(typed_memory_view(outbufsize, zip_buf));
}

std::vector<entry_out> extract_from_zip(std::string zipbuf)
{
	std::vector<entry_out> files;

	struct zip_t* zip = zip_stream_open(zipbuf.c_str(), zipbuf.size(), 0, 'r');
	{
		int i, n = zip_entries_total(zip);

		for (i = 0; i < n; ++i)
		{
			char* file_buf;
			size_t buf_size;

			zip_entry_openbyindex(zip, i);
			{
				const char* name = zip_entry_name(zip);
				int isdir = zip_entry_isdir(zip);
				unsigned long long size = zip_entry_size(zip);

				zip_entry_read(zip, (void**)&file_buf, &buf_size);

				if (!isdir)
				{
					struct entry_out file;
#ifdef DEBUG
					// TODO: filename decode
					printf("filename: %s \n", name);
					printf("filesize: %zu \n", buf_size);
#endif // DEBUG
					file.filename = name;
					file.buf = val(typed_memory_view(buf_size, file_buf));

					files.push_back(file);
				}
			}
			zip_entry_close(zip);
		}
	}
	zip_stream_close(zip);

	return files;
}

EMSCRIPTEN_BINDINGS(ZIP_MODULE)
{
	function("zip_files", &zip_files);
	function("extract_from_zip", &extract_from_zip);

	value_object<entry>("entry")
		.field("filename", &entry::filename)
		.field("buf", &entry::buf);
	value_object<entry_out>("entry_out")
		.field("filename", &entry_out::filename)
		.field("buf", &entry_out::buf);

	register_vector<entry>("file_list");
	register_vector<entry_out>("file_list_out");
}