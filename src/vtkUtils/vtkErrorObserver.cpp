#include "vtkUtils/vtkErrorObserver.h"

#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkErrorObserver);

//=============================================================================
vtkErrorObserver::vtkErrorObserver() = default;
//=============================================================================

//=============================================================================
void vtkErrorObserver::Execute(vtkObject * caller, unsigned long eventId,
	void* callData)
{
	switch (eventId) {
	case vtkCommand::ErrorEvent: {
		lastErrorMessage = std::string(static_cast<char*>(callData));
		break;
	}
	case vtkCommand::WarningEvent: {
		lastWarningMessage = std::string(static_cast<char*>(callData));
		break;
	}
	}
}
//=============================================================================
