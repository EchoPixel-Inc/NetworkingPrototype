#ifndef vtkGeneralizedCallbackCommand_h
#define vtkGeneralizedCallbackCommand_h

#include <vtkCommand.h>

#include <functional>

class vtkGeneralizedCallbackCommand : public vtkCommand
{
public:
	vtkTypeMacro(vtkGeneralizedCallbackCommand, vtkCommand);
	static vtkGeneralizedCallbackCommand* New()
	{
		return new vtkGeneralizedCallbackCommand;
	}

	void Execute(vtkObject* caller, unsigned long eid, void* callData) override
	{
		if (m_Callback) {
			m_Callback(caller, eid, callData);
		}
	}

	template <typename TCallback>
	void setCallback(TCallback&& callback)
	{
		m_Callback = callback;
	}

protected:
	vtkGeneralizedCallbackCommand() = default;
	~vtkGeneralizedCallbackCommand() override = default;

private:
	std::function<void(vtkObject*, unsigned long, void*)> m_Callback;
};

#endif