#ifndef laserWidget_h
#define laserWidget_h

#include "common/coreTypes.h"
#include "widgetInterface.h"

#include <cereal/access.hpp>

#include <vtkSmartPointer.h>

#include <QColor>

#include <array>
#include <functional>
#include <map>

class vtkGeneralizedCallbackCommand;
class vtkLineSource;
class vtkActor;
class vtkPlane;

class LaserWidget : public WidgetInterface
{
	Q_OBJECT

public:
	using PointType = common::Point3dType;
	using ColorVectorType = common::ColorVectorType;
	using PoseType = common::TransformType;

	explicit LaserWidget(QObject* parent = nullptr);
	virtual ~LaserWidget() override;

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

	void setVisible(bool);
	bool getVisible() const;

	void setBase(double x, double y, double z);
	virtual void setBase(const PointType&);
	virtual PointType getBase() const;

	virtual void setTip(double x, double y, double z);
	void setTip(const PointType&);
	virtual PointType getTip() const;

	double getLength() const;

	void setColor(double r, double g, double b);
	virtual void setColor(const ColorVectorType& color);
	virtual ColorVectorType getColor() const;

	void setHighlight(bool);

public slots:
	virtual void updateProperties(const PropertyListType&) override;

protected:
	virtual void processEvents(unsigned long eventId);
	void updateBaseInternal(const PointType&);
	void updateTipInternal(const PointType&);

	vtkSmartPointer<Interactor> m_Interactor;
	vtkSmartPointer<vtkLineSource> m_LineSource;
	vtkSmartPointer<vtkActor> m_LineActor;
	vtkSmartPointer<vtkGeneralizedCallbackCommand> m_CallbackCommand;
	QColor m_LaserColor;
	PoseType m_LaserPose;
	double m_DefaultLineWidth;
	bool m_ProcessEvents;

private:
	friend class cereal::access;

	template <class Archive>
	void save(Archive& archive) const
	{
		archive(cereal::make_nvp("base", getBase()),
			cereal::make_nvp("tip", getTip()),
			cereal::make_nvp("color", getColor()));
	}

	template <class Archive>
	void load(Archive& archive)
	{
		PointType base;
		PointType tip;
		ColorVectorType color;

		archive(base, tip, color);
		updateBaseInternal(base);
		updateTipInternal(tip);
		setColor(color);
	}
};
#endif