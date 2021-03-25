#include "barco/barcoSystem.h"
#include "barco/barcoV2Protocol.h"
#include "barco/barcoUartFrame.h"
#include "barco/barcoSystemImpl.h"
#include "config/config.h"

#include <QSerialPortInfo>
#include <QSerialPort>
#include <QThread>

#include <sstream>
#include <atomic>
#include <mutex>
#include <iostream>

//==============================================================================
// IMPLEMENTATION CLASS
//==============================================================================
BarcoSystem::Impl::Impl(const std::string& comPort) :
	m_COMPort{comPort},
	m_SerialPort{nullptr},
	m_EnableEventReporting{false},
	m_Initialized{false},
	m_TrackerType{0},
	m_EyePositions{{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}
{
	// Set a default head / eye position on the camera so that
	// objects are initially in view
	double defaultHeadPosition[4] = {70.0, 80.0, 400.0, 1.0};

	common::TransformType defaultHeadPose{common::TransformType::Identity()};

	defaultHeadPose(0, 3) = defaultHeadPosition[0];
	defaultHeadPose(1, 3) = defaultHeadPosition[1];
	defaultHeadPose(2, 3) = defaultHeadPosition[2];

	// Set default left and right eye positions as well,
	// assuming an interpupillary distance of 30[mm]
	auto leftEyeDefaultPosition =
		defaultHeadPose * common::TransformType::VectorType{-30.0, 0.0, 0.0};

	auto rightEyeDefaultPosition =
		defaultHeadPose * common::TransformType::VectorType{30.0, 0.0, 0.0};

	m_EyePositions.left = leftEyeDefaultPosition;
	m_EyePositions.right = rightEyeDefaultPosition;

	m_UartFrame.SetReceiveMessageCallback(
		[this](const BarcoUartFrame::MessageType& msg) {
			m_MessageEncoder.ReceiveProtocolMessage(msg);
		});

	m_MessageEncoder.SetSendMessageCallback(
		[this](const BarcoV2Protocol::MessageType& msg) {
			m_UartFrame.SendUartMessage(msg);
		});

	qRegisterMetaType<EyePositionsType>("epxTracking::EyePositions");
	qRegisterMetaType<EyePositionsType>("EyePositionsType");
}
//==============================================================================

//==============================================================================
void BarcoSystem::Impl::run()
{
	// Need to create the serial port on the heap here so that its thread
	// affinity is the current thread spawned by the run() method
	m_SerialPort = std::make_unique<QSerialPort>();
	m_SerialPort->setPortName(QString::fromStdString(m_COMPort));
	m_SerialPort->setBaudRate(QSerialPort::Baud115200);
	m_SerialPort->setDataBits(QSerialPort::Data8);
	m_SerialPort->setParity(QSerialPort::NoParity);
	m_SerialPort->setStopBits(QSerialPort::OneStop);
	m_SerialPort->setFlowControl(QSerialPort::NoFlowControl);

	// Want the error handling to happen in this thread
	QObject::connect(m_SerialPort.get(), &QSerialPort::errorOccurred, this,
		&BarcoSystem::Impl::HandleError, Qt::DirectConnection);

	// Want to process the incomming serialport information in this thread
	QObject::connect(m_SerialPort.get(), &QSerialPort::readyRead, this,
		&BarcoSystem::Impl::ReadData, Qt::DirectConnection);

	// The select tracker requested signal will be emitted from the thread
	// in which Impl lives (i.e., main thread), so we use the serialport as a
	// context object and use an auto (queued) connection so that the slot
	// processing occurs in this thread's event loop
	QObject::connect(
		this, &BarcoSystem::Impl::SelectTrackerRequested, m_SerialPort.get(),
		[this](int tracker) { this->OnSelectTrackerRequested(tracker); },
		Qt::AutoConnection);

	// The write eye positions requested signal will be emitted from the thread
	// in which Impl lives (i.e., main thread), so we use the serialport as a
	// context object and use an auto (queued) connection so that the slot
	// processing occurs in this thread's event loop
	QObject::connect(
		this, &BarcoSystem::Impl::WriteEyePositionsRequested,
		m_SerialPort.get(),
		[this](const EyePositionsType& eyePositions) {
			this->OnWriteEyePositionsRequested(eyePositions);
		},
		Qt::AutoConnection);

	m_UartFrame.SetSendByteCallback([this](unsigned char byte) {
		const char c = static_cast<const char>(byte);
		m_SerialPort->write(&c, 1);
	});

	m_MessageEncoder.SetHandleSendEyeCoordinatesCallback(
		[this](const BarcoV2Protocol::EyeCoordinatesContainerType& coords) {
			this->HandleGetEyeCoordinates(coords);
		});

	BarcoV2Protocol::RequestGetProtocolVersionCallbackType
		getProtVersionCallback([](unsigned char ret, unsigned char val) {
			if (ret == 0) {
				std::cout << "BarcoSystem: Protocol version: "
						  << static_cast<int>(val) << std::endl;
			}
		});

	m_MessageEncoder.RequestGetProtocolVersion(getProtVersionCallback);

	// Once all the relevant signals / slots have been connected, we can
	// proclaim that the communication mechanisms are ready and we can safely
	// emit signals (will be queued)
	m_Initialized.store(true);

	// Now actually try to open the serial port
	if (!m_SerialPort->open(QIODevice::ReadWrite)) {
		this->HandleError(m_SerialPort->error());
		m_SerialPort->close();
		m_SerialPort.reset();

		return;
	}

	// Start an event loop in this thread
	QThread::exec();

	m_SerialPort->close();
	m_SerialPort.reset();
}
//==============================================================================

//==============================================================================
void BarcoSystem::Impl::ReadData()
{
	QByteArray byteArray(m_SerialPort->readAll());
	for (const auto& byte : byteArray) {
		m_UartFrame.ReceiveByte(byte);
	}
}
//==============================================================================

//==============================================================================
void BarcoSystem::Impl::HandleGetEyeCoordinates(
	const BarcoV2Protocol::EyeCoordinatesContainerType& coords)
{
	common::TransformType::VectorType leftEyePosition{
		coords[0], coords[1], coords[2]};
	common::TransformType::VectorType rightEyePosition{
		coords[3], coords[4], coords[5]};

	std::unique_lock<std::mutex> lock(m_EyePositionsMutex);
	m_EyePositions.left = leftEyePosition;
	m_EyePositions.right = rightEyePosition;

	lock.unlock();
}
//==============================================================================

//==============================================================================
void BarcoSystem::Impl::SelectTracker(int tracker)
{
	emit SelectTrackerRequested(tracker, QPrivateSignal{});
}
//==============================================================================

//==============================================================================
void BarcoSystem::Impl::WriteEyePositions(const EyePositionsType& eyePositions)
{
	emit WriteEyePositionsRequested(eyePositions, QPrivateSignal{});
}
//==============================================================================

//==============================================================================
void BarcoSystem::Impl::OnSelectTrackerRequested(int tracker)
{
	std::string trackerType = (tracker ? "External" : "Internal");

	BarcoV2Protocol::RequestSelectTrackerCallbackType
		requestSelectTrackerCallback = [trackerType, tracker, this](
										   unsigned char result) {
			if (result == 0x00)	 // success
			{
				std::cout << "BarcoSystem::SelectTracker - "
						  << "Tracker successfully set to: " << trackerType
						  << std::endl;
				m_TrackerType = tracker;
			}
			else {
				std::cerr << "BarcoSystem::SelectTracker - "
						  << "Could not select " << trackerType << " tracker"
						  << std::endl;
			}
		};

	m_MessageEncoder.RequestSelectTracker(tracker, requestSelectTrackerCallback);
}
//==============================================================================

//==============================================================================
void BarcoSystem::Impl::OnWriteEyePositionsRequested(
	const EyePositionsType& eyePositions)
{
	// Only send the eye coordinates if we're not using the internal Barco
	// eye tracker
	if (m_TrackerType == 1) {
		const auto& leftEyePosition = eyePositions.left;
		const auto& rightEyePosition = eyePositions.right;

		m_MessageEncoder.RequestSendEyeCoordinates(leftEyePosition[0],
			leftEyePosition[1], leftEyePosition[2], rightEyePosition[0],
			rightEyePosition[1], rightEyePosition[2]);

		std::lock_guard<std::mutex> lock(m_EyePositionsMutex);
		m_EyePositions = eyePositions;
	}
}
//==============================================================================

//==============================================================================
auto BarcoSystem::Impl::GetEyePositions() const -> EyePositionsType
{
	std::lock_guard<std::mutex> lock(m_EyePositionsMutex);
	return m_EyePositions;
}
//==============================================================================

//==============================================================================
void BarcoSystem::Impl::HandleError(QSerialPort::SerialPortError error)
{
	QString errorString;

	switch (error) {
		case QSerialPort::NoError: return;
		case QSerialPort::DeviceNotFoundError:
			errorString = "Serial port device not found";
			break;
		case QSerialPort::PermissionError:
			errorString =
				"Device is already opened by another process or you do "
				"not have sufficient permissions and credentials to open it";
			break;
		case QSerialPort::OpenError:
			errorString = "Device is already open";
			break;
		case QSerialPort::NotOpenError:
			errorString =
				"Attempted to perform an operation on a device that was "
				"not open";
			break;
		case QSerialPort::WriteError:
			errorString = "An I/O error occurred while writing data";
			break;
		case QSerialPort::ReadError:
			errorString = "An I/O error occurred while reading data";
			break;
		case QSerialPort::ResourceError:
			errorString = "Device is unavailable";
			break;
		case QSerialPort::UnsupportedOperationError:
			errorString =
				"The requested device operation is not supported or "
				"prohibited by the host operating system";
			break;
		case QSerialPort::TimeoutError:
			errorString = "A timeout error occurred";
			break;
		default: errorString = "An unknown error occurred"; break;
	}

	std::unique_lock<std::mutex> errorStringLocker(m_ErrorStringMutex);
	m_ErrorString.assign(errorString.toStdString());

	errorStringLocker.unlock();

	this->Error(QPrivateSignal());
}
//==============================================================================

//==============================================================================
std::string const BarcoSystem::Impl::GetLastErrorString() const
{
	std::lock_guard<std::mutex> errorStringLocker(m_ErrorStringMutex);
	return m_ErrorString;
}
//==============================================================================

//==============================================================================
// INTERFACE CLASS
//==============================================================================
BarcoSystem::BarcoSystem(const std::string& comPort) :
	m_COMPort{comPort},
	m_Impl{std::make_unique<Impl>(comPort)}
{
	auto availableSerialPorts = QSerialPortInfo::availablePorts();

	auto matchingPortIt = std::find_if(availableSerialPorts.cbegin(),
		availableSerialPorts.cend(), [&comPort](const auto& portInfo) {
			return (portInfo.portName() == QString::fromStdString(comPort));
		});

	if (matchingPortIt == availableSerialPorts.end()) {
		std::string errorStringBase{"Error initializing Barco system: "};
		std::stringstream ss;
		if (comPort.empty()) {
			ss << "no serial port selected";
		}
		else {
			ss << comPort << " is not an available serial port";
		}

		throw std::runtime_error(errorStringBase.append(ss.str()));
	}

	QObject::connect(m_Impl.get(), &Impl::Error, m_Impl.get(), [this]() {
		std::cerr << "Barco system error: " << m_Impl->GetLastErrorString()
				  << std::endl;
	});

	m_Impl->start();

	// Make sure we're ready for event processing before exiting the constructor
	while (m_Impl->isRunning() && !m_Impl->IsInitialized()) {
		std::this_thread::yield();
	}

	auto errorString = m_Impl->GetLastErrorString();
	if (!errorString.empty()) {
		std::stringstream ss;
		ss << "Error initializing Barco system: " << errorString;

		throw std::runtime_error(ss.str());
	}
}
//==============================================================================

//==============================================================================
BarcoSystem::~BarcoSystem()
{
	if (m_Impl && m_Impl->isRunning()) {
		m_Impl->quit();

		if (!m_Impl->wait(5000)) {
			std::cerr << this->ClassName() << "::Destructor"
					  << " - processing thread failed to shut down properly"
					  << std::endl;

			m_Impl->terminate();
			m_Impl->wait();
		}
	}
}
//==============================================================================

//==============================================================================
std::string BarcoSystem::ClassName()
{
	return "BarcoSystem";
}
//==============================================================================

//==============================================================================
void BarcoSystem::SetTrackerType(const TrackerType& trackerType)
{
	int tracker{0};
	switch (trackerType) {
		case TrackerType::Internal: tracker = 0; break;
		case TrackerType::External: tracker = 1; break;
	}

	m_Impl->SelectTracker(tracker);
}
//==============================================================================

//==============================================================================
auto BarcoSystem::GetEyePositions() const -> EyePositionsType
{
	return m_Impl->GetEyePositions();
}
//==============================================================================

//==============================================================================
void BarcoSystem::SetEyePositions(const EyePositionsType& eyePositions)
{
	m_Impl->WriteEyePositions(eyePositions);
}
//==============================================================================

//==============================================================================
void BarcoSystem::SetEnableEventReporting(bool enableEvents)
{
	m_Impl->SetEnableEventReporting(enableEvents);
}
//==============================================================================

//==============================================================================
bool BarcoSystem::GetEnableEventReporting() const
{
	return m_Impl->GetEnableEventReporting();
}
//==============================================================================

//==============================================================================
auto BarcoSystem::getDefaultInstance() -> const std::shared_ptr<BarcoSystem>
{
	static const std::shared_ptr<BarcoSystem> defaultInstance = 
		std::make_shared<BarcoSystem>(Config::getDefaultConfig().barcoCOMPort);

	return defaultInstance;
}
//==============================================================================