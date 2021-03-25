#ifndef trackingManager_h
#define trackingManager_h

#include "tracking/trackingTypes.h"
#include "common/coreTypes.h"

#include <memory>
#include <string>
#include <optional>
#include <mutex>

class TrackerEventProcessor;
class Interactor;

namespace tracking
{
	class InteractionDeviceInterface;
	class HeadTargetInterface;
}

class TrackingManager
{
public:
	using HeadPoseType = tracking::HeadPoseType;

	explicit TrackingManager();
	~TrackingManager();

	void setInteractor(Interactor*);

	void initializeHeadTracking(const std::string& headTargetType);
	void initializeInteractionDevice(const std::string& deviceType);

	void calibrateInteractionDevice();

	HeadPoseType getCurrentHeadPose() const;

private:
	struct InteractionDeviceResources {
		std::optional<common::TransformType> calibrationTransform;
		std::mutex mutex;
	};

	std::unique_ptr<tracking::InteractionDeviceInterface> m_InteractionDevice;
	std::unique_ptr<tracking::HeadTargetInterface> m_HeadTarget;
	std::unique_ptr<TrackerEventProcessor> m_EventProcessor;
	std::shared_ptr<InteractionDeviceResources> m_InteractionDeviceResources;
};

#endif