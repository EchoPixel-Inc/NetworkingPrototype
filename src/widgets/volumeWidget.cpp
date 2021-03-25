#include "widgets/volumeWidget.h"
#include "interaction/interactor.h"
#include "vtkUtils/vtkGeneralizedCallbackCommand.h"
#include "vtkUtils/vtkCommonConversions.h"

#include <vtkVolume.h>
#include <vtkVolumePicker.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkProp3DCollection.h>
#include <vtkPoints.h>
#include <vtkProp3D.h>
#include <vtkRenderer.h>
#include <vtkPropCollection.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPlane.h>
#include <vtkAbstractVolumeMapper.h>

//=============================================================================
VolumeWidget::VolumeWidget() : VolumeWidget(vtkVolume::New()) {}  // delegate
//=============================================================================

//=============================================================================
VolumeWidget::VolumeWidget(vtkVolume* volume) :
	m_Volume{volume},
	m_CallbackCommand{vtkSmartPointer<vtkGeneralizedCallbackCommand>::New()},
	m_VolumePicker{vtkSmartPointer<vtkVolumePicker>::New()},
	m_InteractionState{InteractionState::INACTIVE},
	m_ProcessEvents{false}
{
	assert(volume);

	// Make sure we have the basic volume sub-objects
	if (!m_Volume->GetUserMatrix()) {
		vtkNew<vtkMatrix4x4> userMatrix;
		userMatrix->Identity();
		m_Volume->SetUserMatrix(userMatrix);
	}

	m_Volume->SetPickable(true);
	m_VolumePicker->SetVolumeOpacityIsovalue(0.1);
	m_VolumePicker->PickClippingPlanesOff();
	//m_VolumePicker->PickFromListOn();
	//m_VolumePicker->AddPickList(m_Volume);

	m_CallbackCommand->setCallback(
		[this](vtkObject* caller, unsigned long eventId,
			void* vtkNotUsed(callData)) { this->processEvents(eventId); });
}
//=============================================================================

//=============================================================================
VolumeWidget::~VolumeWidget()
{
	setInteractor(nullptr);
}
//=============================================================================

//=============================================================================
void VolumeWidget::setVolume(vtkVolume* volume)
{
	if (!volume) {
		return;
	}

	if (m_Volume) {
		if (m_Interactor) {
			m_Interactor->GetRenderWindow()->GetRenderers()->
				GetFirstRenderer()->RemoveVolume(m_Volume);

			m_Interactor->GetRenderWindow()
				->GetRenderers()
				->GetFirstRenderer()
				->AddVolume(volume);
		}

		volume->SetUserTransform(m_Volume->GetUserTransform());
	}

	m_Volume = volume;

}
//=============================================================================

//=============================================================================
auto VolumeWidget::getVolume() const -> vtkVolume* const
{
	return m_Volume;
}
//=============================================================================

//=============================================================================
void VolumeWidget::updateProperties(const PropertyListType& propList)
{
	PropertyListType updatedProps;

	for (const auto& elem : propList) {
		auto& [propName, propValue] = elem;
		if (propName == "transform"){
			const auto& transform = std::get<common::TransformType>(propValue);

			setTransformInternal(transform);
			updatedProps.push_back({ propName, propValue });
		}
	}

	emit propertyUpdated(updatedProps);
}
//=============================================================================

//=============================================================================
void VolumeWidget::setInteractor(Interactor* iren)
{
	if (m_Interactor == iren) {
		return;
	}

	if (m_Interactor) {
		m_Interactor->RemoveObserver(m_CallbackCommand);
		if (auto renderer = m_Interactor->GetRenderWindow()
			->GetRenderers()
			->GetFirstRenderer()) {
			renderer->RemoveVolume(m_Volume);
		}
	}

	m_Interactor = iren;
	m_ProcessEvents = false;

	if (m_Interactor) {
		if (auto renderer = m_Interactor->GetRenderWindow()
			->GetRenderers()
			->GetFirstRenderer()) {
			renderer->AddVolume(m_Volume);
		}
	}
}
//=============================================================================

//=============================================================================
Interactor* VolumeWidget::getInteractor() const
{
	return m_Interactor;
}
//=============================================================================

//=============================================================================
void VolumeWidget::setTransform(const TransformType& transform)
{
	emit requestPropertyUpdate({{"transform", transform}});
}
//=============================================================================

//=============================================================================
auto VolumeWidget::getTransform() const -> TransformType
{
	auto transformMatrix = convertVTKToCommonMatrix(m_Volume->GetUserMatrix());
	TransformType transform;
	transform.matrix() = transformMatrix;

	return transform;
}
//=============================================================================

//=============================================================================
void VolumeWidget::setTransformInternal(const TransformType& transform)
{
	if (m_Volume) {
		m_Volume->GetUserMatrix()->DeepCopy(transform.matrix().data());

		emit transformChanged(transform);	
	}
}
//=============================================================================

//=============================================================================
void VolumeWidget::attach(const WidgetInterface* widget) {
	// no-op for volumes
}
//=============================================================================

//=============================================================================
void VolumeWidget::detach(){
	// no-op for volumes
}
//=============================================================================

