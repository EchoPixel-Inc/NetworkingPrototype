#ifndef barcoHeadTarget_h
#define barcoHeadTarget_h

#include "tracking/headTargetInterface.h"

#include <memory>

class BarcoSystem;

namespace tracking
{
class BarcoHeadTarget : public HeadTargetInterface
{
public:
	explicit BarcoHeadTarget(std::shared_ptr<BarcoSystem>);
	virtual ~BarcoHeadTarget();

	BarcoHeadTarget(const BarcoHeadTarget&) = delete;
	BarcoHeadTarget& operator=(const BarcoHeadTarget&) = delete;

	HeadPoseType getHeadPosition() const override;

private:
	std::shared_ptr<BarcoSystem> m_BarcoSystem;
};
}  // namespace tracking

#endif