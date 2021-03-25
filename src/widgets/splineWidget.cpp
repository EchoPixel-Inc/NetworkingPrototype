#include "widgets/splineWidget.h"
#include "interaction/interactor.h"
#include "vtkUtils/vtkGeneralizedCallbackCommand.h"
#include "vtkUtils/vtkCommonConversions.h"

#include <vtkCellPicker.h>
#include <vtkNew.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkParametricFunctionSource.h>
#include <vtkParametricSpline.h>
#include <vtkPolyData.h>
#include <vtkProp.h>
#include <vtkProp3DCollection.h>
#include <vtkProperty.h>
#include <vtkTransform.h>
#include <vtkPoints.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>
#include <vtkAssembly.h>
#include <vtkAssemblyPath.h>
#include <vtkAssemblyNode.h>
#include <vtkPlane.h>

#include <iostream>
#include <string>
#include <algorithm>
#include <limits>

namespace
{
static const common::ColorVectorType defaultLineColor =
	common::ColorVectorType{0.0, 1.0, 0.0};

static const common::ColorVectorType defaultNodeColor =
	common::ColorVectorType{0.0, 1.0, 0.0};

static const common::ColorVectorType highlightedNodeColor =
	common::ColorVectorType{1.0, 0.0, 0.0};

vtkSmartPointer<vtkActor> createNodeActor()
{
	// Now create the new node actor
	vtkNew<vtkSphereSource> sphereSource;
	sphereSource->SetCenter(0.0, 0.0, 0.0);
	sphereSource->SetRadius(2.5);
	sphereSource->SetPhiResolution(15);
	sphereSource->SetThetaResolution(15);
	sphereSource->Update();

	vtkNew<vtkPolyDataMapper> sphereMapper;
	sphereMapper->SetInputData(sphereSource->GetOutput());

	auto sphereActor = vtkSmartPointer<vtkActor>::New();
	sphereActor->SetMapper(sphereMapper);
	sphereActor->SetPickable(true);
	sphereActor->GetProperty()->SetColor(
		defaultNodeColor[0], defaultNodeColor[1], defaultNodeColor[2]);

	return sphereActor;
}
}  // namespace

//=============================================================================
SplineWidget::SplineWidget() :
	m_CallbackCommand{vtkSmartPointer<vtkGeneralizedCallbackCommand>::New()},
	m_ProcessEvents{false},
	m_InteractionState{InteractionState::INACTIVE},
	m_Spline{vtkSmartPointer<vtkParametricSpline>::New()},
	m_PropAssembly{vtkSmartPointer<vtkAssembly>::New()},
	m_NodePicker{vtkSmartPointer<vtkCellPicker>::New()},
	m_ParametricFunction{vtkSmartPointer<vtkParametricFunctionSource>::New()},
	m_ActiveNode{0},
	m_CachedTransform{TransformType::Identity()},
	m_AssemblyTransform{TransformType::Identity()}
{
	m_CallbackCommand->setCallback(
		[this](vtkObject* caller, unsigned long eventId,
			void* vtkNotUsed(callData)) { this->processEvents(eventId); });

	vtkNew<vtkPoints> splinePoints;
	m_Spline->SetPoints(splinePoints);

	m_ParametricFunction->SetParametricFunction(m_Spline);
	m_ParametricFunction->SetScalarModeToNone();
	m_ParametricFunction->GenerateTextureCoordinatesOff();
	m_ParametricFunction->SetUResolution(499);

	vtkNew<vtkPolyDataMapper> lineMapper;
	lineMapper->SetResolveCoincidentTopologyToPolygonOffset();

	m_LineActor = vtkSmartPointer<vtkActor>::New();
	m_LineActor->SetMapper(lineMapper);
	m_LineActor->GetProperty()->SetColor(
		defaultLineColor[0], defaultLineColor[1], defaultLineColor[2]);
	m_LineActor->SetPickable(false);

	vtkNew<vtkMatrix4x4> propAssemblyUserMatrix;
	propAssemblyUserMatrix->DeepCopy(m_AssemblyTransform.matrix().data());

	m_PropAssembly->SetUserMatrix(propAssemblyUserMatrix);
	m_PropAssembly->AddPart(m_LineActor);

	// m_NodePicker->PickFromListOn();
}
//=============================================================================

