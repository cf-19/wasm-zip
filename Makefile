test:
	emcc main.cpp \
		lib/libzip_wasm.a \
		-sALLOW_MEMORY_GROWTH \
		-sEXPORT_NAME="'ZipModule'" \
		-O0 \
		-I ./include \
		-o disk/zip_wasm.js \
		-D EM_BUILD \
		-D DEBUG \
		-lembind

prod:
	emcc main.cpp \
		lib/libzip_wasm.a \
		-sALLOW_MEMORY_GROWTH \
		-sEXPORT_NAME="'ZipModule'" \
		-O2 \
		-I ./include \
		-o disk/zip_wasm.js \
		-D EM_BUILD \
		-lembind