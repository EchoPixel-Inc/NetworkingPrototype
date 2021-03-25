#ifndef serializationTypes_h
#define serializationTypes_h

#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>

namespace serialization
{
	using InputArchiveType = cereal::JSONInputArchive;
	using OutputArchiveType = cereal::JSONOutputArchive;
}

#endif