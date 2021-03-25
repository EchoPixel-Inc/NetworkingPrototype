#ifndef applicationActionsWidget_h
#define applicationActionsWidget_h

#include "clientApp/uiActions.h"

#include <QWidget>

#include <memory>

namespace Ui
{
	class ApplicationActionsWidget;
}

class ApplicationActionsWidget : public QWidget
{
	Q_OBJECT;
public:
	explicit ApplicationActionsWidget(QWidget* parent = nullptr,
		Qt::WindowFlags flags = Qt::WindowFlags());

	virtual ~ApplicationActionsWidget();

protected:
	virtual void closeEvent(QCloseEvent*) override;

private:
	std::unique_ptr<Ui::ApplicationActionsWidget> m_Gui;
	UIActions m_UIActions;
};

#endif