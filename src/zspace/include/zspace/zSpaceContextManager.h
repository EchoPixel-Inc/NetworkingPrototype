#ifndef zSpaceContextManager_h
#define zSpaceContextManager_h

#include "zSpace.h"
#include <memory>

namespace zspace
{
class ContextManager
{
public:
	ContextManager();
	~ContextManager();

	ZCContext getContext() const;

	static std::shared_ptr<ContextManager> defaultContextManager();

private:
	ZCContext m_Context;
};
}  // end namespace zspace
#endif