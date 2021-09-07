#include "tracking/zSpaceInteractionDevice.h"
#include "zspace/zSpaceContextManager.h"
#include "zspace/zSpaceUtilities.h"

#include <iostream>

namespace tracking
{
//==============================================================================
zSpaceInteractionDevice::zSpaceInteractionDevice(
	std::shared_ptr<zspace::ContextManager> contextManager) :
	m_ContextManager{contextManager},
	m_CurrentPose{DevicePoseType::Identity()}
{
	assert(contextManager);

	int numTargets;
	if (auto error = zcGetNumTargetsByType(m_ContextManager->getContext(),
			ZCTargetType::ZC_TARGET_TYPE_PRIMARY, &numTargets);
		error != ZC_ERROR_OK) {
		throw std::runtime_error(zspace::getErrorString(error));
	}

	for (int i = 0; i < numTargets; i++) {
		ZCHandle stylusTargetHandle = nullptr;

		if (auto error = zcGetTargetByType(m_ContextManager->getContext(),
				ZCTargetType::ZC_TARGET_TYPE_PRIMARY, i, &stylusTargetHandle);
			error != ZC_ERROR_OK) {
			continue;
		}

		m_StylusHandle = stylusTargetHandle;
	}

	if (m_StylusHandle) {
		float stylusRotThreshold = 0.001f;	// The angle threshold in degrees.
		float stylusMoveThreshold =
			0.00001f;  // The distance threshold in meters.
		float stylusSampleTime = 0.016f;  // The time threshold in seconds.

		if (auto error = zcSetTargetMoveEventThresholds(m_StylusHandle,
				stylusSampleTime, stylusMoveThreshold, stylusRotThreshold);
			error != ZC_ERROR_OK) {
			std::cerr << zspace::getErrorString(error) << std::endl;
		}
	}
	else {
		throw std::runtime_error("stylus not found");
	}

	if (auto error = zcAddTrackerEventHandler(m_StylusHandle,
			ZCTrackerEventType::ZC_TRACKER_EVENT_MOVE,
			&zSpaceInteractionDevice::onStylusMovedImpl, this);
		error != ZC_ERROR_OK) {
		std::cerr << zspace::getErrorString(error) << std::endl;
	}

	if (auto error = zcAddTrackerEventHandler(m_StylusHandle,
			ZCTrackerEventType::ZC_TRACKER_EVENT_BUTTON_PRESS,
			&zSpaceInteractionDevice::onButtonPressedImpl, this);
		error != ZC_ERROR_OK) {
		std::cerr << zspace::getErrorString(error) << std::endl;
	}

	if (auto error = zcAddTrackerEventHandler(m_StylusHandle,
			ZCTrackerEventType::ZC_TRACKER_EVENT_BUTTON_RELEASE,
			&zSpaceInteractionDevice::onButtonReleasedImpl, this);
		error != ZC_ERROR_OK) {
		std::cerr << zspace::getErrorString(error) << std::endl;
	}
}
//==============================================================================

//==============================================================================
zSpaceInteractionDevice::~zSpaceInteractionDevice()
{
	if (auto error = zcRemoveTrackerEventHandler(m_StylusHandle,
		ZCTrackerEventType::ZC_TRACKER_EVENT_MOVE,
		&zSpaceInteractionDevice::onStylusMovedImpl, this);
		error != ZC_ERROR_OK) {
		std::cerr << "Error removing zspace stylus move callback"
			<< std::endl;
	}

	if (auto error = zcRemoveTrackerEventHandler(m_StylusHandle,
		ZCTrackerEventType::ZC_TRACKER_EVENT_BUTTON_PRESS,
		&zSpaceInteractionDevice::onButtonPressedImpl, this);
		error != ZC_ERROR_OK) {
		std::cerr << "Error removing zspace stylus button press callback"
			<< std::endl;
	}

	if (auto error = zcRemoveTrackerEventHandler(m_StylusHandle,
		ZCTrackerEventType::ZC_TRACKER_EVENT_BUTTON_RELEASE,
		&zSpaceInteractionDevice::onButtonReleasedImpl, this);
		error != ZC_ERROR_OK) {
		std::cerr << "Error removing zspace stylus button release callback"
			<< std::endl;
	}
}
//==============================================================================

//==============================================================================
auto zSpaceInteractionDevice::getPose() const -> DevicePoseType
{
	std::shared_lock<std::shared_mutex> lock(m_Mutex);
	return m_CurrentPose;
}
//==============================================================================

//==============================================================================
void zSpaceInteractionDevice::onStylusMovedImpl(
	ZCHandle handle, const ZCTrackerEventData* eventData, const void* userData)
{
	auto interactionDevice =
		static_cast<const zSpaceInteractionDevice*>(userData);

	const_cast<zSpaceInteractionDevice*>(interactionDevice)->onStylusMoved();
}
//==============================================================================

//==============================================================================
void zSpaceInteractionDevice::onButtonPressedImpl(
	ZCHandle handle, const ZCTrackerEventData* eventData, const void* userData)
{
	auto interactionDevice =
		static_cast<const zSpaceInteractionDevice*>(userData);

	if (eventData->buttonId == 0) {
		const_cast<zSpaceInteractionDevice*>(interactionDevice)
			->onButtonPressed();
	}
}
//==============================================================================

//==============================================================================
void zSpaceInteractionDevice::onButtonReleasedImpl(
	ZCHandle handle, const ZCTrackerEventData* eventData, const void* userData)
{
	auto interactionDevice =
		static_cast<const zSpaceInteractionDevice*>(userData);

	if (eventData->buttonId == 0) {
		const_cast<zSpaceInteractionDevice*>(interactionDevice)
			->onButtonReleased();
	}
}
//==============================================================================

//==============================================================================
void zSpaceInteractionDevice::setDeviceMovedCallback(
	MoveCallbackType moveCallback)
{
	m_StylusMovedCallback = moveCallback;
}
//==============================================================================

//==============================================================================
void zSpaceInteractionDevice::setButtonPressCallback(
	ButtonPressCallbackType buttonPressCallback)
{
	m_StylusButtonPressedCallback = buttonPressCallback;
}
//==============================================================================

//==============================================================================
void zSpaceInteractionDevice::setButtonReleaseCallback(
	ButtonReleaseCallbackType buttonReleaseCallback)
{
	m_StylusButtonReleasedCallback = buttonReleaseCallback;
}
//==============================================================================

//==============================================================================
void zSpaceInteractionDevice::onStylusMoved()
{
	if (auto error = zcUpdate(m_ContextManager->getContext());
		error != ZC_ERROR_OK) {
		std::cerr << zspace::getErrorString(error) << std::endl;
	}

	ZCTrackerPose stylusPose;
	if (auto error = zcGetTargetPose(m_StylusHandle, &stylusPose);
		error != ZC_ERROR_OK) {
		std::cerr << zspace::getErrorString(error) << std::endl;

		return;
	}

	auto currentPose = zspace::convertPose(stylusPose);

	{
		std::lock_guard<std::shared_mutex> locker(m_Mutex);
		m_CurrentPose = currentPose;
	}

	if (m_StylusMovedCallback) {
		m_StylusMovedCallback(currentPose);
	}

    std::cout << "ZSpace stylus moved - " << currentPose.data[3] << " " << currentPose.data[7] << " " << currentPose.data[11] << " " << currentPose.data[15] << std::endl;
}
//==============================================================================

//==============================================================================
void zSpaceInteractionDevice::onButtonPressed()
{
	if (m_StylusButtonPressedCallback) {
		m_StylusButtonPressedCallback();
	}
}
//==============================================================================

//==============================================================================
void zSpaceInteractionDevice::onButtonReleased()
{
	if (m_StylusButtonReleasedCallback) {
		m_StylusButtonReleasedCallback();
	}
}
//==============================================================================
}  // end namespace tracking