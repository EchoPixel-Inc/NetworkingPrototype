#include "tracking/ewmaFilter.h"

#include <Eigen/Geometry>

#include <algorithm>
#include <iostream>

namespace tracking
{
namespace
{
	static DevicePoseType::LinearMatrixType AsOrthogonalMatrix(
		const DevicePoseType::LinearMatrixType& mtx)
	{
		DevicePoseType::LinearMatrixType orthogonalized = mtx;

		orthogonalized.col(0).normalize();
		orthogonalized.col(1).normalize();
		orthogonalized.col(2) =
			orthogonalized.col(0).cross(orthogonalized.col(1));

		orthogonalized.col(2).normalize();
		orthogonalized.col(0) =
			orthogonalized.col(1).cross(orthogonalized.col(2));

		orthogonalized.col(0).normalize();

		return orthogonalized;
	}
}  // end anonymous namespace

//==============================================================================
EWMAFilter::EWMAFilter() : m_Alpha{1.0}, m_State{DevicePoseType::Identity()} {}
//==============================================================================

//==============================================================================
EWMAFilter::EWMAFilter(const DevicePoseType& initialEstimate) :
	m_Alpha{1.0},
	m_State{initialEstimate}
{
	// Ensure that the linear part represents a true rotation
	m_State.linear() = AsOrthogonalMatrix(m_State.linear());
}
//==============================================================================

//==============================================================================
EWMAFilter::EWMAFilter(const EWMAFilter&) = default;
EWMAFilter& EWMAFilter::operator=(const EWMAFilter&) = default;
EWMAFilter::EWMAFilter(EWMAFilter&&) = default;
EWMAFilter& EWMAFilter::operator=(EWMAFilter&&) = default;
//==============================================================================

//==============================================================================
EWMAFilter::~EWMAFilter() = default;
//==============================================================================

//==============================================================================
void EWMAFilter::setAlpha(double alpha)
{
	if ((alpha > 1.0) || (alpha < 0.0)) {
		std::cerr << " - input alpha of " << alpha
				  << " is not in the range [0.0, 1.0]" << std::endl;
	}

	m_Alpha = (std::max)(0.0, (std::min)(1.0, alpha));
}
//==============================================================================

//==============================================================================
double EWMAFilter::getAlpha() const
{
	return m_Alpha;
}
//==============================================================================

//==============================================================================
const DevicePoseType& EWMAFilter::update(const DevicePoseType& measurement)
{
	Eigen::Quaterniond currentQuat =
		Eigen::Quaterniond{AsOrthogonalMatrix(measurement.linear())};

	Eigen::Quaterniond pastQuat = Eigen::Quaterniond{m_State.linear()};

	m_State.linear() =
		pastQuat.slerp(m_Alpha, currentQuat).normalized().toRotationMatrix();

	m_State.translation() = m_State.translation() +
		m_Alpha * (measurement.translation() - m_State.translation());

	return m_State;
}
//==============================================================================

//==============================================================================
const DevicePoseType& EWMAFilter::getCurrentEstimate() const
{
	return m_State;
}
//==============================================================================
}  // end namespace tracking