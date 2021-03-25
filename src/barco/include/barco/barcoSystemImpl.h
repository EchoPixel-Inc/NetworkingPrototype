#ifndef barcoSystemImpl_h
#define barcoSystemImpl_h

#include "common/coreTypes.h"

#include "barcoSystem.h"
#include "barcoV2Protocol.h"
#include "barcoUartFrame.h"

#include <QSerialPort>
#include <QThread>

#include <string>
#include <mutex>
#include <atomic>

class BarcoSystem::Impl : public QThread
{
	Q_OBJECT;

public:
	using EyePositionsType = BarcoSystem::EyePositionsType;

	/// \brief Constructor
	explicit Impl(const std::string& comPort);

	/// \brief Returns the tracked eye positions
	EyePositionsType GetEyePositions() const;

	/// \brief Writes the eye positions to the Barco display
	void WriteEyePositions(const EyePositionsType&);

	/// \brief Selects the eye tracker (0 - internal, 1 - external)
	void SelectTracker(int);

	/// \brief Gets / Sets whether tracker events should be posted to the
	/// EchoPixel event queue
	void SetEnableEventReporting(bool enableEvents)
	{
		m_EnableEventReporting.store(enableEvents);
	}
	bool GetEnableEventReporting() const
	{
		return m_EnableEventReporting.load();
	}

	std::string const GetLastErrorString() const;

	/// \brief This method is a bit of a hack to ensure that all the event
	/// processing mechanisms have been set up for the serialport connection
	/// before the parent class attempts to communicate with the display
	bool IsInitialized() const { return m_Initialized.load(); }

protected:
	void run() override;
	void OnSelectTrackerRequested(int);
	void OnWriteEyePositionsRequested(const EyePositionsType&);
	void HandleGetEyeCoordinates(
		const BarcoV2Protocol::EyeCoordinatesContainerType&);

protected slots:

	void ReadData();
	void HandleError(QSerialPort::SerialPortError error);

signals:

	void Error(QPrivateSignal);
	void WriteEyePositionsRequested(const EyePositionsType&, QPrivateSignal);
	void SelectTrackerRequested(int, QPrivateSignal);

private:
	std::string m_COMPort;
	std::unique_ptr<QSerialPort> m_SerialPort;
	BarcoV2Protocol m_MessageEncoder;
	BarcoUartFrame m_UartFrame;
	mutable std::mutex m_EyePositionsMutex;
	mutable std::mutex m_ErrorStringMutex;
	std::atomic<bool> m_EnableEventReporting;
	EyePositionsType m_EyePositions;
	std::string m_ErrorString;
	std::atomic<bool> m_Initialized;
	int m_TrackerType;
};

#endif