//=============================================================================
SplineWidget::~SplineWidget()
{
	setInteractor(nullptr);
}
//=============================================================================

//=============================================================================
void SplineWidget::updateProperties(const PropertyListType& propertyList)
{
	PropertyListType updatedProps;

	for (const auto& elem : propertyList) {
		auto& [propName, propValue] = elem;
		if (propName == "nodePosition") {
			const auto& indexValPair =
				std::get<std::vector<VariantType>>(propValue);
			auto index = std::get<std::uint16_t>(indexValPair[0]);
			const auto& position =
				std::get<common::Point3dType>(indexValPair[1]);
			updateNodePositionInternal(index, position);

			if (auto nodePosition = getNodePosition(index)) {
				updatedProps.push_back({"nodePosition",
					std::vector<VariantType>{
						static_cast<uint16_t>(index), nodePosition.value()}});
			}
		}
		else if (propName == "nodes") {
			const auto& nodeVector =
				std::get<std::vector<VariantType>>(propValue);
			NodeListType newNodeList;
			std::transform(nodeVector.begin(), nodeVector.end(),
				std::back_inserter(newNodeList),
				[](auto&& elem) { return std::get<NodePositionType>(elem); });
			initializeFromNodes(newNodeList);

			const auto& nodes = getNodes();
			std::vector<VariantType> variantNodes;
			std::copy(
				nodes.begin(), nodes.end(), std::back_inserter(variantNodes));
			updatedProps.push_back({propName, variantNodes});
		}
		else if (propName == "transform") {
			const auto& transform = std::get<TransformType>(propValue);
			setTransformInternal(transform);
			updatedProps.push_back({propName, transform});
		}
	}

	emit propertyUpdated(updatedProps);
}
//=============================================================================

//=============================================================================
void SplineWidget::setInteractor(Interactor* iren)
{
	if (m_Interactor == iren) {
		return;
	}

	// If we had a valid interactor, remove the callbacks and view props
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
Interactor* SplineWidget::getInteractor() const
{
	return m_Interactor;
}
//=============================================================================

//=============================================================================
void SplineWidget::setTransform(const TransformType& transform)
{
	emit requestPropertyUpdate({{"transform", transform}});
}
//=============================================================================

//=============================================================================
auto SplineWidget::getTransform() const -> TransformType
{
	return m_AssemblyTransform;
}
//=============================================================================

//=============================================================================
void SplineWidget::setTransformInternal(const TransformType& transform)
{
	m_AssemblyTransform = transform;
	m_PropAssembly->GetUserMatrix()->DeepCopy(
		m_AssemblyTransform.matrix().data());

	emit transformChanged(transform);
}
//=============================================================================

//=============================================================================
void SplineWidget::attach(const WidgetInterface* widget)
{
	const auto& targetWidgetTransform = widget->getTransform();
	m_CachedTransform = targetWidgetTransform.inverse() * getTransform();

	m_AttachmentConnection = QObject::connect(
		widget, &WidgetInterface::transformChanged, this,
		[this](const auto& transform) {
			setTransform(transform * m_CachedTransform);
		},
		Qt::AutoConnection);
}
//=============================================================================

//=============================================================================
void SplineWidget::detach()
{
	QObject::disconnect(m_AttachmentConnection);
}
//=============================================================================

//=============================================================================
void SplineWidget::setProcessEvents(bool process)
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

		m_Interactor->AddObserver(vtkCommand::KeyPressEvent, m_CallbackCommand);
	}
	else {
		m_Interactor->RemoveObserver(m_CallbackCommand);
	}
}
//=============================================================================

//=============================================================================
bool SplineWidget::getProcessEvents() const
{
	return m_ProcessEvents;
}
//=============================================================================

//=============================================================================
void SplineWidget::setPickable(bool pickable)
{
	m_PropAssembly->SetPickable(pickable);
}
//=============================================================================

//=============================================================================
bool SplineWidget::getPickable() const
{
	return m_PropAssembly->GetPickable();
}
//=============================================================================

//=============================================================================
void SplineWidget::addClippingPlane(vtkPlane* plane)
{
	m_LineActor->GetMapper()->AddClippingPlane(plane);
	for (auto& actor : m_NodeActors) {
		actor->GetMapper()->AddClippingPlane(plane);
	}
}
//=============================================================================

