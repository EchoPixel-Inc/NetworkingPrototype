#ifndef splineWidget_h
#define splineWidget_h

#include "widgetInterface.h"

#include <cereal/access.hpp>

#include <vtkSmartPointer.h>

#include <vector>
#include <optional>

class vtkGeneralizedCallbackCommand;
class vtkActor;
class vtkCellPicker;
class vtkParametricFunctionSource;
class vtkParametricSpline;
class vtkPoints;
class vtkPolyData;
class vtkProp;
class vtkSphereSource;
class vtkTransform;
class vtkAssembly;

class SplineWidget : public WidgetInterface
{
	Q_OBJECT;
	
public:
	using NodeIdType = unsigned int;
	using NodePositionType = common::Point3dType;
	using NodeListType = std::vector<NodePositionType>;

	enum class InteractionState {
		DEFINING,
		INACTIVE,
		INTERSECTING,
		ACTIVE
	};

	explicit SplineWidget();
	virtual ~SplineWidget() override;

	SplineWidget(const SplineWidget&) = delete;
	SplineWidget& operator=(const SplineWidget&) = delete;

	SplineWidget(SplineWidget&&) = default;
	SplineWidget& operator=(SplineWidget&&) = default;

	virtual void setInteractor(Interactor*) override;
	virtual Interactor* getInteractor() const override;

	virtual void setTransform(const TransformType&) override;
	virtual TransformType getTransform() const override;

	virtual void attach(const WidgetInterface*) override;
	virtual void detach() override;

	void startInteractivePlacement();
	InteractionState getInteractionState() const;

	virtual void setProcessEvents(bool) override;
	virtual bool getProcessEvents() const override;

	virtual void setPickable(bool) override;
	virtual bool getPickable() const override;

	virtual void addClippingPlane(vtkPlane*) override;
	virtual void removeClippingPlane(vtkPlane*) override;
	virtual void removeAllClippingPlanes() override;

	virtual void addNode(const NodePositionType&);
	void addNode(double x, double y, double z);

	virtual void removeNode(NodeIdType);

	void setNodePosition(NodeIdType, double x, double y, double z);
	virtual void setNodePosition(NodeIdType, const NodePositionType&);
	std::optional<NodePositionType> getNodePosition(NodeIdType) const;

	NodeListType getNodes() const;

public slots:
	virtual void updateProperties(const PropertyListType&) override;

signals:
	void interactionStateChanged(InteractionState newState, 
		InteractionState previousState, QPrivateSignal);

protected:
	std::optional<NodeIdType> getNodeIndex(vtkProp*);
	void processEvents(unsigned long eventId);
	void initializeFromNodes(const NodeListType&);
	void updateNodePositionInternal(NodeIdType, const NodePositionType&);
	void onMoveEvent();
	void onButtonPressEvent();
	void onButtonReleseEvent();
	void onKeyPressEvent();
	void changeInteractionState(InteractionState newState);
	void setTransformInternal(const TransformType&);

	vtkSmartPointer<Interactor> m_Interactor;
	vtkSmartPointer<vtkGeneralizedCallbackCommand> m_CallbackCommand;
	vtkSmartPointer<vtkCellPicker> m_NodePicker;
	vtkSmartPointer<vtkParametricSpline> m_Spline;
	vtkSmartPointer<vtkParametricFunctionSource> m_ParametricFunction;
	vtkSmartPointer<vtkActor> m_LineActor;
	vtkSmartPointer<vtkAssembly> m_PropAssembly;
	std::vector<vtkSmartPointer<vtkActor>> m_NodeActors;
	InteractionState m_InteractionState;
	NodeIdType m_ActiveNode;
	bool m_ProcessEvents;
	common::Point3dType m_ActiveNodeDeviceCoords;
	TransformType m_CachedTransform;
	TransformType m_AssemblyTransform; // convenience so we don't always have
	// to query the actual vtk object and perform conversions
	QMetaObject::Connection m_AttachmentConnection;

private:
	friend class cereal::access;

	template <class Archive>
	void save(Archive& archive) const
	{
		archive(cereal::make_nvp("nodes", getNodes()));
		archive(cereal::make_nvp("transform", getTransform()));
	}

	template <class Archive>
	void load(Archive& archive)
	{
		NodeListType nodes;
		TransformType transform;
		archive(nodes, transform);

		setTransformInternal(transform);
		initializeFromNodes(nodes);
	}
};

#endif