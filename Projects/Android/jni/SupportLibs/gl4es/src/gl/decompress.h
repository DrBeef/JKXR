#ifndef _GL4ES_DECOMPRESS_H_
#define _GL4ES_DECOMPRESS_H_

void DecompressBlockDXT1(uint32_t x, uint32_t y, uint32_t width,
	const uint8_t* blockStorage,
	uint32_t* image);

void DecompressBlockDXT3(uint32_t x, uint32_t y, uint32_t width,
	const uint8_t* blockStorage,
	uint32_t* image);

void DecompressBlockDXT5(uint32_t x, uint32_t y, uint32_t width,
	const uint8_t* blockStorage, uint32_t* image);

#endif // _GL4ES_DECOMPRESS_H_
