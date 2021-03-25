#include "tracking/zSpaceHeadTargetBuilder.h"

#ifdef USE_ZSPACE
#	include "zspace/zSpaceContextManager.h"
#	include "tracking/zSpaceHeadTarget.h"
#endif

namespace tracking
{
//==============================================================================
std::unique_ptr<HeadTargetInterface> zSpaceHeadTargetBuilder::create() const 
{
	std::unique_ptr<HeadTargetInterface> headTarget;

#ifdef USE_ZSPACE
	auto contextManager = zspace::ContextManager::defaultContextManager();
	if (!contextManager) {
		throw std::runtime_error("No zspace context available");
	}

	headTarget = std::make_unique<zSpaceHeadTarget>(contextManager);
#endif

	return headTarget;
}
//==============================================================================
}  // end namespace tracking