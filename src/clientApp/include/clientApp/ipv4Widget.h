#ifndef ipv4Widget_h
#define ipv4Widget_h

#include <QLineEdit>
#include <QFrame>
#include <QHostAddress>

class IPv4EditSegment : public QLineEdit
{
	Q_OBJECT;

public:
	explicit IPv4EditSegment(
		const QString& contents = QString(), QWidget* parent = nullptr);

	virtual ~IPv4EditSegment();

signals:
	void jumpToNext(QPrivateSignal);
	void jumpToPrevious(QPrivateSignal);

public slots:
	void jumpIn();

protected:
	using Superclass = QLineEdit;

	virtual void focusInEvent(QFocusEvent*) override;
	virtual void focusOutEvent(QFocusEvent*) override;
	virtual void keyPressEvent(QKeyEvent*) override;
	virtual void mouseReleaseEvent(QMouseEvent*) override;

private:
	bool m_SelectOnMouseRelease;
};

/// \class epxIPv4EditWidget
/// \brief A GUI object for displaying and editing IPv4 addresses

class IPv4Widget : public QFrame
{
	Q_OBJECT;

public:
	explicit IPv4Widget(QWidget* parent = nullptr);
	virtual ~IPv4Widget();

	/// \brief Set / get the IPv4 address
	void setIPv4Address(const QHostAddress&);
	QHostAddress getIPv4Address() const;

	virtual QSize sizeHint() const override;

protected:
	virtual void focusOutEvent(QFocusEvent*) override;

private:
	IPv4EditSegment* m_Segment1;
	IPv4EditSegment* m_Segment2;
	IPv4EditSegment* m_Segment3;
	IPv4EditSegment* m_Segment4;
};

#endif
