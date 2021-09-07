#include "tracking/vrInkInteractionDevice.h"
#include "common/coreTypes.h"

#include "vr_ink_api.h"

#include <chrono>
#include <iostream>

namespace
{
static const common::TransformType lighthouseTransform = [] {
	common::TransformType lighthouseTransform{
		common::TransformType::Identity()};
	lighthouseTransform.matrix().col(0) *= -1.0;
	lighthouseTransform.matrix().col(2) *= -1.0;
	return lighthouseTransform;
}();
}

namespace tracking
{
//=============================================================================
VRInkInteractionDevice::VRInkInteractionDevice() : m_ThreadAbortFlag{false}
{
	std::uint8_t apiVersionMajor;
	std::uint8_t apiVersionMinor;
	std::uint8_t driverVersionMajor;
	std::uint8_t driverVersionMinor;

	VrInkApi::GetApiVersion(apiVersionMajor, apiVersionMinor);
	VrInkApi::GetDriverVersion(driverVersionMajor, driverVersionMinor);

	if ((driverVersionMajor == 0) && (driverVersionMinor == 0)) {
		throw std::runtime_error(
			"Could not locate Logitech VR Ink driver information.  "
			"Ensure both SteamVR and the Logitech VR Ink driver are running");
	}

	std::cout << "VR Ink API version: v" << apiVersionMajor << "."
			  << apiVersionMinor << "\n"
			  << "VR Ink Driver version: v" << driverVersionMajor << "."
			  << driverVersionMinor << std::endl;

	VrInkApi::DeviceInfo deviceInfo;
	VrInkApi::GetDeviceInfo(deviceInfo);

	// If the following variables are zero-valued, it means that the device
	// could not be successfully located
	if ((deviceInfo.firmwareVersion == 0) ||
		(deviceInfo.hardwareRevision == 0) || (deviceInfo.fpgaVersion == 0)) {
		throw std::runtime_error(
			"Could not retrieve Logitech VR Ink device information.  "
			"Ensure the Logitech VR Ink stylus is powered on and paired");
	}

	std::cout << "VR Ink device info:\n"
			  << "Model number: " << deviceInfo.modelNumber << "\n"
			  << "Serial number: " << deviceInfo.serialNumber << "\n"
			  << "Firmware version: " << deviceInfo.firmwareVersion << "\n"
			  << "Hardware revision: " << deviceInfo.hardwareRevision << "\n"
			  << "FPGA version: " << deviceInfo.fpgaVersion << "\n"
			  << "Radio version: " << deviceInfo.radioVersion << "\n"
			  << "Dongle version: " << deviceInfo.dongleVersion << "\n"
			  << "VRC version: " << deviceInfo.vrcVersion << std::endl;

	// Query the current device status; throw if the device is not connected
	VrInkApi::InkStatus deviceStatus;
	VrInkApi::GetDeviceStatus(deviceStatus);

	if (!deviceStatus.deviceIsConnected) {
		throw std::runtime_error("Logitech VR Ink device is not connected");
	}

	// Store the initial device status information
	m_DeviceInfo.pose.matrix() = lighthouseTransform.matrix() *
		Eigen::Map<Eigen::Matrix<float,
			DevicePoseType::MatrixType::RowsAtCompileTime,
			DevicePoseType::MatrixType::ColsAtCompileTime, Eigen::ColMajor>>(
			deviceStatus.poseMatrix)
			.cast<DevicePoseType::Scalar>();

	m_DeviceInfo.connected = deviceStatus.deviceIsConnected;

	m_UpdateThread = std::thread(&VRInkInteractionDevice::runUpdateLoop, this);
}
//=============================================================================

//=============================================================================
VRInkInteractionDevice::~VRInkInteractionDevice()
{
	m_ThreadAbortFlag.store(true);
	if (m_UpdateThread.joinable()) {
		m_UpdateThread.join();
	}
}
//=============================================================================

//=============================================================================
auto VRInkInteractionDevice::getPose() const -> DevicePoseType
{
	std::shared_lock<std::shared_mutex> deviceInfoLocker(m_DeviceMutex);
	return m_DeviceInfo.pose;
}
//=============================================================================

//=============================================================================
void VRInkInteractionDevice::setDeviceMovedCallback(MoveCallbackType clbk)
{
	std::lock_guard<std::mutex> lock(m_CallbackMutex);
	m_MoveCallback = clbk;
}
//=============================================================================

//=============================================================================
void VRInkInteractionDevice::setButtonPressCallback(
	ButtonPressCallbackType clbk)
{
	std::lock_guard<std::mutex> lock(m_CallbackMutex);
	m_ButtonPressCallback = clbk;
}
//=============================================================================

//=============================================================================
void VRInkInteractionDevice::setButtonReleaseCallback(
	ButtonReleaseCallbackType clbk)
{
	std::lock_guard<std::mutex> lock(m_CallbackMutex);
	m_ButtonReleaseCallback = clbk;
}
//=============================================================================

//=============================================================================
void VRInkInteractionDevice::runUpdateLoop()
{
	VrInkApi::InkStatus pastStatus;
	VrInkApi::InkStatus currentStatus;
	DevicePoseType currentPose = DevicePoseType::Identity();

	while (!m_ThreadAbortFlag) {
		VrInkApi::GetDeviceStatus(currentStatus);

		std::unique_lock<std::shared_mutex> deviceInfoLocker(m_DeviceMutex);
		m_DeviceInfo.connected = currentStatus.deviceIsConnected;

		// No need to proceed further if device is not connected
		if (!currentStatus.deviceIsConnected) {
			continue;
		}

		currentPose.matrix() = lighthouseTransform.matrix() *
			Eigen::Map<Eigen::Matrix<float,
				DevicePoseType::MatrixType::RowsAtCompileTime,
				DevicePoseType::MatrixType::ColsAtCompileTime,
				Eigen::ColMajor>>(currentStatus.poseMatrix)
				.cast<DevicePoseType::Scalar>();

		currentPose.translation() *= 1.0e+03; // [m] to [mm]

		m_DeviceInfo.pose = currentPose;
		deviceInfoLocker.unlock();

		std::unique_lock<std::mutex> callbackLocker(m_CallbackMutex);
		if (m_MoveCallback) {
			std::invoke(m_MoveCallback, currentPose);
		}

		if (currentStatus.touchstripClick != pastStatus.touchstripClick) {
			if (currentStatus.touchstripClick && m_ButtonPressCallback) {
				std::invoke(m_ButtonPressCallback);
			}
			else if (m_ButtonReleaseCallback) {
				std::invoke(m_ButtonReleaseCallback);
			}
		}

		if (currentStatus.primaryClick != pastStatus.primaryClick) {
			// reserved for additional button press functionality
		}

		if (currentStatus.applicationMenu != pastStatus.applicationMenu) {
			// reserved for additional button press functionality
		}

		callbackLocker.unlock();
		pastStatus = currentStatus;

		// Need to throttle the updates otherwise the event queue might be
		// overwhelmed
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
//=============================================================================

//=============================================================================
}  // end namespace tracking
