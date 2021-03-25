#ifndef displayInfo_h
#define displayInfo_h

#include <string>
#include <optional>
#include <array>
#include <iostream>

struct DisplayInfo
{
	using PointType = std::array<int, 2>;
	using SizeType = std::array<int, 2>;
	using FloatSizeType = std::array<double, 2>;

	enum class StereoType {
		NonStereo,
		TimeSequential,
		SideBySide,
		TopBottom,
		Interleaved
	};

	std::string manufacturer;
	std::optional<std::string> serialString;
	std::optional<std::string> productName;
	std::optional<std::string> productString;
	unsigned int serialNumber;
	int productCode;
	PointType position;
	SizeType size;
	std::optional<SizeType> sizeInMM;
	std::optional<FloatSizeType> pixelSizeInMM;
	StereoType stereoType = StereoType::NonStereo;
};

inline std::ostream& operator<<(std::ostream& os, const DisplayInfo& info)
{
	std::string indent{"  "};
	os << "manufacturer: " << info.manufacturer << "\n";
	os << "serial number: "
	   << (info.serialString.has_value() ? info.serialString.value() :
										   std::to_string(info.serialNumber))
	   << "\n";
	os << "product name: "
	   << (info.productName.has_value() ? info.productName.value() : "N/A")
	   << "\n";
	os << "product string: "
	   << (info.productString.has_value() ? info.productString.value() : "N/A")
	   << "\n";
	os << "product code: " << info.productCode << "\n";
	os << "position: (" << info.position[0] << ", " << info.position[1]
	   << ")\n";
	os << "size: (" << info.size[0] << ", " << info.size[1] << ")\n";

	std::stringstream sizeInMMStream;
	if (info.sizeInMM.has_value()) {
		sizeInMMStream << "(" << info.sizeInMM.value()[0] << ", "
					   << info.sizeInMM.value()[1] << ")";
	}
	else {
		sizeInMMStream << "N/A";
	}

	os << "size in [mm]: " << sizeInMMStream.str() << "\n";

	std::string stereoTypeString;
	switch (info.stereoType) {
		case DisplayInfo::StereoType::NonStereo:
			stereoTypeString.assign("non-stereo");
			break;
		case DisplayInfo::StereoType::TimeSequential:
			stereoTypeString.assign("time-sequential");
			break;
		case DisplayInfo::StereoType::SideBySide:
			stereoTypeString.assign("side-by-side");
			break;
		case DisplayInfo::StereoType::TopBottom:
			stereoTypeString.assign("top-bottom");
			break;
		case DisplayInfo::StereoType::Interleaved:
			stereoTypeString.assign("interleaved");
			break;
	}

	os << "stereo type: " << stereoTypeString;

	return os;
}

#endif