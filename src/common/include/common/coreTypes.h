#ifndef coreTypes_h
#define coreTypes_h

#include <Eigen/Geometry>

#include <variant>
#include <vector>
#include <string>
#include <array>

#include <QObject>

namespace common
{
Q_NAMESPACE

using TransformType = Eigen::Transform<double, 3, Eigen::Affine,
	Eigen::RowMajor | Eigen::DontAlign>;

using Point3dType = typename TransformType::VectorType;
using Vector3dType = Point3dType;
using ColorVectorType = std::array<double, 3>;
using IdType = unsigned long;

struct EyePositions
{
	EyePositions(const Point3dType& left = { 0.0, 0.0, 0.0 },
		const Point3dType& right = { 0.0, 0.0, 0.0 }) :
		left{ left }, right{ right }{}
	Point3dType left;
	Point3dType right;
};

namespace detail
{
	template <typename... Ts>
	struct TypeList
	{};

	template <typename T, typename U>
	struct TypeCat;

	template <typename... Ts, typename... Us>
	struct TypeCat<TypeList<Ts...>, TypeList<Us...>>
	{
		using type = TypeList<Ts..., Us...>;
	};

	template <class... Ts>
	std::variant<Ts...> asVariant(TypeList<Ts...>);

	using PropTypes = TypeList<bool, std::uint8_t, std::uint16_t, std::uint32_t,
		std::uint64_t, std::int8_t, std::int16_t, std::int32_t, std::int64_t,
		float, double, std::string, TransformType, Vector3dType, ColorVectorType>;

	using VariantType = decltype(asVariant(PropTypes{}));
}  // namespace detail

using VariantType = detail::VariantType;

using PropertyVariantType =
	decltype(detail::asVariant(detail::TypeCat<detail::PropTypes,
		detail::TypeList<std::vector<detail::VariantType>>>::type{}));

using PropertyListType = std::vector<std::pair<std::string, PropertyVariantType>>;

Q_DECLARE_METATYPE(PropertyVariantType);
Q_DECLARE_METATYPE(PropertyListType);
}  // namespace common

#endif