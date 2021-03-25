#ifndef widgetInterface_h
#define widgetInterface_h

#include "common/coreTypes.h"

#include <QObject>

class Interactor;
class vtkPlane;

class WidgetInterface : public QObject
{
	Q_OBJECT;

public:
	using VariantType = common::VariantType;
	using PropertyVariantType = common::PropertyVariantType;
	using PropertyListType = common::PropertyListType;
	using TransformType = common::TransformType;

	virtual void setInteractor(Interactor*) = 0;
	virtual Interactor* getInteractor() const = 0;

	virtual void setTransform(const TransformType&) = 0;
	virtual TransformType getTransform() const = 0;

	virtual void attach(const WidgetInterface*) = 0;
	virtual void detach() = 0;

	virtual void setProcessEvents(bool) = 0;
	virtual bool getProcessEvents() const = 0;

	virtual void setPickable(bool) = 0;
	virtual bool getPickable() const = 0;

	virtual void addClippingPlane(vtkPlane*) = 0;
	virtual void removeClippingPlane(vtkPlane*) = 0;
	virtual void removeAllClippingPlanes() = 0;

public slots:
	virtual void updateProperties(const PropertyListType&) = 0;

signals:
	void requestPropertyUpdate(const PropertyListType&);
	void propertyUpdated(const PropertyListType&);
	void transformChanged(const TransformType&);
};

#endif