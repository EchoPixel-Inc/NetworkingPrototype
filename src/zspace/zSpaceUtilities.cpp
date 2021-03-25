#include "zspace/zSpaceUtilities.h"
#include "zspace/zSpaceContextManager.h"

#include <iostream>

namespace zspace
{
//=============================================================================
	std::string getErrorString(ZCError error)
	{
		char errorString[256];
		zcGetErrorString(error, errorString, sizeof(errorString));
		return std::string(errorString);
	}
	//=============================================================================

	//=============================================================================
	auto convertPose(const ZCTrackerPose& trackerPose) -> common::TransformType
	{
		auto zSpaceMtx = trackerPose.matrix;
		auto devicePose{ common::TransformType::Identity() };

		double metersToMillimeters{ 1.0e+03 };

		devicePose(0, 0) = zSpaceMtx.m00;
		devicePose(0, 1) = zSpaceMtx.m01;
		devicePose(0, 2) = zSpaceMtx.m02;
		devicePose(0, 3) = metersToMillimeters * zSpaceMtx.m03;

		devicePose(1, 0) = zSpaceMtx.m10;
		devicePose(1, 1) = zSpaceMtx.m11;
		devicePose(1, 2) = zSpaceMtx.m12;
		devicePose(1, 3) = metersToMillimeters * zSpaceMtx.m13;

		devicePose(2, 0) = zSpaceMtx.m20;
		devicePose(2, 1) = zSpaceMtx.m21;
		devicePose(2, 2) = zSpaceMtx.m22;
		devicePose(2, 3) = metersToMillimeters * zSpaceMtx.m23;

		devicePose(3, 0) = zSpaceMtx.m30;
		devicePose(3, 1) = zSpaceMtx.m31;
		devicePose(3, 2) = zSpaceMtx.m32;
		devicePose(3, 3) = zSpaceMtx.m33;

		return devicePose;
	}

} // end namespace zspace
//=============================================================================
//// creating buffer
//if (auto error = zcCreateStereoBuffer(ZSPACECONTEXT,
//	ZC_RENDERER_QUAD_BUFFER_GL, 0, &ZSTEREOBUFFER);
//	error != ZC_ERROR_OK) {
//	std::cerr << getErrorString(error) << std::endl;
//	return false;
//}
//bool isConnectedToZSpaceDisplay()
//{
//	if (!(ZSPACECONTEXT || zspaceInit())) {
//		std::cerr << "No zspace context available" << std::endl;
//
//		return false;
//	}
//
//	int numDisplays{0};
//	if (zcGetNumDisplays(ZSPACECONTEXT, &numDisplays) == ZC_ERROR_OK) {
//		ZCHandle displayHandle = nullptr;
//
//		for (int i = 0; i < numDisplays; i++) {
//			if (auto error = zcGetDisplayByIndex(
//					ZSPACECONTEXT, i, &displayHandle);
//				error != ZC_ERROR_OK) {
//				std::cerr << getErrorString(error) << std::endl;
//
//				continue;
//			}
//
//			ZCDisplayType displayType;
//			if (auto error =
//					zcGetDisplayType(displayHandle, &displayType);
//				error != ZC_ERROR_OK) {
//				std::cerr << getErrorString(error) << std::endl;
//
//				continue;
//			}
//
//			ZSBool isHardwarePresent;
//			if (auto error = zcIsDisplayHardwarePresent(
//					displayHandle, &isHardwarePresent);
//				error != ZC_ERROR_OK) {
//				std::cerr << getErrorString(error) << std::endl;
//
//				continue;
//			}
//
//			if (displayHandle &&
//				(displayType == ZC_DISPLAY_TYPE_ZSPACE) &&
//				isHardwarePresent) {
//				return true;
//			}
//		}
//	}
//
//	return false;
//}
////=====================================================================
//
////=====================================================================