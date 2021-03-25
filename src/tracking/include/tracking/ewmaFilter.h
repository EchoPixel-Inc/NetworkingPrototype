#ifndef ewmaFilter_h
#define ewmaFilter_h

#include "tracking/trackingTypes.h"

namespace tracking
{
/// \class EWMAFilter
/// \brief A exponentially-weighted moving average filter for tracking
/// device poses
/// \details An exponentially-weighted moving average (EWMA) filter is a
/// first-order infinite impulse repsonse filter that applies weighting
/// factors that decrease exponentially.  The relative weighting between
/// current and past estimates is determined by the alpha parameter.
/// The current pose estimate is given by:
///
/// est_pose_{n + 1} = est_pose_{n} + alpha * (current_pose - est_pose_{n})
///
/// In this implementation, the necessary binary operations on the rotation
/// component of the overall device pose is accomplished via slerp
/// (spherical linear interpolation) on the equivalent quaternion
/// representation
class EWMAFilter
{
public:
	/// \brief Constructor
	EWMAFilter();
	EWMAFilter(const DevicePoseType& initialEstimate);

	/// \brief Destructor
	~EWMAFilter();

	/// \brief Copy / copy assignment
	EWMAFilter(const EWMAFilter&);
	EWMAFilter& operator=(const EWMAFilter&);

	/// \brief Move / move assignment
	EWMAFilter(EWMAFilter&&);
	EWMAFilter& operator=(EWMAFilter&&);

	/// \brief Updates the filter using the input measurement.
	/// Returns the current estimate for convenience
	/// \warning It is assumed that the input rotation parts represent
	/// "pure" rotations (i.e., the determinant is equal to 1).  This is
	/// enforced by the filter through a pre-orthogonalization, but may
	/// therefore result in conflicting rotation spaces if the input
	/// rotations are not pure.
	const DevicePoseType& update(const DevicePoseType& measurement);

	/// \brief Returns the current estimate
	const DevicePoseType& getCurrentEstimate() const;

	/// \brief Set / get the alpha value (weighting between current and
	/// past estimates).
	/// Must be between [0, 1]
	/// \details Larger values of alpha will weight current measurements
	/// more heavily, resulting in less smoothing.  Smaller values will
	/// produce greater smoothing by relying more heavily on past estimates,
	/// but will introduce lag
	void setAlpha(double);
	double getAlpha() const;

private:
	double m_Alpha;
	DevicePoseType m_State;
};

}  // end namespace tracking

#endif