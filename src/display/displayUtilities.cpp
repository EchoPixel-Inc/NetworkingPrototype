#include "display/displayUtilities.h"
#include "display/genericDisplay.h"
#include "display/edidInfo.h"
#include "display/displayInfo.h"

#include <QScreen>
#include <QApplication>

#ifdef WIN32
#	pragma comment(lib, "Setupapi.lib")
#	include "Windows.h"
#	include "setupapi.h"
#	include "tchar.h"
#endif

#include <iostream>
#include <map>
#include <string>

// Forward declaration
bool decode_edid(const unsigned char* data, EDIDInfo&);

namespace
{
const std::map<std::string, std::string>& getDisplayVendorMap()
{
	static const std::map<std::string, std::string> vendorMap = {
		{"AAA", "Avolites Ltd"}, {"ACI", "Ancor Communications Inc"},
		{"ACR", "Acer Technologies"}, {"ACT", "Applied Creative Technology"},
		{"ADA", "Addi-Data GmbH"}, {"AGO", "AlgolTek, Inc."},
		{"API", "A Plus Info Corporation"}, {"APP", "Apple Computer Inc"},
		{"ARD", "AREC Inc."}, {"ART", "Corion Industrial Corporation"},
		{"ASK", "Ask A/S"}, {"ATO", "ASTRO DESIGN, INC."},
		{"AUO", "DO NOT USE - AUO"}, {"AUS", "ASUSTek COMPUTER INC"},
		{"AVT", "Avtek (Electronics) Pty Ltd"}, {"BDS", "Barco"},
		{"BEL", "Beltronic Industrieelektronik GmbH"},
		{"BMD", "Blackmagic Design"}, {"BNO", "Bang & Olufsen"}, {"BOE", "BOE"},
		{"BPS", "Barco, N.V."}, {"CAT", "Consultancy in Advanced Technology"},
		{"CHR", "christmann informationstechnik + medien GmbH & Co. KG"},
		{"CIN", "Citron GmbH"}, {"CMN", "Chimei Innolux Corporation"},
		{"CMO", "Chi Mei Optoelectronics corp."},
		{"CPL", "Compal Electronics Inc"}, {"CPT", "cPATH"},
		{"CRO", "Extraordinary Technologies PTY Limited"},
		{"CTX", "Creatix Polymedia GmbH"}, {"CUK", "Calibre UK Ltd"},
		{"DEL", "Dell Inc."}, {"DGC", "Data General Corporation"},
		{"DON", "DENON, Ltd."}, {"EGA", "Elgato Systems LLC"},
		{"ENC", "Eizo Nanao Corporation"}, {"EPH", "Epiphan Systems Inc."},
		{"EXN", "RGB Systems, Inc. dba Extron Electronics"},
		{"EXP", "Data Export Corporation"}, {"FNI", "Funai Electric Co., Ltd."},
		{"FUS", "Fujitsu Siemens Computers GmbH"}, {"GFN", "Gefen Inc."},
		{"GGL", "Google Inc."}, {"GSM", "Goldstar Company Ltd"},
		{"HIQ", "Kaohsiung Opto Electronics Americas, Inc."},
		{"HOL", "Holoeye Photonics AG"}, {"HPN", "HP Inc."},
		{"HSD", "HannStar Display Corp"}, {"HTC", "Hitachi Ltd"},
		{"HWP", "Hewlett Packard"}, {"HYT", "Heng Yu Technology (HK) Limited"},
		{"IFZ", "ZSpace"}, {"INO", "Innolab Pte Ltd"},
		{"INT", "Interphase Corporation"},
		{"INX", "Communications Supply Corporation (A division of WESCO)"},
		{"ITE", "Integrated Tech Express Inc"}, {"IVM", "Iiyama North America"},
		{"JVC", "JVC"}, {"KTC", "Kingston Tech Corporation"},
		{"LEN", "Lenovo Group Limited"}, {"LNX", "The Linux Foundation"},
		{"LPL", "DO NOT USE - LPL"}, {"LWR", "Lightware Visual Engineering"},
		{"MAX", "Rogen Tech Distribution Inc"}, {"MEG", "Abeam Tech Ltd"},
		{"MEI", "Panasonic Industry Company"},
		{"MEL", "Mitsubishi Electric Corporation"},
		{"MJI", "MARANTZ JAPAN, INC."}, {"MST", "MS Telematica"},
		{"MSX", "Micomsoft Co., Ltd."}, {"MTC", "Mars-Tech Corporation"},
		{"MTX", "Matrox"}, {"NCP", "Najing CEC Panda FPD Technology CO. ltd"},
		{"NCR", "NCR Electronics"}, {"NEC", "NEC Corporation"},
		{"NEX", "Nexgen Mediatech Inc.,"}, {"ONK", "ONKYO Corporation"},
		{"ORN", "ORION ELECTRIC CO., LTD."}, {"OTM", "Optoma Corporation"},
		{"OVR", "Oculus VR, Inc."}, {"PAR", "Parallan Comp Inc"},
		{"PCC", "PowerCom Technology Company Ltd"},
		{"PHL", "Philips Consumer Electronics Company"},
		{"PIO", "Pioneer Electronic Corporation"}, {"PLY", "Polycom Inc."},
		{"PNR", "Planar Systems, Inc."}, {"QDS", "Quanta Display Inc."},
		{"RAT", "Rent-A-Tech"}, {"REN", "Renesas Technology Corp."},
		{"SAM", "Samsung Electric Company"}, {"SAN", "Sanyo Electric Co.,Ltd."},
		{"SEC", "Seiko Epson Corporation"}, {"SGT", "Stargate Technology"},
		{"SHP", "Sharp Corporation"}, {"SII", "Silicon Image, Inc."},
		{"SIS", "Silicon Integrated Systems Corporation"}, {"SNY", "Sony"},
		{"STD", "STD Computer Inc"}, {"STN", "Samsung Electronics America"},
		{"SVS", "SVSI"}, {"SYN", "Synaptics Inc"},
		{"TAI", "Toshiba America Info Systems Inc"},
		{"TCL", "Technical Concepts Ltd"}, {"TDC", "Teradici"},
		{"TOP", "Orion Communications Co., Ltd."},
		{"TOS", "Toshiba Corporation"},
		{"TSB", "Toshiba America Info Systems Inc"}, {"TST", "Transtream Inc"},
		{"UNK", "Unknown"},
		{"VES", "Vestel Elektronik Sanayi ve Ticaret A. S."},
		{"VID", "Ingram Macrotron Germany"}, {"VIT", "Visitech AS"},
		{"VIZ", "VIZIO, Inc"}, {"VSC", "ViewSonic Corporation"},
		{"VTK", "Viewteck Co., Ltd."},
		{"WDE", "Westinghouse Digital Electronics"}, {"XLX", "Xilinx, Inc."},
		{"YMH", "Yamaha Corporation"}};

	return vendorMap;
}

DisplayInfo displayInfoFromEDID(const EDIDInfo& edidInfo)
{
	DisplayInfo displayInfo;

	const auto& displayVendorMap = getDisplayVendorMap();
	if (auto it = displayVendorMap.find(edidInfo.manufacturer_code);
		it != displayVendorMap.end()) {
		displayInfo.manufacturer = it->second;
	}

	displayInfo.serialNumber = edidInfo.serial_number;
	displayInfo.productCode = edidInfo.product_code;
	displayInfo.serialString = edidInfo.dsc_serial_number;
	displayInfo.productName = edidInfo.dsc_product_name;
	displayInfo.productString = edidInfo.dsc_string;

	if (edidInfo.n_detailed_timings > 0) {
		switch (edidInfo.detailed_timings[0].stereo) {
			case EDIDInfo::StereoType::NO_STEREO: {
				break;
			}
			case EDIDInfo::StereoType::FIELD_RIGHT: {
			}
			case EDIDInfo::StereoType::FIELD_LEFT: {
				break;
			}
			case EDIDInfo::StereoType::TWO_WAY_RIGHT_ON_EVEN: {
			}
			case EDIDInfo::StereoType::TWO_WAY_LEFT_ON_EVEN: {
				break;
			}
			case EDIDInfo::StereoType::FOUR_WAY_INTERLEAVED: {
				break;
			}
			case EDIDInfo::StereoType::SIDE_BY_SIDE: {
				break;
			}
		}
	}

	if (edidInfo.width_mm.has_value() && edidInfo.height_mm.has_value()) {
		displayInfo.sizeInMM = DisplayInfo::SizeType(
			{edidInfo.width_mm.value(), edidInfo.height_mm.value()});
	}

	return displayInfo;
}

#ifdef WIN32
bool getEDIDInfoFromRegistry(
	HDEVINFO hInfo, SP_DEVINFO_DATA* pDevInfoData, EDIDInfo& edidInfo)
{
	bool success{false};

	// now we can open the registry key
	HKEY hEDIDRegKey = SetupDiOpenDevRegKey(
		hInfo, pDevInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);

	if (hEDIDRegKey && (hEDIDRegKey != INVALID_HANDLE_VALUE)) {
		BYTE EDIDdata[256];
		DWORD edidSize = sizeof(EDIDdata);

		if (RegQueryValueEx(hEDIDRegKey, _T("EDID"), NULL, NULL,
				(LPBYTE)&EDIDdata, &edidSize) == ERROR_SUCCESS) {
			success = decode_edid(EDIDdata, edidInfo);
		}

		RegCloseKey(hEDIDRegKey);
	}

	return success;
}

bool getDisplayDeviceInfo(const std::string& deviceID, EDIDInfo& info)
{
	const GUID GUID_DEVINTERFACE_MONITOR = {0xe6f07b5f, 0xee97, 0x4a90, 0xb0,
		0x76, 0x33, 0xf5, 0x7b, 0xf4, 0xea, 0xa7};

	HDEVINFO devInfo = SetupDiGetClassDevs(
		&GUID_DEVINTERFACE_MONITOR, NULL, NULL, DIGCF_DEVICEINTERFACE);

	if (devInfo == NULL) {
		return false;
	}

	SP_DEVINFO_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof(deviceInfoData);

	auto success{false};

	for (DWORD dwDeviceIndex = 0;
		 SetupDiEnumDeviceInfo(devInfo, dwDeviceIndex, &deviceInfoData);
		 dwDeviceIndex++) {
		SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
		deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		for (DWORD dwMemberIndex = 0; SetupDiEnumDeviceInterfaces(devInfo,
				 &deviceInfoData, &GUID_DEVINTERFACE_MONITOR, dwMemberIndex,
				 &deviceInterfaceData);
			 dwMemberIndex++) {
			DWORD dwDeviceInterfaceDetailDataSize =
				offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA, DevicePath) +
				MAX_PATH * sizeof(TCHAR);
			PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData =
				(PSP_DEVICE_INTERFACE_DETAIL_DATA) new BYTE
					[dwDeviceInterfaceDetailDataSize];
			pDeviceInterfaceDetailData->cbSize =
				sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (!SetupDiGetDeviceInterfaceDetail(devInfo, &deviceInterfaceData,
					pDeviceInterfaceDetailData, dwDeviceInterfaceDetailDataSize,
					NULL, NULL)) {
				return false;
			}

			if (stricmp(pDeviceInterfaceDetailData->DevicePath,
					deviceID.c_str()) == 0) {
				success =
					getEDIDInfoFromRegistry(devInfo, &deviceInfoData, info);
			}

			delete[] pDeviceInterfaceDetailData;
		}
	}

