#include "display/displayInterface.h"
#include "display/genericDisplay.h"
#include "display/displayUtilities.h"
#include "display/barcoDisplay.h"
#include "barco/barcoSystem.h"

#ifdef USE_ZSPACE
#	include "display/zspaceDisplay.h"
#	include "zspace/zSpaceContextManager.h"
#endif

#include <iostream>
#include <algorithm>

namespace
{
std::shared_ptr<DisplayInterface> makeDefaultImplementation()
{
	std::shared_ptr<DisplayInterface> defaultImplementation;

	auto connectedDisplayInfoList = getConnectedDisplayInfo();
	if (connectedDisplayInfoList.empty()) {
		std::cerr << "Couldn't find any connected displays" << std::endl;
		return nullptr;
	}

	for (const auto& info : connectedDisplayInfoList) {
		if (info.manufacturer == "ZSpace" ||
            (info.manufacturer == "Hewlett Packard" && info.productName == "HP Zvr"))
        {
#ifdef USE_ZSPACE
			DisplayInfo zSpaceInfo = info;
			zSpaceInfo.stereoType = DisplayInfo::StereoType::TimeSequential;

			try {
				defaultImplementation =
					std::make_shared<zSpaceDisplay>(zSpaceInfo,
						zspace::ContextManager::defaultContextManager());
			}
			catch (std::exception& e) {
				std::cerr << e.what() << std::endl;
			}
			break;
#endif
		}
		else if ((info.manufacturer == "Barco") &&
			((info.size[0] == 3840) && (info.size[1] == 1080)) &&
			info.pixelSizeInMM.has_value()) {
			DisplayInfo barcoDisplayInfo = info;
			barcoDisplayInfo.stereoType = DisplayInfo::StereoType::SideBySide;

			try {
				defaultImplementation = std::make_shared<BarcoDisplay>(
					barcoDisplayInfo, BarcoSystem::getDefaultInstance());
			}
			catch (std::exception& e) {
				std::cerr << e.what() << std::endl;
			}
		}
	}

	if (!defaultImplementation) {
		// all else being equal, just pick the primary display
		if (auto it = std::find_if(connectedDisplayInfoList.begin(),
				connectedDisplayInfoList.end(),
				[](const DisplayInfo& info) {
					return (info.position == DisplayInfo::PointType{0, 0});
				});
			it != connectedDisplayInfoList.end()) {
			defaultImplementation = std::make_shared<GenericDisplay>(*it);
		}
		else {
			std::cerr << "Couldn't find a display at (0, 0)" << std::endl;
		}
	}

	return defaultImplementation;
}
}  // namespace

//==============================================================================
std::shared_ptr<DisplayInterface> DisplayInterface::getDefaultImplementation()
{
	static std::shared_ptr<DisplayInterface> defaultInterface =
		makeDefaultImplementation();

	return defaultInterface;
}
//==============================================================================