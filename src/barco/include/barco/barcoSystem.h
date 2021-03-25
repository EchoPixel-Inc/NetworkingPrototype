#ifndef barcoSystem_h
#define barcoSystem_h

#include "common/coreTypes.h"

#include <string>
#include <utility>
#include <memory>

class QThread;
class QObject;

class BarcoSystem
{
public:
	using EyePositionsType = common::EyePositions;
	enum class TrackerType { Internal, External };

	/// \brief Constructor
	explicit BarcoSystem(const std::string& comPort);

	/// \brief Destructor
	~BarcoSystem();

	/// \brief Returns the name of this class as a string
	static std::string ClassName();

	/// \brief Gets / Sets whether tracker events should be posted to the
	/// EchoPixel event queue
	void SetEnableEventReporting(bool);
	bool GetEnableEventReporting() const;

	/// \brief Sets whether the Barco system internal eye tracker or an
	/// external tracking system should be used
	void SetTrackerType(const TrackerType&);

	/// \brief Writes the input eye positions to the Barco display
	void SetEyePositions(const EyePositionsType&);

	/// \brief Returns the current tracked eye positions
	EyePositionsType GetEyePositions() const;

	/// \brief Returns the default Barco System instance
	static const std::shared_ptr<BarcoSystem> getDefaultInstance();

private:
	class Impl;

	std::string m_COMPort;
	std::unique_ptr<Impl> m_Impl;
};

#endif