//=============================================================================
void SplineWidget::removeClippingPlane(vtkPlane* plane)
{
	m_LineActor->GetMapper()->RemoveClippingPlane(plane);
	for (auto& actor : m_NodeActors) {
		actor->GetMapper()->RemoveClippingPlane(plane);
	}
}
//=============================================================================

//=============================================================================
void SplineWidget::removeAllClippingPlanes()
{
	m_LineActor->GetMapper()->RemoveAllClippingPlanes();
	for (auto& actor : m_NodeActors) {
		actor->GetMapper()->RemoveAllClippingPlanes();
	}
}
//=============================================================================

//=============================================================================
void SplineWidget::startInteractivePlacement()
{
	changeInteractionState(InteractionState::DEFINING);

	// start with at least one node
	NodePositionType newNodePosition;
	if (m_Interactor) {
		const auto& currentDevicePose = m_Interactor->GetDevicePose();
		newNodePosition = currentDevicePose.translation() -
			m_Interactor->GetInteractionRayLength() *
				currentDevicePose.linear().col(2);
	}
	else {
		newNodePosition = NodePositionType{0.0, 0.0, 0.0};
	}

	addNode(newNodePosition);
}
//=============================================================================

//=============================================================================
auto SplineWidget::getInteractionState() const -> InteractionState
{
	return m_InteractionState;
}
//=============================================================================

//=============================================================================
void SplineWidget::setNodePosition(NodeIdType id, double x, double y, double z)
{
	setNodePosition(id, NodePositionType{x, y, z});
}
//=============================================================================

//=============================================================================
void SplineWidget::setNodePosition(
	NodeIdType nodeId, const NodePositionType& nodePosition)
{
	emit requestPropertyUpdate({{"nodePosition",
		std::vector<VariantType>{
			static_cast<uint16_t>(nodeId), nodePosition}}});
}
//=============================================================================

//=============================================================================
void SplineWidget::updateNodePositionInternal(
	NodeIdType nodeId, const NodePositionType& nodePosition)
{
	if ((nodeId < 0) || (nodeId >= m_NodeActors.size())) {
		std::cerr << "Node index: " << nodeId << " is out of range"
				  << std::endl;
		return;
	}

	auto transformedPosition = m_AssemblyTransform.inverse() * nodePosition;

	m_Spline->GetPoints()->SetPoint(nodeId, transformedPosition[0],
		transformedPosition[1], transformedPosition[2]);
	m_Spline->Modified();
	m_NodeActors[nodeId]->SetPosition(
		transformedPosition[0], transformedPosition[1], transformedPosition[2]);
}
//=============================================================================

//=============================================================================
void SplineWidget::addNode(const NodePositionType& node)
{
	std::vector<VariantType> variantNodes;
	auto nodeList = getNodes();

	std::copy(
		nodeList.begin(), nodeList.end(), std::back_inserter(variantNodes));

	variantNodes.push_back(node);

	emit requestPropertyUpdate({{"nodes", variantNodes}});
}
//=============================================================================

//=============================================================================
void SplineWidget::addNode(double x, double y, double z)
{
	addNode(NodePositionType{x, y, z});
}
//=============================================================================

//=============================================================================
void SplineWidget::removeNode(NodeIdType nodeId)
{
	if ((nodeId < 0) || (nodeId >= m_NodeActors.size())) {
		std::cerr << "Node index: " << nodeId << " is out of range"
				  << std::endl;
		return;
	}

	auto nodeList = getNodes();
	nodeList.erase(nodeList.begin() + nodeId);

	std::vector<VariantType> variantNodes;
	std::copy(
		nodeList.begin(), nodeList.end(), std::back_inserter(variantNodes));

	emit requestPropertyUpdate({{"nodes", variantNodes}});
}
//=============================================================================

//=============================================================================
auto SplineWidget::getNodePosition(NodeIdType nodeId) const
	-> std::optional<NodePositionType>
{
	if (nodeId >= 0 && nodeId < m_NodeActors.size()) {
		double p[3];
		m_NodeActors[nodeId]->GetPosition(p);

		Eigen::Map<NodePositionType> m(p);

		return NodePositionType{m_AssemblyTransform * m};
	}
	else {
		return std::nullopt;
	}
}
//=============================================================================

