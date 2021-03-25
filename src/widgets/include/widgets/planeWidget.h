#ifndef planeWidget_h
#define planeWidget_h

#include "widgetInterface.h"

#include <cereal/access.hpp>

#include <vtkSmartPointer.h>

class vtkActor;
class vtkCellPicker;
class vtkAssembly;
class vtkPlaneSource;
class vtkPlane;
class vtkConeSource;
class vtkGeneralizedCallbackCommand;

class PlaneWidget : public WidgetInterface
{
	Q_OBJECT;

public:
	enum class InteractionState {
		INACTIVE,
		INTERSECTING,
		ACTIVE
	};

	explicit PlaneWidget();
	virtual ~PlaneWidget();

	PlaneWidget(const PlaneWidget&) = delete;
	PlaneWidget& operator=(const PlaneWidget&) = delete;

	PlaneWidget(PlaneWidget&&) = default;
	PlaneWidget& operator=(PlaneWidget&&) = default;

	virtual void setInteractor(Interactor*) override;
	virtual Interactor* getInteractor() const override;

	virtual void setTransform(const TransformType&) override;
	virtual TransformType getTransform() const override;

	virtual void attach(const WidgetInterface*) override;
	virtual void detach() override;

	virtual void setProcessEvents(bool) override;
	virtual bool getProcessEvents() const override;

	virtual void setPickable(bool) override;
	virtual bool getPickable() const override;

	virtual void addClippingPlane(vtkPlane*) override;
	virtual void removeClippingPlane(vtkPlane*) override;
	virtual void removeAllClippingPlanes() override;

	vtkPlane* const getPlane() const;

public slots:
	virtual void updateProperties(const PropertyListType&) override;

signals:
	void interactionStateChanged(InteractionState newState,
		InteractionState previousState, QPrivateSignal);

protected:
	void processEvents(unsigned long eventId);
	void onMoveEvent();
	void onButtonPressEvent();
	void onButtonReleaseEvent();
	void changeInteractionState(InteractionState newState);
	void setTransformInternal(const TransformType&);

	InteractionState m_InteractionState;
	vtkSmartPointer<Interactor> m_Interactor;
	vtkSmartPointer<vtkGeneralizedCallbackCommand> m_CallbackCommand;
	vtkSmartPointer<vtkActor> m_PlaneActor;
	vtkSmartPointer<vtkPlane> m_Plane;
	vtkSmartPointer<vtkActor> m_ConeActor;
	vtkSmartPointer<vtkActor> m_PlaneCornersActor;
	vtkSmartPointer<vtkPlaneSource> m_PlaneSource;
	vtkSmartPointer<vtkConeSource> m_ConeSource;
	vtkSmartPointer<vtkAssembly> m_PropAssembly;
	vtkSmartPointer<vtkCellPicker> m_Picker;
	TransformType m_TempTransform;
	TransformType m_AssemblyTransform;
	bool m_ProcessEvents;

private:
	friend class cereal::access;

	template <class Archive>
	void save(Archive& archive) const
	{
		archive(cereal::make_nvp("transform", m_AssemblyTransform));
	}
	template <class Archive>
	void load(Archive& archive)
	{
		auto transform{ TransformType::Identity() };
		archive(transform);

		setTransformInternal(transform);
	}
};

#endif
