#ifndef serializationHelper_h
#define serializationHelper_h

#include "common/coreTypes.h"
#include "appcore/applicationObjects.h"
#include "appcore/messages.h"

#include <cereal/types/vector.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/array.hpp>

namespace cereal
{
template <class Archive>
void save(Archive& archive, const common::TransformType& transform)
{
	archive(cereal::make_nvp("x", transform.translation()(0)));
	archive(cereal::make_nvp("y", transform.translation()(1)));
	archive(cereal::make_nvp("z", transform.translation()(2)));

	Eigen::AngleAxisd angleAxisRep(transform.linear());

	archive(cereal::make_nvp("axis1", angleAxisRep.axis()(0)));
	archive(cereal::make_nvp("axis2", angleAxisRep.axis()(1)));
	archive(cereal::make_nvp("axis3", angleAxisRep.axis()(2)));
	archive(cereal::make_nvp("angle", angleAxisRep.angle()));
}

template <class Archive>
void load(Archive& archive, common::TransformType& transform)
{
	Eigen::AngleAxisd angleAxisRep;
	archive(transform.translation()(0), transform.translation()(1),
		transform.translation()(2), angleAxisRep.axis()(0),
		angleAxisRep.axis()(1), angleAxisRep.axis()(2), angleAxisRep.angle());

	// Ensure axis is normalized
	angleAxisRep.axis().normalize();
	transform.linear() = angleAxisRep.toRotationMatrix();
}
//==============================================================================
template <class Archive>
void save(Archive& archive, const common::Vector3dType& vector)
{
	archive(cereal::make_nvp("x", vector(0)));
	archive(cereal::make_nvp("y", vector(1)));
	archive(cereal::make_nvp("z", vector(2)));
}
template <class Archive>
void load(Archive& archive, common::Vector3dType& vector)
{
	archive(vector(0));
	archive(vector(1));
	archive(vector(2));
}
//==============================================================================
template <class Archive>
void serialize(Archive& archive, PeerInfo& peerInfo)
{
	archive(cereal::make_nvp("id", peerInfo.id),
		cereal::make_nvp("alias", peerInfo.alias),
		cereal::make_nvp("color", peerInfo.color));
}
//==============================================================================
template <class Archive>
void serialize(Archive& archive, PeerCredentials& credentials)
{
	archive(cereal::make_nvp("sessionCode", credentials.sessionCode),
		cereal::make_nvp("alias", credentials.alias));
}
//==============================================================================
template <class Archive>
void serialize(Archive& archive, LaserUpdate& l)
{
	archive(
		cereal::make_nvp("id", l.id), cereal::make_nvp("propList", l.propList));
}
//==============================================================================
template <class Archive>
void serialize(Archive& archive, VolumeUpdate& v)
{
	archive(cereal::make_nvp("id", v.id),
		cereal::make_nvp("propList", v.propList),
		cereal::make_nvp("type", v.msgType));
}
//==============================================================================
template <class Archive>
void serialize(Archive& archive, WidgetUpdate& w)
{
	archive(cereal::make_nvp("widgetId", w.widgetId),
		cereal::make_nvp("ownerId", w.ownerId),
		cereal::make_nvp("propList", w.propList),
		cereal::make_nvp("type", w.msgType));
}
//==============================================================================
template <class Archive>
void serialize(Archive& archive, PlaneUpdate& p)
{
	archive(cereal::make_nvp("propList", p.propList));
	archive(cereal::make_nvp("type", p.msgType));
}
//==============================================================================
template <class Archive>
void serialize(Archive& archive, ApplicationObjects& objects)
{
	archive(cereal::make_nvp("lasers", objects.lasers));
	archive(cereal::make_nvp("volume", objects.volume));
	archive(cereal::make_nvp("widgets", objects.widgets));
	archive(cereal::make_nvp("planeWidget", objects.cutplane));
}
//==============================================================================
}  // end namespace cereal

#endif