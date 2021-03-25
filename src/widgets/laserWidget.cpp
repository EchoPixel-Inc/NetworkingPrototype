#include "widgets/laserWidget.h"
#include "interaction/interactor.h"
#include "vtkUtils/vtkGeneralizedCallbackCommand.h"

#include <vtkLineSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkPlane.h>

#include <assert.h>
#include <iostream>

//=============================================================================
LaserWidget::LaserWidget(QObject* parent) :
	m_Interactor{nullptr},
	m_LineSource{vtkSmartPointer<vtkLineSource>::New()},
	m_LineActor{vtkSmartPointer<vtkActor>::New()},
	m_CallbackCommand{vtkSmartPointer<vtkGeneralizedCallbackCommand>::New()},
	m_ProcessEvents{false},
	m_DefaultLineWidth{1.0},
	m_LaserPose{PoseType::Identity()}
{
	m_CallbackCommand->setCallback(
		[this](vtkObject* caller, unsigned long eventId,
			void* vtkNotUsed(callData)) { this->processEvents(eventId); });

	auto defaultLaserLength{150.0};
	PoseType::VectorType laserBase = m_LaserPose.translation();
	PoseType::VectorType laserTip = m_LaserPose.translation() -
		defaultLaserLength * m_LaserPose.linear().col(2);

	m_LineSource->SetPoint1(laserBase[0], laserBase[1], laserBase[2]);
	m_LineSource->SetPoint2(laserTip[0], laserTip[1], laserTip[2]);

	vtkNew<vtkPolyDataMapper> mapper;
	mapper->SetInputConnection(m_LineSource->GetOutputPort());

	m_LineActor->SetMapper(mapper);
	m_LineActor->GetProperty()->SetColor(0.0, 1.0, 0.0);
	m_LineActor->GetProperty()->SetLineWidth(1.5);
	m_LineActor->SetPickable(false);
	m_LineActor->SetDragable(false);

	auto defaultColor = m_LineActor->GetProperty()->GetColor();
	m_LaserColor =
		QColor::fromRgbF(defaultColor[0], defaultColor[1], defaultColor[2]);

	qRegisterMetaType<PropertyListType>();
}
//=============================================================================

//=============================================================================
LaserWidget::~LaserWidget()
{
	setInteractor(nullptr);
	std::cout << "LaserWidget::destructor" << std::endl;
}
//=============================================================================

//=============================================================================
void LaserWidget::updateProperties(const PropertyListType& propList)
{
	PropertyListType updatedProps;

	for (const auto& elem : propList) {
		auto& [propName, propValue] = elem;
		
		if (propName == "color") {
			const auto& colorVec = std::get<ColorVectorType>(propValue);
			setColor(colorVec);
			updatedProps.push_back({ propName, getColor() });
		}
		else if (propName == "base") {
			const auto& base = std::get<PointType>(propValue);
			updateBaseInternal(base);
			updatedProps.push_back({ propName, getBase() });
		}
		else if (propName == "tip") {
			const auto& tip = std::get<PointType>(propValue);
			updateTipInternal(tip);
			updatedProps.push_back({ propName, getTip() });
		}
	}

	emit propertyUpdated(updatedProps);
}
//=============================================================================

//=============================================================================
void LaserWidget::setInteractor(Interactor* iren)
{
	if (iren == m_Interactor) {
		return;
	}

	// If we were previously attached to an interactor, remove the
	// callbacks and view props
	if (m_Interactor) {
		m_Interactor->RemoveObserver(m_CallbackCommand);
		if (auto renderer = m_Interactor->GetRenderWindow()
			->GetRenderers()
			->GetFirstRenderer()) {
			renderer->RemoveViewProp(m_LineActor);
		}
	}

	m_Interactor = iren;
	m_ProcessEvents = false;

	if (m_Interactor) {
		if (auto renderer = m_Interactor->GetRenderWindow()
			->GetRenderers()
			->GetFirstRenderer()) {
			renderer->AddViewProp(m_LineActor);
		}		
	}
}
//=============================================================================

//=============================================================================
void LaserWidget::setProcessEvents(bool process)
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
	}
	else {
		m_Interactor->RemoveObserver(m_CallbackCommand);
	}
}
//=============================================================================

//=============================================================================
bool LaserWidget::getProcessEvents() const
{
	return m_ProcessEvents;
}
//=============================================================================

//=============================================================================
void LaserWidget::setVisible(bool visible)
{
	m_LineActor->SetVisibility(visible);
}
//=============================================================================

//=============================================================================
bool LaserWidget::getVisible() const
{
	return m_LineActor->GetVisibility();
}
//=============================================================================

//=============================================================================
Interactor* LaserWidget::getInteractor() const
{
	return m_Interactor;
}
//=============================================================================

//=============================================================================
void LaserWidget::setTransform(const TransformType& transform)
{}
//=============================================================================

