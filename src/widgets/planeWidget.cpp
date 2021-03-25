#include "widgets/planeWidget.h"
#include "interaction/interactor.h"
#include "vtkUtils/vtkGeneralizedCallbackCommand.h"

#include <vtkPlaneSource.h>
#include <vtkConeSource.h>
#include <vtkActor.h>
#include <vtkAssembly.h>
#include <vtkPlane.h>
#include <vtkCellPicker.h>
#include <vtkPolyDataMapper.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkProperty.h>
#include <vtkAssemblyPath.h>
#include <vtkAssemblyNode.h>
#include <vtkMatrix4x4.h>
#include <vtkOutlineCornerFilter.h>

namespace
{
constexpr double defaultPlaneDim = 150.0;  // default side of plane in [mm]
constexpr double defaultConeRadius = 5.0;  // default cone radius in [mm]
const common::ColorVectorType defaultConeColor = {0.6, 0.8, 1.0};
const common::ColorVectorType highlightedConeColor = {0.0, 1.0, 1.0};
}  // namespace

//=============================================================================
PlaneWidget::PlaneWidget() :
	m_Interactor{nullptr},
	m_CallbackCommand{vtkSmartPointer<vtkGeneralizedCallbackCommand>::New()},
	m_PlaneSource{vtkSmartPointer<vtkPlaneSource>::New()},
	m_PlaneActor{vtkSmartPointer<vtkActor>::New()},
	m_Plane{vtkSmartPointer<vtkPlane>::New()},
	m_PlaneCornersActor{vtkSmartPointer<vtkActor>::New()},
	m_ConeSource{vtkSmartPointer<vtkConeSource>::New()},
	m_ConeActor{vtkSmartPointer<vtkActor>::New()},
	m_PropAssembly{vtkSmartPointer<vtkAssembly>::New()},
	m_Picker{vtkSmartPointer<vtkCellPicker>::New()},
	m_InteractionState{InteractionState::INACTIVE},
	m_ProcessEvents{false},
	m_TempTransform{TransformType::Identity()},
	m_AssemblyTransform{TransformType::Identity()}
{
	m_CallbackCommand->setCallback(
		[this](vtkObject* caller, unsigned long eventId,
			void* vtkNotUsed(callData)) { this->processEvents(eventId); });

	m_PlaneSource->SetPoint1(defaultPlaneDim, 0.0, 0.0);
	m_PlaneSource->SetPoint2(0.0, defaultPlaneDim, 0.0);
	m_PlaneSource->SetCenter(0.0, 0.0, 0.0);
	m_PlaneSource->SetResolution(10, 10);
	m_PlaneSource->Update();

	vtkNew<vtkOutlineCornerFilter> cornerFilter;
	cornerFilter->SetInputConnection(m_PlaneSource->GetOutputPort());
	cornerFilter->SetCornerFactor(0.1);

	vtkNew<vtkPolyDataMapper> planeCornersMapper;
	planeCornersMapper->SetInputConnection(cornerFilter->GetOutputPort());

	m_PlaneCornersActor->SetMapper(planeCornersMapper);
	m_PlaneCornersActor->GetProperty()->SetColor(
		defaultConeColor[0], defaultConeColor[1], defaultConeColor[2]);
	m_PlaneCornersActor->GetProperty()->SetLineWidth(4.0);
	m_PlaneCornersActor->GetProperty()->SetRepresentationToWireframe();
	m_PlaneCornersActor->SetPickable(false);

	m_Plane->SetOrigin(0.0, 0.0, 0.0);
	m_Plane->SetNormal(0.0, 0.0, -1.0);

	vtkNew<vtkPolyDataMapper> planeMapper;
	planeMapper->SetInputConnection(m_PlaneSource->GetOutputPort());

	m_PlaneActor->SetMapper(planeMapper);
	m_PlaneActor->GetProperty()->SetColor(1.0, 1.0, 1.0);
	//m_PlaneActor->GetProperty()->SetOpacity(0.05); // in case we can turn 
	// on depth peeling
	m_PlaneActor->SetVisibility(false);
	m_PlaneActor->SetPickable(false);

	m_ConeSource->SetCenter(0.0, 0.0, 0.0);
	m_ConeSource->SetRadius(defaultConeRadius);
	m_ConeSource->SetHeight(2.0 * defaultConeRadius);
	m_ConeSource->SetDirection(0.0, 0.0, -1.0);
	m_ConeSource->SetCapping(false);
	m_ConeSource->Update();

	vtkNew<vtkPolyDataMapper> coneMapper;
	coneMapper->SetInputConnection(m_ConeSource->GetOutputPort());

	m_ConeActor->SetMapper(coneMapper);
	m_ConeActor->GetProperty()->SetColor(
		defaultConeColor[0], defaultConeColor[1], defaultConeColor[2]);

	// m_ConeActor->GetProperty()->SetRepresentationToWireframe();
	m_ConeActor->SetPickable(true);

	vtkNew<vtkMatrix4x4> assemblyMatrix;
	assemblyMatrix->Identity();

	m_PropAssembly->SetUserMatrix(assemblyMatrix);
	m_PropAssembly->AddPart(m_PlaneActor);
	m_PropAssembly->AddPart(m_ConeActor);
	m_PropAssembly->AddPart(m_PlaneCornersActor);

	m_Picker->SetPickClippingPlanes(false);
}
//=============================================================================

