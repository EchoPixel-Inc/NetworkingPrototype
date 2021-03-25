#include "clientApp/trackerEventProcessor.h"
#include "interaction/customQEvents.h"
#include "interaction/interactor.h"

#include <QEvent>

#include <iostream>
//=============================================================================
TrackerEventProcessor::TrackerEventProcessor(Interactor* iren, QObject* parent) :
	QObject{parent},
	m_Interactor{iren}
{}
//=============================================================================

//=============================================================================
TrackerEventProcessor::~TrackerEventProcessor() = default;
//=============================================================================

//=============================================================================
void TrackerEventProcessor::setInteractor(Interactor* iren)
{
	m_Interactor = iren;
}
//=============================================================================

//=============================================================================
bool TrackerEventProcessor::event(QEvent* event)
{
	if (m_Interactor) {
		switch (event->type()) {
		case CustomQEvents::DEVICE_MOVE: {
			auto moveEvent = static_cast<DeviceMoveEvent*>(event);
			m_Interactor->SetDevicePose(moveEvent->devicePose);
			m_Interactor->InvokeEvent(vtkCommand::Move3DEvent);
			break;
		}
		case CustomQEvents::DEVICE_BUTTONPRESS: {
			m_Interactor->InvokeEvent(vtkCommand::FifthButtonPressEvent);
			break;
		}
		case CustomQEvents::DEVICE_BUTTONRELEASE: {
			m_Interactor->InvokeEvent(vtkCommand::FifthButtonReleaseEvent);
			break;
		}
		} // end switch
	}

	return Superclass::event(event);
}
//=============================================================================

//=============================================================================