//=============================================================================
void SplineWidget::initializeFromNodes(const NodeListType& nodes)
{
	if (nodes.empty()) {
		std::cerr << "Node list is empty!" << std::endl;
		return;
	}

	NodeListType transformedNodes;
	std::transform(nodes.begin(), nodes.end(),
		std::back_inserter(transformedNodes), [this](const auto& node) {
			return m_AssemblyTransform.inverse() * node;
		});

	vtkNew<vtkPoints> newNodes;
	for (const auto& node : transformedNodes) {
		newNodes->InsertNextPoint(node[0], node[1], node[2]);
	}

	m_Spline->SetPoints(newNodes);
	m_Spline->Modified();

	// If there are zero input connections it means this is the first time
	// we're initializing the nodes, and we need to set the mapper input
	// from the spline function output (doing so before there are any spline
	// nodes triggers a VTK error)
	if (m_LineActor->GetMapper()->GetNumberOfInputConnections(0) == 0) {
		m_LineActor->GetMapper()->SetInputConnection(
			m_ParametricFunction->GetOutputPort());
	}

	if (m_NodeActors.size() < transformedNodes.size()) {
		for (auto it = transformedNodes.begin() + m_NodeActors.size();
			 it != transformedNodes.end(); it++) {
			m_NodeActors.push_back(createNodeActor());
			m_PropAssembly->AddPart(m_NodeActors.back());
		}
	}
	else if (m_NodeActors.size() > transformedNodes.size()) {
		for (auto it = m_NodeActors.begin() + transformedNodes.size();
			it != m_NodeActors.end(); it++) {
			m_PropAssembly->RemovePart(*it);
		}

		m_NodeActors.erase(m_NodeActors.begin() + transformedNodes.size(),
			m_NodeActors.end());
	}

	for (unsigned int i = 0; i < transformedNodes.size(); i++) {
		m_NodeActors[i]->SetPosition(transformedNodes[i][0],
			transformedNodes[i][1], transformedNodes[i][2]);
	}
}
//=============================================================================

//=============================================================================
auto SplineWidget::getNodeIndex(vtkProp* prop) -> std::optional<NodeIdType>
{
	auto nodeIter = std::find(
		m_NodeActors.begin(), m_NodeActors.end(), vtkActor::SafeDownCast(prop));

	if (nodeIter != m_NodeActors.end()) {
		return std::distance(m_NodeActors.begin(), nodeIter);
	}
	else {
		return std::nullopt;
	}
}
//=============================================================================

//=============================================================================
auto SplineWidget::getNodes() const -> NodeListType
{
	auto pts = m_Spline->GetPoints();
	NodeListType nodes;
	for (vtkIdType i = 0; i < pts->GetNumberOfPoints(); i++) {
		double pt[3];
		pts->GetPoint(i, pt);
		Eigen::Map<NodePositionType> m(pt);
		nodes.push_back(NodePositionType{m_AssemblyTransform * m});
	}

	return nodes;
}
//=============================================================================

//=============================================================================
void SplineWidget::processEvents(unsigned long eventId)
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
			onButtonReleseEvent();
			break;
		}
		case vtkCommand::KeyPressEvent: {
			onKeyPressEvent();
			break;
		}
	}  // end switch
}
//=============================================================================