//=============================================================================
PlaneWidget::~PlaneWidget()
{
	setInteractor(nullptr);
}
//=============================================================================

//=============================================================================
void PlaneWidget::updateProperties(const PropertyListType& propList)
{
	PropertyListType updatedProps;

	for (const auto& elem : propList) {
		auto& [propName, propValue] = elem;
		if (propName == "transform") {
			const auto& transform = std::get<TransformType>(propValue);

			setTransformInternal(transform);
			updatedProps.push_back({propName, propValue});
		}
	}

	emit propertyUpdated(updatedProps);
}
//=============================================================================

//=============================================================================
void PlaneWidget::setInteractor(Interactor* iren)
{
	if (m_Interactor == iren) {
		return;
	}

	if (m_Interactor) {
		m_Interactor->RemoveObserver(m_CallbackCommand);
		if (auto renderer = m_Interactor->GetRenderWindow()
			->GetRenderers()
			->GetFirstRenderer()) {
			renderer->RemoveViewProp(m_PropAssembly);
		}
	}

	m_Interactor = iren;
	m_ProcessEvents = false;

	if (m_Interactor) {
		if (auto renderer = m_Interactor->GetRenderWindow()
								->GetRenderers()
								->GetFirstRenderer()) {
			renderer->AddViewProp(m_PropAssembly);
		}
	}
}
//=============================================================================

//=============================================================================
auto PlaneWidget::getInteractor() const -> Interactor*
{
	return m_Interactor;
}
//=============================================================================

//=============================================================================
void PlaneWidget::setTransform(const TransformType& transform)
{
	emit requestPropertyUpdate({{"transform", transform}});
}
//=============================================================================

//=============================================================================
auto PlaneWidget::getTransform() const -> TransformType
{
	return m_AssemblyTransform;
}
//=============================================================================

//=============================================================================
void PlaneWidget::attach(const WidgetInterface* widget)
{
	// No-op for now
}
//=============================================================================

//=============================================================================
void PlaneWidget::detach()
{
	// No-op for now
}
//=============================================================================

//=============================================================================
void PlaneWidget::setProcessEvents(bool process)
{
	if (!m_Interactor) {
		m_ProcessEvents = false;
		return;
	}

	if (process == m_ProcessEvents) {
		return;
	}

	m_ProcessEvents = process;

	if (process) {
		m_Interactor->AddObserver(vtkCommand::Move3DEvent, m_CallbackCommand);

		m_Interactor->AddObserver(
			vtkCommand::FifthButtonPressEvent, m_CallbackCommand);

		m_Interactor->AddObserver(
			vtkCommand::FifthButtonReleaseEvent, m_CallbackCommand);
	}
	else {
		m_Interactor->RemoveObserver(m_CallbackCommand);
	}
}
//=============================================================================

//=============================================================================
bool PlaneWidget::getProcessEvents() const
{
	return m_ProcessEvents;
}
//=============================================================================

//=============================================================================
void PlaneWidget::setPickable(bool) {}
//=============================================================================

//=============================================================================
bool PlaneWidget::getPickable() const
{
	return false;
}
//=============================================================================

//=============================================================================
void PlaneWidget::addClippingPlane(vtkPlane* plane) {}	// no-op
//=============================================================================

