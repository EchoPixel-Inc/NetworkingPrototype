#ifndef volumeWidget_h
#define volumeWidget_h

#include "widgetInterface.h"

#include <cereal/access.hpp>

#include <vtkSmartPointer.h>
#include <optional>

class vtkGeneralizedCallbackCommand;
class vtkVolume;
class vtkVolumePicker;
class vtkPicker;
class vtkPropPicker;

class VolumeWidget : public WidgetInterface
{
	Q_OBJECT;

public:
	enum class InteractionState {
		INACTIVE,
		INTERSECTING,
		ACTIVE
	};
	Q_ENUM(InteractionState);

	VolumeWidget(); // default constructor
	explicit VolumeWidget(vtkVolume* volume);
	virtual ~VolumeWidget() override;

	void setVolume(vtkVolume*);
	vtkVolume* const getVolume() const;

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

public slots:
	virtual void updateProperties(const PropertyListType&) override;

signals:
	void interactionStateChanged(const InteractionState&, QPrivateSignal);

protected:
	void processEvents(unsigned long eventId);
	void onMoveEvent();
	void changeInteractionState(InteractionState newState);
	void setTransformInternal(const TransformType&);

	vtkSmartPointer<Interactor> m_Interactor;
	vtkSmartPointer<vtkVolume> m_Volume;
	vtkSmartPointer<vtkGeneralizedCallbackCommand> m_CallbackCommand;
	vtkSmartPointer<vtkVolumePicker> m_VolumePicker;
	InteractionState m_InteractionState;
	common::TransformType m_TempTransform;
	bool m_ProcessEvents;

private:
	friend class cereal::access;

	template <class Archive>
	void save(Archive& archive) const
	{
		archive(cereal::make_nvp("transform", getTransform()));
	}

	template <class Archive>
	void load(Archive& archive)
	{
		TransformType transform{ TransformType::Identity() };
		archive(transform);
		setTransformInternal(transform);
	}
};

#endif