//=============================================================================
void SplineWidget::onMoveEvent()
{
	const auto& currentDevicePose = m_Interactor->GetDevicePose();

	common::TransformType::VectorType rayBase = currentDevicePose.translation();

	common::TransformType::VectorType rayTip = rayBase -
		m_Interactor->GetInteractionRayLength() *
			currentDevicePose.linear().col(2);

	switch (m_InteractionState) {
		case InteractionState::DEFINING: {
			if (m_NodeActors.empty()) {
				return;
			}

			auto activeNode = m_NodeActors.size() - 1;

			emit requestPropertyUpdate({{"nodePosition",
				std::vector<VariantType>{static_cast<uint16_t>(activeNode),
					NodePositionType{rayTip[0], rayTip[1], rayTip[2]}}}});

			break;
		}
		case InteractionState::INACTIVE: {
		}
		case InteractionState::INTERSECTING: {
			bool nodePicked{false};

			if (m_NodePicker->Pick3DPoint(rayBase.data(), rayTip.data(),
					m_Interactor->GetRenderWindow()
						->GetRenderers()
						->GetFirstRenderer())) {
				auto path = m_NodePicker->GetPath();
				if (auto viewProp = path->GetLastNode()->GetViewProp()) {
					if (auto nodeIndex = getNodeIndex(viewProp);
						nodeIndex.has_value()) {
						m_ActiveNode = nodeIndex.value();

						nodePicked = true;

						m_NodeActors[m_ActiveNode]->GetProperty()->SetColor(
							highlightedNodeColor[0], highlightedNodeColor[1],
							highlightedNodeColor[2]);

						// record the current node position in interaction
						// device coordinates.  This will allow us to determine
						// the (desired) future position of the node when the
						// interaction device pose changes
						m_ActiveNodeDeviceCoords = currentDevicePose.inverse() *
							getNodePosition(m_ActiveNode).value();

						changeInteractionState(InteractionState::INTERSECTING);
					}
				}
			}

			if (!nodePicked) {
				if ((m_InteractionState == InteractionState::INTERSECTING) &&
					(m_NodeActors.size() > 0) && (m_ActiveNode >= 0) &&
					m_ActiveNode < m_NodeActors.size()) {
					m_NodeActors[m_ActiveNode]->GetProperty()->SetColor(
						defaultNodeColor[0], defaultNodeColor[1],
						defaultNodeColor[2]);
				}
				changeInteractionState(InteractionState::INACTIVE);
			}

			break;
		}
		case InteractionState::ACTIVE: {
			const auto& lastDevicePose = m_Interactor->GetLastDevicePose();
			common::TransformType newTransform =
				common::TransformType::Identity();

			auto newNodePosition = currentDevicePose * m_ActiveNodeDeviceCoords;

			emit requestPropertyUpdate({{"nodePosition",
				std::vector<VariantType>{
					static_cast<uint16_t>(m_ActiveNode), newNodePosition}}});

			break;
		}
	}  // end switch
}
//=============================================================================

//=============================================================================
void SplineWidget::onButtonPressEvent()
{
	switch (m_InteractionState) {
		case InteractionState::DEFINING: {
			const auto& devicePose = m_Interactor->GetDevicePose();
			common::Point3dType rayTip = devicePose.translation() -
				m_Interactor->GetInteractionRayLength() *
					devicePose.linear().col(2);

			addNode(rayTip);

			break;
		}
		case InteractionState::INTERSECTING: {
			changeInteractionState(InteractionState::ACTIVE);
			break;
		}
	}  // end switch
}
//=============================================================================

//=============================================================================
void SplineWidget::onButtonReleseEvent()
{
	switch (m_InteractionState) {
		case InteractionState::ACTIVE: {
			changeInteractionState(InteractionState::INACTIVE);

			if ((m_NodeActors.size() > 0) && (m_ActiveNode >= 0) &&
				(m_ActiveNode < m_NodeActors.size())) {
				m_NodeActors[m_ActiveNode]->GetProperty()->SetColor(
					defaultNodeColor[0], defaultNodeColor[1],
					defaultNodeColor[2]);
			}
			break;
		}
	}  // end switch
}
//=============================================================================

//=============================================================================
void SplineWidget::onKeyPressEvent()
{
	switch (m_InteractionState) {
		case InteractionState::DEFINING: {
			std::string key{m_Interactor->GetKeySym()};
			if (key == "Escape") {
				std::vector<VariantType> variantNodes;
				auto nodeList = getNodes();

				std::copy(nodeList.begin(), nodeList.end() - 1,
					std::back_inserter(variantNodes));

				emit requestPropertyUpdate({{"nodes", variantNodes}});
				changeInteractionState(InteractionState::INACTIVE);
			}
			break;
		}  // end switch
	}
}
//=============================================================================

//=============================================================================
void SplineWidget::changeInteractionState(InteractionState newState)
{
	if (m_InteractionState == newState) {
		return;
	}

	auto oldState = m_InteractionState;
	m_InteractionState = newState;

	emit interactionStateChanged(newState, oldState, QPrivateSignal{});
}
//=============================================================================