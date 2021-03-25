#include "tracking/zSpaceHeadTarget.h"
#include "zspace/zSpaceUtilities.h"
#include "zspace/zSpaceContextManager.h"

#include <iostream>

namespace tracking
{
	//=========================================================================
	zSpaceHeadTarget::zSpaceHeadTarget(std::shared_ptr<zspace::ContextManager> contextManager) : 
		m_ContextManager{contextManager},
		m_HeadPose{ DevicePoseType::Identity() }
	{
		assert(contextManager);

		int numTargets;
		if (auto error = zcGetNumTargetsByType(
				m_ContextManager->getContext(), ZCTargetType::ZC_TARGET_TYPE_PRIMARY, &numTargets);
			error != ZC_ERROR_OK) {
			throw std::runtime_error(zspace::getErrorString(error));
		}

		for (int i = 0; i < numTargets; i++) {
			ZCHandle headTargetHandle = nullptr;

			if (auto error = zcGetTargetByType(m_ContextManager->getContext(),
					ZCTargetType::ZC_TARGET_TYPE_HEAD, i, &headTargetHandle);
				error != ZC_ERROR_OK) {
				continue;
			}

			m_HeadTargetHandle = headTargetHandle;
		}

		if (m_HeadTargetHandle) {
			float headRotationThreshold = 0.1f;
			float headSampleTime = 0.016f;	// The time threshold in seconds.
			float dummy;
			float headMoveThreshold;

			if (auto error =
					zcGetTargetMoveEventThresholds(m_HeadTargetHandle,
						&dummy, &headMoveThreshold, &dummy);
				error != ZC_ERROR_OK) {
				std::cerr << zspace::getErrorString(error) << std::endl;
			}

			if (auto error = zcSetTargetMoveEventThresholds(
				m_HeadTargetHandle, headSampleTime, headMoveThreshold,
					headRotationThreshold);
				error != ZC_ERROR_OK) {
				std::cerr << zspace::getErrorString(error) << std::endl;
			}
		}
		else {
			throw std::runtime_error("head target not found");
		}

		if (auto error = zcAddTrackerEventHandler(m_HeadTargetHandle,
				ZCTrackerEventType::ZC_TRACKER_EVENT_MOVE,
				&zSpaceHeadTarget::onHeadMovedImpl, this);
			error != ZC_ERROR_OK) {
			std::cerr << zspace::getErrorString(error) << std::endl;
		}
	}
	//=========================================================================

	//=========================================================================
	zSpaceHeadTarget::~zSpaceHeadTarget()
	{
		if (auto error = zcRemoveTrackerEventHandler(m_HeadTargetHandle,
				ZCTrackerEventType::ZC_TRACKER_EVENT_MOVE,
				&zSpaceHeadTarget::onHeadMovedImpl, this);
			error != ZC_ERROR_OK) {
			std::cerr << zspace::getErrorString(error) << std::endl;
		}
	}
	//=========================================================================

	//=========================================================================
	void zSpaceHeadTarget::onHeadMovedImpl(ZCHandle handle, const ZCTrackerEventData* eventData, const void* userData)
	{
		auto headTarget = static_cast<const zSpaceHeadTarget*>(userData);

		const_cast<zSpaceHeadTarget*>(headTarget)->onHeadMoved();
	}
	//=========================================================================

	//=========================================================================
	void zSpaceHeadTarget::onHeadMoved()
	{
		if (auto error = zcUpdate(m_ContextManager->getContext());
			error != ZC_ERROR_OK) {
			std::cerr << zspace::getErrorString(error) << std::endl;
		}

		ZCTrackerPose zHeadPose;
		if (auto error = zcGetTargetPose(m_HeadTargetHandle, &zHeadPose);
			error != ZC_ERROR_OK) {
			std::cerr << zspace::getErrorString(error) << std::endl;

			return;
		}

		auto headPose = zspace::convertPose(zHeadPose);

		std::unique_lock<std::shared_mutex> lock(m_Mutex);
		m_HeadPose = headPose;
	}
	//=========================================================================

	//=========================================================================
	auto zSpaceHeadTarget::getHeadPosition() const -> HeadPoseType
	{
		std::shared_lock<std::shared_mutex> lock(m_Mutex);
		return m_HeadPose;
	}
	//=========================================================================
}  // namespace tracking