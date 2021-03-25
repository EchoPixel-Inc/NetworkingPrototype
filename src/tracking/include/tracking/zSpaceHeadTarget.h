#ifndef zSpaceHeadTarget_h
#define zSpaceHeadTarget_h

#include "tracking/headTargetInterface.h"

#include "zSpace.h"

#include <memory>
#include <shared_mutex>

namespace zspace
{
class ContextManager;
}

namespace tracking
{
class zSpaceHeadTarget : public HeadTargetInterface
{
public:
	explicit zSpaceHeadTarget(std::shared_ptr<::zspace::ContextManager>);
	virtual ~zSpaceHeadTarget();

	zSpaceHeadTarget(const zSpaceHeadTarget&) = delete;
	zSpaceHeadTarget& operator=(const zSpaceHeadTarget&) = delete;

	HeadPoseType getHeadPosition() const override;

protected:
	void onHeadMoved();

	static void onHeadMovedImpl(
		ZCHandle, const ZCTrackerEventData*, const void*);

private:
	std::shared_ptr<zspace::ContextManager> m_ContextManager;
	ZCHandle m_HeadTargetHandle;
	mutable std::shared_mutex m_Mutex;
	HeadPoseType m_HeadPose;
};
}  // namespace tracking

#endif