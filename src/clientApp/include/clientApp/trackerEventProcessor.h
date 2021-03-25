#ifndef trackerEventProcessor_h
#define trackerEventProcessor_h

#include <vtkSmartPointer.h>

#include <QObject>

class Interactor;
class QEvent;

class TrackerEventProcessor : public QObject
{
public:
	using Superclass = QObject;

	TrackerEventProcessor(Interactor* iren = nullptr, QObject* parent = nullptr);

	virtual ~TrackerEventProcessor();

	void setInteractor(Interactor*);

protected:
	bool event(QEvent*) override;
	vtkSmartPointer<Interactor> m_Interactor;
};

#endif