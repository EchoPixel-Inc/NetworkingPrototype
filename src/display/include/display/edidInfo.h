#ifndef edidInfo_h
#define edidInfo_h

#include <optional>
#include <variant>
#include <string>

struct EDIDInfo
{
	enum class StereoType {
		NO_STEREO,
		FIELD_RIGHT,
		FIELD_LEFT,
		TWO_WAY_RIGHT_ON_EVEN,
		TWO_WAY_LEFT_ON_EVEN,
		FOUR_WAY_INTERLEAVED,
		SIDE_BY_SIDE
	};

	struct Timing
	{
		int width;
		int height;
		int frequency;
	};

	struct DetailedTiming
	{
		int pixel_clock;
		int h_addr;
		int h_blank;
		int h_sync;
		int h_front_porch;
		int v_addr;
		int v_blank;
		int v_sync;
		int v_front_porch;
		int width_mm;
		int height_mm;
		int right_border;
		int top_border;
		int interlaced;
		StereoType stereo;

		int digital_sync;

		struct DigitalData
		{
			int composite;
			int serrations;
			int negative_vsync;
			int negative_hsync;
		};

		struct AnalogData
		{
			int bipolar;
			int serrations;
			int sync_on_green;
		};

		std::variant<DigitalData, AnalogData> technology;
	};

	struct DisplayDescriptor {};

	struct DigitalData {
		enum class Interface {
			UNDEFINED,
			DVI,
			HDMI_A,
			HDMI_B,
			MDDI,
			DISPLAY_PORT
		};

		int bits_per_primary;
		int rgb444;
		int ycrcb444;
		int ycrcb422;
		Interface interface;
	};

	struct AnalogData {
		enum class ColorType {
			UNDEFINED_COLOR,
			MONOCHROME,
			RGB,
			OTHER_COLOR
		};

		double video_signal_level;
		double sync_signal_level;
		double total_signal_level;
		int blank_to_black;
		int separate_hv_sync;
		int composite_sync_on_h;
		int composite_sync_on_green;
		int serration_on_vsync;
		ColorType color_type;
	};

	int checksum;
	char manufacturer_code[4];
	int product_code;
	unsigned int serial_number;

	std::optional<int> production_week;
	std::optional<int> production_year;
	std::optional<int> model_year;

	int major_version;
	int minor_version;
	int is_digital;

	std::variant<DigitalData, AnalogData> technology;

	std::optional<int> width_mm;
	std::optional<int> height_mm;
	std::optional<double> aspect_ratio;
	std::optional<double> gamma; /* -1.0 if not specified */

	int standby;
	int suspend;
	int active_off;

	int srgb_is_standard;
	int preferred_timing_includes_native;
	int continuous_frequency;

	double red_x;
	double red_y;
	double green_x;
	double green_y;
	double blue_x;
	double blue_y;
	double white_x;
	double white_y;

	Timing established[24]; /* Terminated by 0x0x0 */
	Timing standard[8];

	int n_detailed_timings;

	/* If monitor has a preferred mode, it is the first one
	(whether it has, is determined by the preferred_timing_includes bit.) */
	DetailedTiming detailed_timings[4];

	/* Optional product description */
	std::optional<std::string> dsc_serial_number;
	std::optional<std::string> dsc_product_name;
	std::optional<std::string> dsc_string; /* Unspecified ASCII data */
};

#endif