//=============================================================================
void VolumeWidget::setProcessEvents(bool process)
{
	if (!m_Interactor) {
		m_ProcessEvents = false;
		return;
	}

	if (process == m_ProcessEvents) {
		return;
	}

	m_ProcessEvents = process;

	if (m_ProcessEvents) {
		m_Interactor->AddObserver(vtkCommand::Move3DEvent, m_CallbackCommand);

		m_Interactor->AddObserver(
			vtkCommand::FifthButtonPressEvent, m_CallbackCommand);

		m_Interactor->AddObserver(
			vtkCommand::FifthButtonReleaseEvent, m_CallbackCommand);

		if (auto renderer = m_Interactor->GetRenderWindow()
			->GetRenderers()
			->GetFirstRenderer()) {
			renderer->AddVolume(m_Volume);
		}
	}
	else {
		m_Interactor->RemoveObserver(m_CallbackCommand);
	}
}
//=============================================================================

//=============================================================================
bool VolumeWidget::getProcessEvents() const
{
	return m_ProcessEvents;
}
//=============================================================================

//=============================================================================
void VolumeWidget::setPickable(bool pickable)
{
	if (m_Volume) {
		m_Volume->SetPickable(pickable);
	}
}
//=============================================================================

//=============================================================================
bool VolumeWidget::getPickable() const
{
	if (m_Volume) {
		return m_Volume->GetPickable();
	}
	else {
		return false;
	}
}
//=============================================================================

//=============================================================================
void VolumeWidget::addClippingPlane(vtkPlane* plane)
{
	if (m_Volume && m_Volume->GetMapper()) {
		m_Volume->GetMapper()->AddClippingPlane(plane);
	}
}
//=============================================================================

//=============================================================================
void VolumeWidget::removeClippingPlane(vtkPlane* plane)
{
	if (m_Volume && m_Volume->GetMapper()) {
		m_Volume->GetMapper()->RemoveClippingPlane(plane);
	}
}
//=============================================================================

//=============================================================================
void VolumeWidget::removeAllClippingPlanes()
{
	if (m_Volume && m_Volume->GetMapper()) {
		m_Volume->GetMapper()->RemoveAllClippingPlanes();
	}
}
//=============================================================================

//=============================================================================
void VolumeWidget::processEvents(unsigned long eventId)
{
	if (!m_Volume) {
		return;
	}

	switch (eventId) {
		case vtkCommand::Move3DEvent: {
			onMoveEvent();
			break;
		}
		case vtkCommand::FifthButtonPressEvent: {
			if (m_InteractionState == InteractionState::INTERSECTING) {
				changeInteractionState(InteractionState::ACTIVE);
			}
			break;
		}
		case vtkCommand::FifthButtonReleaseEvent: {
			changeInteractionState(InteractionState::INACTIVE);
			break;
		}
	}
}
//=============================================================================

//=============================================================================
void VolumeWidget::onMoveEvent()
{
	auto currentDevicePose = m_Interactor->GetDevicePose();
	common::Point3dType rayBase = currentDevicePose.translation();
	common::Point3dType rayTip = rayBase -
		m_Interactor->GetInteractionRayLength() *
			currentDevicePose.linear().col(2);

	switch (m_InteractionState) {
		case InteractionState::INACTIVE: {
		}
		case InteractionState::INTERSECTING: {
			bool volumePicked{false};

			if (m_VolumePicker->Pick3DPoint(rayBase.data(), rayTip.data(),
					m_Interactor->GetRenderWindow()
						->GetRenderers()
						->GetFirstRenderer())) {
				auto pickedProps = m_VolumePicker->GetProp3Ds();
				auto pickedPoints = m_VolumePicker->GetPickedPositions();

				if (pickedProps->GetNumberOfItems() > 0) {
					auto minDistance = std::numeric_limits<double>::max();
					vtkProp3D* currentProp{ nullptr };
					vtkProp3D* closestProp{ nullptr };
					vtkCollectionSimpleIterator sit;
					pickedProps->InitTraversal(sit);

					// Find prop closest to ray base by transforming the picked
					// points into interaction device space and looking at the
					// transformed point nearest the origin (i.e., ray base)
					for (unsigned int i = 0;
						 i < pickedPoints->GetNumberOfPoints(); ++i) {
						Eigen::Map<Eigen::Vector3d> m(
							pickedPoints->GetPoint(i));
						currentProp = pickedProps->GetNextProp3D(sit);

						if (auto z =
								std::abs((currentDevicePose.inverse() * m)[2]);
							z < minDistance) {
							minDistance = z;
							closestProp = currentProp;
						}
					}

					if (closestProp == m_Volume) {
						volumePicked = true;

						// record the current transform in interaction device
						// coordinates.  This will allow us to determine the 
						// (desired) future transform when the interaction 
						// device pose changes
						m_TempTransform = currentDevicePose.inverse() *
							convertVTKToCommonTransform(
								m_Volume->GetUserTransform());

						changeInteractionState(InteractionState::INTERSECTING);
					}
				}
			}

			if (!volumePicked) {
				changeInteractionState(InteractionState::INACTIVE);
			}

			break;
		}
		case InteractionState::ACTIVE: {
			common::TransformType xForm = currentDevicePose * m_TempTransform;
			emit requestPropertyUpdate({ {"transform", xForm} });

			break;
		}
	}
}
//=============================================================================

//=============================================================================
void VolumeWidget::changeInteractionState(InteractionState newState)
{
	if (m_InteractionState == newState) {
		return;
	}

	auto oldState = m_InteractionState;
	m_InteractionState = newState;

	emit interactionStateChanged(newState, QPrivateSignal{});
}
//=============================================================================

//=============================================================================
