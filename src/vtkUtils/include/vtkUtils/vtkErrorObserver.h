#ifndef vtkErrorObserver_h
#define vtkErrorObserver_h

#include <vtkCommand.h>

#include <string>
#include <optional>

class vtkErrorObserver : public vtkCommand
{
public:
	static vtkErrorObserver* New();

	std::optional<std::string> lastErrorMessage;
	std::optional<std::string> lastWarningMessage;

	virtual void Execute(vtkObject* caller, unsigned long eventId,
		void* callData) override;

protected:
	vtkErrorObserver();
	vtkErrorObserver(vtkErrorObserver&) = delete;
	vtkErrorObserver& operator=(const vtkErrorObserver&) = delete;
};

#endif