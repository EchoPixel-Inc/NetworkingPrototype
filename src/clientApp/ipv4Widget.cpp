#include "clientApp/ipv4Widget.h"

#include <QKeyEvent>
#include <QValidator>
#include <QHBoxLayout>

#include <iostream>

namespace
{
class IPSegmentValidator : public QValidator
{
public:
	IPSegmentValidator(QObject* parent = nullptr) : QValidator{parent} {}

	virtual QValidator::State validate(QString& input, int& pos) const override
	{
		bool isInteger;
		auto intVal = input.toInt(&isInteger);

		// Allow empty strings
		if (input.isEmpty()) {
			return QValidator::State::Intermediate;
		}

		// Don't allow non-numeric characters
		if (!isInteger) {
			return QValidator::State::Invalid;
		}

		// Values outside of the range are strictly forbidden
		if ((intVal < 0) || (intVal > 255)) {
			return QValidator::State::Invalid;
		}
		else {
			return QValidator::State::Acceptable;
		}
	}
};
}  // namespace

//=============================================================================
// IPv4EditSegment
//=============================================================================
IPv4EditSegment::IPv4EditSegment(const QString& contents, QWidget* parent) :
	Superclass{contents, parent},
	m_SelectOnMouseRelease{false}
{}
//=============================================================================

//=============================================================================
IPv4EditSegment::~IPv4EditSegment() = default;
//=============================================================================

//=============================================================================
void IPv4EditSegment::jumpIn()
{
	setFocus();
	m_SelectOnMouseRelease = false;
	selectAll();
}
//=============================================================================

//=============================================================================
void IPv4EditSegment::focusInEvent(QFocusEvent* focusEvent)
{
	Superclass::focusInEvent(focusEvent);
	m_SelectOnMouseRelease = true;
}
//=============================================================================

//=============================================================================
void IPv4EditSegment::focusOutEvent(QFocusEvent* focusEvent)
{
	Superclass::focusOutEvent(focusEvent);
	if (text().isEmpty()) {
		setText(QString::number(0));
	}
	else {
		setText(QString::number(text().toInt()));
	}
}
//=============================================================================

//=============================================================================
void IPv4EditSegment::keyPressEvent(QKeyEvent* keyEvent)
{
	auto key = keyEvent->key();
	auto cursorPos = cursorPosition();

	if (key == Qt::Key_Space) {
		emit jumpToNext(QPrivateSignal());
		keyEvent->accept();
		return;
	}

	if (cursorPos == 0) {
		if ((key == Qt::Key_Left) || (key == Qt::Key_Backspace)) {
			emit jumpToPrevious(QPrivateSignal());
			keyEvent->accept();
			return;
		}
	}

	if (cursorPos == text().count()) {
		if (key == Qt::Key_Right) {
			emit jumpToNext(QPrivateSignal());
			keyEvent->accept();
			return;
		}
	}

	Superclass::keyPressEvent(keyEvent);
	auto newCursorPos = cursorPosition();

	if ((newCursorPos == 3) && (key != Qt::Key_Right)) {
		emit jumpToNext(QPrivateSignal());
	}
}
//=============================================================================

//=============================================================================
void IPv4EditSegment::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
	if (!m_SelectOnMouseRelease) {
		return;
	}

	m_SelectOnMouseRelease = false;
	selectAll();

	Superclass::mouseReleaseEvent(mouseEvent);
}
//=============================================================================

