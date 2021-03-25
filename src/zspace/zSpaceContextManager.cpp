#include "zspace/ZSpaceContextManager.h"
#include "zspace/zSpaceUtilities.h"

#include <exception>
#include <iostream>

namespace {
	std::shared_ptr<zspace::ContextManager> makeDefaultContextManager()
	{
		std::shared_ptr<zspace::ContextManager> defaultContextManager{ nullptr };
		try {
			defaultContextManager = std::make_shared<zspace::ContextManager>();
		}
		catch (std::exception& e) {
			std::cerr << "Couldn't create z-space context manager" 
				<< e.what() << std::endl;
		}

		return defaultContextManager;
	}
}

namespace zspace
{
//==============================================================================
ContextManager::ContextManager() 
{
	if (auto error = zcInitialize(&m_Context); error != ZC_ERROR_OK) {
		throw std::runtime_error(getErrorString(error));
	}
}
//==============================================================================

//==============================================================================
ContextManager::~ContextManager() 
{
	if (auto error = zcShutDown(&m_Context); error != ZC_ERROR_OK) {
		std::cerr << getErrorString(error) << std::endl;
	}
}
//==============================================================================

//==============================================================================
auto ContextManager::getContext() const -> ZCContext
{
	return m_Context;
}
//==============================================================================

//==============================================================================
std::shared_ptr<ContextManager> ContextManager::defaultContextManager()
{
	static auto contextManager = makeDefaultContextManager();
	return contextManager;
}
//==============================================================================
}  // end namespace zspace