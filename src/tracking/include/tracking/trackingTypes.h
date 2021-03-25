#ifndef trackingTypes_h
#define trackingTypes_h

#include <Eigen/Dense>
#include <Eigen/Geometry>

namespace tracking
{
	using DevicePoseType = Eigen::Transform<double, 3,
		Eigen::Affine, Eigen::RowMajor | Eigen::DontAlign>;

	using HeadPoseType = DevicePoseType;
	using EyePositionPointType = DevicePoseType::VectorType;
} // end namespace tracking

#endif