//=============================================================================
// IPv4Widget
//=============================================================================
IPv4Widget::IPv4Widget(QWidget* parent) : QFrame{parent}
{
	setContentsMargins(0, 0, 0, 0);

	m_Segment1 = new IPv4EditSegment;
	m_Segment1->setValidator(new IPSegmentValidator(this));

	m_Segment2 = new IPv4EditSegment;
	m_Segment2->setValidator(new IPSegmentValidator(this));

	m_Segment3 = new IPv4EditSegment;
	m_Segment3->setValidator(new IPSegmentValidator(this));

	m_Segment4 = new IPv4EditSegment;
	m_Segment4->setValidator(new IPSegmentValidator(this));

	auto layout = new QHBoxLayout;
	layout->setSpacing(0);
	layout->setMargin(2);
	setLayout(layout);

	auto setupWidgetHelperFcn = [](QLineEdit* widget) {
		widget->setContentsMargins(0, 0, 0, 0);
		widget->setAlignment(Qt::AlignCenter);
		widget->setStyleSheet("QLineEdit {border: 0px none;}");
		widget->setFrame(false);
		widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	};

	auto makeSplitter = [&setupWidgetHelperFcn]() -> QLineEdit* {
		auto splitter = new QLineEdit(".");
		setupWidgetHelperFcn(splitter);
		splitter->setReadOnly(true);
		splitter->setMaximumWidth(
			splitter->fontMetrics().tightBoundingRect(" . ").width());

		return splitter;
	};

	// Segment1
	setupWidgetHelperFcn(m_Segment1);
	m_Segment1->setMaxLength(3);
	QObject::connect(m_Segment1, &IPv4EditSegment::jumpToNext, m_Segment2,
		&IPv4EditSegment::jumpIn);

	layout->addWidget(m_Segment1);
	layout->addWidget(makeSplitter());

	// Segment2
	setupWidgetHelperFcn(m_Segment2);
	m_Segment2->setMaxLength(3);
	QObject::connect(m_Segment2, &IPv4EditSegment::jumpToNext, m_Segment3,
		&IPv4EditSegment::jumpIn);

	QObject::connect(m_Segment2, &IPv4EditSegment::jumpToPrevious, m_Segment1,
		&IPv4EditSegment::jumpIn);

	layout->addWidget(m_Segment2);
	layout->addWidget(makeSplitter());

	// Segment3
	setupWidgetHelperFcn(m_Segment3);
	m_Segment3->setMaxLength(3);
	QObject::connect(m_Segment3, &IPv4EditSegment::jumpToNext, m_Segment4,
		&IPv4EditSegment::jumpIn);

	QObject::connect(m_Segment3, &IPv4EditSegment::jumpToPrevious, m_Segment2,
		&IPv4EditSegment::jumpIn);

	layout->addWidget(m_Segment3);
	layout->addWidget(makeSplitter());

	// Segment4
	setupWidgetHelperFcn(m_Segment4);
	m_Segment4->setMaxLength(3);
	QObject::connect(m_Segment4, &IPv4EditSegment::jumpToPrevious, m_Segment3,
		&IPv4EditSegment::jumpIn);

	layout->addWidget(m_Segment4);

	setFocusPolicy(Qt::StrongFocus);
	setFocusProxy(m_Segment1);
}
//=============================================================================

//=============================================================================
IPv4Widget::~IPv4Widget() = default;
//=============================================================================

//=============================================================================
void IPv4Widget::focusOutEvent(QFocusEvent* focusEvent)
{
	QFrame::focusOutEvent(focusEvent);

	// Make sure that empty fields are set to appropriate default
	// values if we focus out before completing the IPv4 address
	if (m_Segment1->text().isEmpty()) {
		m_Segment1->setText(QString::number(0));
	}

	if (m_Segment2->text().isEmpty()) {
		m_Segment2->setText(QString::number(0));
	}

	if (m_Segment3->text().isEmpty()) {
		m_Segment3->setText(QString::number(0));
	}

	if (m_Segment4->text().isEmpty()) {
		m_Segment4->setText(QString::number(0));
	}
}
//=============================================================================

//=============================================================================
void IPv4Widget::setIPv4Address(const QHostAddress& hostAddress)
{
	bool conversionSuccess;
	auto hostAddressAsInt = hostAddress.toIPv4Address(&conversionSuccess);
	if (conversionSuccess) {
		m_Segment1->setText(QString::number((hostAddressAsInt >> 24) & 0xFF));
		m_Segment2->setText(QString::number((hostAddressAsInt >> 16) & 0xFF));
		m_Segment3->setText(QString::number((hostAddressAsInt >> 8) & 0xFF));
		m_Segment4->setText(QString::number(hostAddressAsInt & 0xFF));
	}
	else {
		std::cerr << "Attempted to set an invalid IPv4 address: "
				  << hostAddress.toString().toStdString() << " ... aborting"
				  << std::endl;
	}
}
//=============================================================================

//=============================================================================
QHostAddress IPv4Widget::getIPv4Address() const
{
	quint32 hostAddress{0};
	hostAddress +=
		m_Segment1->text().isEmpty() ? 0 : m_Segment1->text().toInt();

	hostAddress <<= 8;
	hostAddress +=
		m_Segment2->text().isEmpty() ? 0 : m_Segment2->text().toInt();

	hostAddress <<= 8;
	hostAddress +=
		m_Segment3->text().isEmpty() ? 0 : m_Segment3->text().toInt();

	hostAddress <<= 8;
	hostAddress +=
		m_Segment4->text().isEmpty() ? 0 : m_Segment4->text().toInt();

	return QHostAddress{hostAddress};
}
//=============================================================================

//=============================================================================
QSize IPv4Widget::sizeHint() const
{
	auto rect = m_Segment1->fontMetrics().boundingRect("000.000.000.000");

	return {rect.width(), rect.height()};
}
//=============================================================================

//=============================================================================