//=============================================================================
void PlaneWidget::removeClippingPlane(vtkPlane* plane) {}  // no-op
//=============================================================================

//=============================================================================
void PlaneWidget::removeAllClippingPlanes() {}	// no-op
//=============================================================================

//=============================================================================
auto PlaneWidget::getPlane() const -> vtkPlane* const
{
	return m_Plane;
}
//=============================================================================

//=============================================================================
void PlaneWidget::processEvents(unsigned long eventId)
{
	switch (eventId) {
		case vtkCommand::Move3DEvent: {
			onMoveEvent();
			break;
		}
		case vtkCommand::FifthButtonPressEvent: {
			onButtonPressEvent();
			break;
		}
		case vtkCommand::FifthButtonReleaseEvent: {
			onButtonReleaseEvent();
			break;
		}
	}
}
//=============================================================================

//=============================================================================
void PlaneWidget::onMoveEvent()
{
	const auto& currentDevicePose = m_Interactor->GetDevicePose();
	common::TransformType::VectorType rayBase = currentDevicePose.translation();
	common::TransformType::VectorType rayTip = rayBase -
		m_Interactor->GetInteractionRayLength() *
			currentDevicePose.linear().col(2);

	switch (m_InteractionState) {
		case InteractionState::INACTIVE: {
		}
		case InteractionState::INTERSECTING: {
			bool picked{false};

			if (m_Picker->Pick3DPoint(rayBase.data(), rayTip.data(),
					m_Interactor->GetRenderWindow()
						->GetRenderers()
						->GetFirstRenderer())) {
				auto path = m_Picker->GetPath();
				if (vtkActor::SafeDownCast(
						path->GetLastNode()->GetViewProp()) == m_ConeActor) {
					picked = true;
					m_ConeActor->GetProperty()->SetColor(
						highlightedConeColor[0], highlightedConeColor[1],
						highlightedConeColor[2]);

					m_PlaneCornersActor->GetProperty()->SetColor(
						highlightedConeColor[0], highlightedConeColor[1],
						highlightedConeColor[2]);

					m_TempTransform =
						currentDevicePose.inverse() * m_AssemblyTransform;

					changeInteractionState(InteractionState::INTERSECTING);
				}
			}

			if (!picked) {
				if (m_InteractionState == InteractionState::INTERSECTING) {
					m_ConeActor->GetProperty()->SetColor(defaultConeColor[0],
						defaultConeColor[1], defaultConeColor[2]);

					m_PlaneCornersActor->GetProperty()->SetColor(
						defaultConeColor[0], defaultConeColor[1],
						defaultConeColor[2]);
				}
				changeInteractionState(InteractionState::INACTIVE);
			}
			break;
		}
		case InteractionState::ACTIVE: {
			common::TransformType xForm = currentDevicePose * m_TempTransform;
			emit requestPropertyUpdate({{"transform", xForm}});

			break;
		}
	}  // end switch
}
//=============================================================================

//=============================================================================
void PlaneWidget::onButtonPressEvent()
{
	if (m_InteractionState == InteractionState::INTERSECTING) {
		changeInteractionState(InteractionState::ACTIVE);
	}
}
//=============================================================================

//=============================================================================
void PlaneWidget::onButtonReleaseEvent()
{
	changeInteractionState(InteractionState::INACTIVE);
}
//=============================================================================

//=============================================================================
void PlaneWidget::changeInteractionState(InteractionState newState)
{
	if (m_InteractionState == newState) {
		return;
	}

	auto oldState = m_InteractionState;
	m_InteractionState = newState;

	emit interactionStateChanged(newState, oldState, QPrivateSignal{});
}
//=============================================================================

//=============================================================================
void PlaneWidget::setTransformInternal(const TransformType& transform)
{
	m_AssemblyTransform = transform;
	m_PropAssembly->GetUserMatrix()->DeepCopy(transform.matrix().data());

	const TransformType::VectorType planeOrigin = transform.translation();

	const TransformType::VectorType planeNormal =
		-1.0 * transform.linear().col(2);

	m_Plane->SetOrigin(planeOrigin[0], planeOrigin[1], planeOrigin[2]);
	m_Plane->SetNormal(planeNormal[0], planeNormal[1], planeNormal[2]);

	emit transformChanged(transform);
}
//=============================================================================