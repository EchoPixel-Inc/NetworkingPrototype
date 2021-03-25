#ifndef vtkCommonConversions_h
#define vtkCommonConversions_h

#include "common/coreTypes.h"

#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkLinearTransform.h>

inline common::TransformType::MatrixType convertVTKToCommonMatrix(
	const vtkMatrix4x4* mtx)
{
	common::TransformType::MatrixType outputMtx;
	outputMtx(0, 0) = mtx->GetElement(0, 0);
	outputMtx(0, 1) = mtx->GetElement(0, 1);
	outputMtx(0, 2) = mtx->GetElement(0, 2);
	outputMtx(0, 3) = mtx->GetElement(0, 3);
	outputMtx(1, 0) = mtx->GetElement(1, 0);
	outputMtx(1, 1) = mtx->GetElement(1, 1);
	outputMtx(1, 2) = mtx->GetElement(1, 2);
	outputMtx(1, 3) = mtx->GetElement(1, 3);
	outputMtx(2, 0) = mtx->GetElement(2, 0);
	outputMtx(2, 1) = mtx->GetElement(2, 1);
	outputMtx(2, 2) = mtx->GetElement(2, 2);
	outputMtx(2, 3) = mtx->GetElement(2, 3);
	outputMtx(3, 0) = mtx->GetElement(3, 0);
	outputMtx(3, 1) = mtx->GetElement(3, 1);
	outputMtx(3, 2) = mtx->GetElement(3, 2);
	outputMtx(3, 3) = mtx->GetElement(3, 3);

	return outputMtx;
}

inline vtkNew<vtkMatrix4x4> convertCommonToVTKMatrix(
	const common::TransformType::MatrixType& mtx)
{
	vtkNew<vtkMatrix4x4> outputMtx;
	outputMtx->DeepCopy(mtx.data());
	return outputMtx;
}

inline common::TransformType convertVTKToCommonTransform(vtkLinearTransform* transform)
{
	common::TransformType outputTransform;
	const auto& matrix = transform->GetMatrix();
	outputTransform.matrix() = convertVTKToCommonMatrix(matrix);

	return outputTransform;
}

inline vtkNew<vtkLinearTransform> convertCommonToVTKTransform(
	const common::TransformType& transform)
{
	vtkNew<vtkTransform> outputTransform;
	outputTransform->GetMatrix()->DeepCopy(transform.matrix().data());

	return outputTransform;
}

#endif
