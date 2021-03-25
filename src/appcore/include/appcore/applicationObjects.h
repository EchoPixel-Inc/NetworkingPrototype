#ifndef applicationObjects_h
#define applicationObjects_h

#include <unordered_map>
#include <memory>

class LaserWidget;
class VolumeWidget;
class SplineWidget;
class PlaneWidget;

class ApplicationObjects
{
public:
	using LaserMap =
		std::unordered_map<unsigned long, std::unique_ptr<LaserWidget>>;
	using WidgetMap =
		std::unordered_map<unsigned long, std::unique_ptr<SplineWidget>>;
	using VolumePtr = std::unique_ptr<VolumeWidget>;
	using PlaneWidgetPtr = std::unique_ptr<PlaneWidget>;

	ApplicationObjects();
	~ApplicationObjects();

	ApplicationObjects(const ApplicationObjects&) = delete;
	ApplicationObjects& operator=(const ApplicationObjects&) = delete;

	ApplicationObjects(ApplicationObjects&&) = default;
	ApplicationObjects& operator=(ApplicationObjects&&) = default;

	LaserMap lasers;
	WidgetMap widgets;
	VolumePtr volume;
	PlaneWidgetPtr cutplane;
};

#endif