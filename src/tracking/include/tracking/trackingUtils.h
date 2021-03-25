#ifndef trackingUtils_h
#define trackingUtils_h

#include "tracking/trackingTypes.h"
#include "common/coreTypes.h"

namespace tracking
{
inline HeadPoseType estimateHeadPoseFromEyePositions(
	const common::EyePositions& eyePositions)
{
	using VectorType = HeadPoseType::VectorType;
	VectorType zAxis{0.0, 0.0, 1.0};  // vector defining the optical axis

	VectorType e1{(eyePositions.right - eyePositions.left).normalized()};
	VectorType e3{(zAxis - (zAxis.dot(e1) * e1)).normalized()};
	VectorType e2{(e3.cross(e1)).normalized()};

	HeadPoseType::LinearMatrixType rotationBasis =
		HeadPoseType::LinearMatrixType::Identity();

	rotationBasis.col(0) = e1;
	rotationBasis.col(1) = e2;
	rotationBasis.col(2) = e3;

	HeadPoseType headPose{HeadPoseType::Identity()};

	Eigen::FullPivLU<HeadPoseType::LinearMatrixType> lu(rotationBasis);
	if (lu.isInvertible()) {
		headPose.linear() = rotationBasis;
	}

	headPose.translation() = 0.5 * (eyePositions.left + eyePositions.right);

	return headPose;
}
inline common::EyePositions estimateEyePositionsFromHeadPose(
	const HeadPoseType& headPose, double interpupillaryDistance = 60.0)
{
	return common::EyePositions{
		(headPose *
			HeadPoseType::VectorType{-0.5 * interpupillaryDistance, 0.0, 0.0}),
		(headPose *
			HeadPoseType::VectorType{0.5 * interpupillaryDistance, 0.0, 0.0})};
}
}  // namespace tracking

#endif