	return success;
}
}  // end anonymouse namespace
#endif

std::vector<DisplayInfo> getConnectedDisplayInfo()
{
	std::vector<DisplayInfo> displayInfoList;

#ifdef WIN32
	DISPLAY_DEVICE displayAdapter;
	displayAdapter.cb = sizeof(displayAdapter);
	DWORD adapterIndex = 0;	 // device index

	while (EnumDisplayDevices(
		NULL, adapterIndex, &displayAdapter, EDD_GET_DEVICE_INTERFACE_NAME)) {
		DISPLAY_DEVICE displayDevice;
		ZeroMemory(&displayDevice, sizeof(displayDevice));
		displayDevice.cb = sizeof(displayDevice);
		DWORD displayIndex = 0;

		while (EnumDisplayDevices(displayAdapter.DeviceName, displayIndex,
			&displayDevice, EDD_GET_DEVICE_INTERFACE_NAME)) {
			if (displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE &&
				!(displayDevice.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER)) {

				EDIDInfo edidInfo;
				if (getDisplayDeviceInfo(
						std::string(displayDevice.DeviceID), edidInfo)) {
					DEVMODE deviceMode;
					ZeroMemory(&deviceMode, sizeof(deviceMode));
					deviceMode.dmSize = sizeof(deviceMode);
					DWORD iModNum = 0;

					if (EnumDisplaySettings(displayAdapter.DeviceName,
							ENUM_CURRENT_SETTINGS, &deviceMode)) {
						auto displayInfo = displayInfoFromEDID(edidInfo);
						displayInfo.size[0] = deviceMode.dmPelsWidth;
						displayInfo.size[1] = deviceMode.dmPelsHeight;
						displayInfo.position[0] = deviceMode.dmPosition.x;
						displayInfo.position[1] = deviceMode.dmPosition.y;

						// set the pixel pitch convenience variable if we have
						// access to the display physical dimensions
						if (displayInfo.sizeInMM.has_value()) {
							displayInfo.pixelSizeInMM = {
								static_cast<double>(
									displayInfo.sizeInMM.value()[0]) /
									displayInfo.size[0],
								static_cast<double>(
									displayInfo.sizeInMM.value()[1]) /
									displayInfo.size[1]};
						}

						displayInfoList.push_back(displayInfo);
					}
				}
			}

			ZeroMemory(&displayDevice, sizeof(displayDevice));
			displayDevice.cb = sizeof(displayDevice);
			displayIndex++;
		}

		ZeroMemory(&displayAdapter, sizeof(displayAdapter));
		displayAdapter.cb = sizeof(displayAdapter);
		adapterIndex++;
	}
#endif

	return displayInfoList;
}
