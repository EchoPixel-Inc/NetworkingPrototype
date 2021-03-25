#include "display/zSpaceDisplay.h"
#include "zspace/zSpaceContextManager.h"
#include "zspace/zSpaceUtilities.h"

#include <Eigen/Geometry>

#include <iostream>

constexpr double pi = 3.141592653;

//=============================================================================
zSpaceDisplay::zSpaceDisplay(const DisplayInfo& displayInfo,
	std::shared_ptr<zspace::ContextManager> contextManager) :
	m_Info{displayInfo},
	m_ContextManager{contextManager},
	m_DisplayHandle{nullptr},
	m_StereoBufferHandle{nullptr}
{
	assert(contextManager);

	int numDisplays{0};
	if (auto error =
			zcGetNumDisplays(m_ContextManager->getContext(), &numDisplays);
		error != ZC_ERROR_OK) {
		auto errorString = zspace::getErrorString(error);

		throw std::runtime_error(errorString);
	}

	for (int displayIndex = 0; displayIndex < numDisplays; displayIndex++) {
		ZCHandle displayHandle;

		if (zcGetDisplayByIndex(m_ContextManager->getContext(), displayIndex,
				&displayHandle) != ZC_ERROR_OK) {
			continue;
		}

		ZCDisplayType displayType;
		if (zcGetDisplayType(displayHandle, &displayType) != ZC_ERROR_OK) {
			continue;
		}

		ZSBool isHardwarePresent;
		if (zcIsDisplayHardwarePresent(displayHandle, &isHardwarePresent) !=
			ZC_ERROR_OK) {
			continue;
		}

		if (displayHandle && (displayType == ZC_DISPLAY_TYPE_ZSPACE) &&
			isHardwarePresent) {
			m_DisplayHandle = displayHandle;
			break;
		}
	}

	if (!m_DisplayHandle) {
		throw std::runtime_error("No zSpace displays detected...");
	}

	if (auto error = zcCreateStereoBuffer(m_ContextManager->getContext(),
			ZC_RENDERER_QUAD_BUFFER_GL, 0, &m_StereoBufferHandle);
		error != ZC_ERROR_OK) {
		auto errorString = zspace::getErrorString(error);
		throw std::runtime_error(errorString);
	}
}
//=============================================================================

//=============================================================================
zSpaceDisplay::~zSpaceDisplay() = default;
//=============================================================================

//=============================================================================
auto zSpaceDisplay::getInfo() const -> const DisplayInfo&
{
	return m_Info;
}
//=============================================================================

//=============================================================================
auto zSpaceDisplay::getPhysicalPose() const -> PoseType
{
	PoseType displayPose = PoseType::Identity();

	float angleX{0.0f};	 // pitch axis
	float angleY{0.0f};	 // yaw axis
	float angleZ{0.0f};	 // roll axis

	auto degreesToRadians = [](double angleInDegrees) -> double {
		return (angleInDegrees * pi / 360.0);
	};

	if (zcGetDisplayAngle(m_DisplayHandle, &angleX, &angleY, &angleZ) ==
		ZC_ERROR_OK) {

		angleX -= 90.0; // zspace convention is to give this angle assuming
		// that the screen starts flat on a table

		Eigen::AngleAxisd rollAngle(
			degreesToRadians(angleZ), Eigen::Vector3d::UnitZ());
		Eigen::AngleAxisd yawAngle(
			degreesToRadians(angleY), Eigen::Vector3d::UnitY());
		Eigen::AngleAxisd pitchAngle(
			degreesToRadians(angleX), Eigen::Vector3d::UnitX());

		Eigen::Quaterniond q = yawAngle * pitchAngle * rollAngle;
		displayPose.linear() = q.matrix();
	}

	return displayPose;
}
//=============================================================================

//=============================================================================
void zSpaceDisplay::syncStereoBuffers()
{
	if (auto error = zcSyncStereoBuffer(m_StereoBufferHandle);
		error != ZC_ERROR_OK) {
		auto errorString = zspace::getErrorString(error);
		std::cerr << "Couldn't sync zspace stereo buffer: " << errorString
				  << std::endl;
	}
}
//=============================================================================

//=============================================================================
void zSpaceDisplay::sendEyePositions(const EyePositionsType& eyePositions) {}
//=============================================================================