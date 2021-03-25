#include "tracking/barcoHeadTargetBuilder.h"
#include "tracking/barcoHeadTarget.h"
#include "barco/barcoSystem.h"

namespace tracking
{
//=============================================================================
std::unique_ptr<HeadTargetInterface> BarcoHeadTargetBuilder::create() const
{
	return std::make_unique<BarcoHeadTarget>(BarcoSystem::getDefaultInstance());
}
//=============================================================================
}  // end namespace tracking