//=============================================================================
auto LaserWidget::getTransform() const -> TransformType
{
	return TransformType::Identity();
}
//=============================================================================

//=============================================================================
void LaserWidget::attach(const WidgetInterface*)
{  // not supported
}
//=============================================================================

//=============================================================================
void LaserWidget::detach()
{  // not supported
}
//=============================================================================

//=============================================================================
void LaserWidget::setPickable(bool pickable) {}
//=============================================================================

//=============================================================================
bool LaserWidget::getPickable() const
{
	return false;
}
//=============================================================================

//=============================================================================
void LaserWidget::addClippingPlane(vtkPlane*)
{  // not supported
}
//=============================================================================

//=============================================================================
void LaserWidget::removeClippingPlane(vtkPlane*)
{  // not supported
}
//=============================================================================

//=============================================================================
void LaserWidget::removeAllClippingPlanes()
{  // not supported
}
//=============================================================================

//=============================================================================
void LaserWidget::setBase(double x, double y, double z)
{
	setBase({ x, y, z });
}
//=============================================================================

//=============================================================================
void LaserWidget::setBase(const PointType& point)
{
	emit requestPropertyUpdate({ {"base", point} });
}
//=============================================================================

//=============================================================================
auto LaserWidget::getBase() const -> PointType
{
	auto base = m_LineSource->GetPoint1();
	return {base[0], base[1], base[2]};
}
//=============================================================================

//=============================================================================
void LaserWidget::setTip(double x, double y, double z)
{
	setTip({ x, y, z });
}
//=============================================================================

//=============================================================================
void LaserWidget::setTip(const PointType& point)
{
	emit requestPropertyUpdate({ {"tip", point} });
}
//=============================================================================

//=============================================================================
auto LaserWidget::getTip() const -> PointType
{
	auto tip = m_LineSource->GetPoint2();
	return {tip[0], tip[1], tip[2]};
}
//=============================================================================

//=============================================================================
void LaserWidget::updateBaseInternal(const PointType& base)
{
	m_LineSource->SetPoint1(base[0], base[1], base[2]);
	m_LineSource->Modified();
}
//=============================================================================

//=============================================================================
void LaserWidget::updateTipInternal(const PointType& tip)
{
	m_LineSource->SetPoint2(tip[0], tip[1], tip[2]);
	m_LineSource->Modified();
}
//=============================================================================

//=============================================================================
double LaserWidget::getLength() const
{
	auto base = m_LineSource->GetPoint1();
	auto tip = m_LineSource->GetPoint2();

	double diff[3] = {tip[0] - base[0], tip[1] - base[1], tip[2] - base[2]};

	return vtkMath::Norm(diff);
}
//=============================================================================

//=============================================================================
void LaserWidget::setColor(double r, double g, double b)
{
	setColor({ r, g, b });	
}
//=============================================================================

//=============================================================================
void LaserWidget::setColor(const ColorVectorType& color)
{
	m_LineActor->GetProperty()->SetColor(color[0], color[1], color[2]);
	m_LaserColor = QColor::fromRgbF(color[0], color[1], color[2]);
}
//=============================================================================

//=============================================================================
auto LaserWidget::getColor() const -> ColorVectorType
{
	return ColorVectorType{
		m_LaserColor.redF(), m_LaserColor.greenF(), m_LaserColor.blueF()};
}
//=============================================================================

//=============================================================================
void LaserWidget::setHighlight(bool highlight)
{
	if (highlight) {
		// Increase value and saturation by 20%
		double h = m_LaserColor.hueF();
		double s = 0.5 * m_LaserColor.hsvSaturationF();
		double v = std::min(1.0, 1.2 * m_LaserColor.valueF());

		QColor newColor{QColor::fromHsvF(h, s, v)};

		m_LineActor->GetProperty()->SetColor(
			newColor.redF(), newColor.greenF(), newColor.blueF());

		m_LineActor->GetProperty()->SetLineWidth(3.0);
	}
	else {
		m_LineActor->GetProperty()->SetColor(
			m_LaserColor.redF(), m_LaserColor.greenF(), m_LaserColor.blueF());

		m_LineActor->GetProperty()->SetLineWidth(m_DefaultLineWidth);
	}
}
//=============================================================================

//=============================================================================
void LaserWidget::processEvents(unsigned long eventId)
{
	if (!m_Interactor) {
		return;
	}

	switch (eventId) {
	case vtkCommand::Move3DEvent: {
		const auto& currentDevicePose = m_Interactor->GetDevicePose();
		const PointType rayTip = currentDevicePose.translation() -
			m_Interactor->GetInteractionRayLength() *
			currentDevicePose.linear().col(2);

		emit requestPropertyUpdate({ {"base", currentDevicePose.translation()},
			{"tip", rayTip } });

			break;
		}
	}
}
//=============================================================================

//=============================================================================