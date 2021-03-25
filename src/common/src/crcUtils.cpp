#include "common/crcUtils.h"

namespace crc
{
void updateCRC32(unsigned char byte, std::uint32_t& crc)
{
	crc = lookupTable[(crc >> 24) ^ byte] ^ (crc << 8);
}

std::uint32_t calculateCRC32(unsigned char* buffer, std::size_t length)
{
	std::uint32_t crc{0};
	for (auto i = 0; i < length; ++i) {
		updateCRC32(*buffer, crc);
		buffer++;
	}

	return crc;
}
}  // namespace crc