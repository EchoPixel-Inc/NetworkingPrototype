#ifndef displayUtilities_h
#define displayUtilities_h

#include "display/displayInterface.h"
#include "display/displayInfo.h"

#include <vector>
#include <memory>
#include <string>
#include <map>

std::vector<DisplayInfo> getConnectedDisplayInfo();

const std::map<std::string, std::string>& getDisplayVendorMap();

#endif