#include <string.h>
#include <stdbool.h>
#include <obs-module.h>
#include "nicolive.h"
#include "nicolive-ui.h"

static bool adjust_bitrate(obs_output_t *output, long long bitrate)
{
	obs_encoder_t *video_encoder = obs_output_get_video_encoder(output);
	// get audio 0
	obs_encoder_t *audio_encoder = obs_output_get_audio_encoder(output, 0);
	obs_data_t *video_encoder_settings =
			obs_encoder_get_settings(video_encoder);
	obs_data_t *audio_encoder_settings =
			obs_encoder_get_settings(audio_encoder);

	long long video_bitrate = obs_data_get_int(video_encoder_settings,
			"bitrate");
	long long audio_bitrate = obs_data_get_int(audio_encoder_settings,
			"bitrate");

	// the smallest video bitrate is 200?
	if (bitrate - audio_bitrate < 200) {
		nicolive_log_warn("audio bitrate is too large");
		return false;
	}

	if (bitrate != video_bitrate + audio_bitrate) {
		obs_data_set_int(video_encoder_settings, "bitrate",
			bitrate - audio_bitrate);
		obs_encoder_update(video_encoder, video_encoder_settings);
		nicolive_log_debug("adjust bitrate: %lld",
				bitrate - audio_bitrate);
	} else {
		nicolive_log_debug("need not adjust bitrate");
	}

	return true;
}

static const char *rtmp_nicolive_getname(void)
{
	return obs_module_text("NiconicoLive");
}

static void rtmp_nicolive_update(void *data, obs_data_t *settings)
{
	// TODO load viqo every time the setting changes
	if (obs_data_get_bool(settings, "load_viqo")) {
		if (!nicolive_load_viqo_settings(data)) {
			nicolive_mbox_warn(obs_module_text(
					"MessageFailedLoadViqoSettings"));
			obs_data_set_bool(settings, "load_viqo", false);
			nicolive_set_settings(data,
				obs_data_get_string(settings, "mail"),
				obs_data_get_string(settings, "password"),
				obs_data_get_string(settings, "session"));
		}
	} else {
		nicolive_set_settings(data,
				obs_data_get_string(settings, "mail"),
				obs_data_get_string(settings, "password"),
				obs_data_get_string(settings, "session"));
	}

	nicolive_set_enabled_adjust_bitrate(data,
				obs_data_get_bool(settings, "adjust_bitrate"));

	if (obs_data_get_bool(settings, "auto_start")) {
		nicolive_start_watching(data,
				obs_data_get_int(settings, "watch_interval"));
	} else {
		nicolive_stop_watching(data);
	}

	if (obs_data_get_bool(settings, "cmd_server")) {
		if (!nicolive_start_cmd_server(data,
				obs_data_get_int(settings,
						"cmd_server_port"))) {
			nicolive_mbox_warn(obs_module_text(
					"MessageFailedStartCmdServer"));
			obs_data_set_bool(settings, "cmd_server", false);
		}
	} else {
		nicolive_stop_cmd_server(data);
	}
}

static void rtmp_nicolive_destroy(void *data)
{
	nicolive_destroy(data);
}

static void *rtmp_nicolive_create(obs_data_t *settings, obs_service_t *service)
{
	void *data = nicolive_create();
	UNUSED_PARAMETER(service);

	rtmp_nicolive_update(data, settings);

	return data;
}

static bool rtmp_nicolive_initialize(void *data, obs_output_t *output)
{
	bool success = false;
	bool msg_gui = !nicolive_silent_once(data);

	if (nicolive_check_session(data)) {
		if (nicolive_check_live(data)) {
			success = true;
		} else {
			nicolive_msg_warn(msg_gui,
					obs_module_text("MessageNoLive"),
					"cannot start streaming: no live");
			success = false;
		}
	} else {
		nicolive_msg_warn(msg_gui,
				obs_module_text("MessageFailedLogin"),
				"cannot start streaming: failed login");
		success = false;
	}

	if (success && nicolive_enabled_adjust_bitrate(data)) {
		if (!adjust_bitrate(output, nicolive_get_live_bitrate(data))) {
			nicolive_msg_warn(msg_gui,
					obs_module_text(
						"MessageFailedAdjustBitrate"),
					"cannot start streaming: "
					"failed adjust bitrate");
			success = false;
		}
	}
	return success;
}

static void rtmp_nicolive_activate(void *data, obs_data_t *settings)
{
	nicolive_start_streaming(data);
	if (!obs_data_get_bool(settings, "load_viqo"))
		obs_data_set_string(settings, "session",
				nicolive_get_session(data));
}

static void rtmp_nicolive_deactivate(void *data)
{
	nicolive_stop_streaming(data);
}

static bool load_viqo_modified(obs_properties_t *props,
	obs_property_t *prop, obs_data_t *settings)
{
	UNUSED_PARAMETER(prop);
	nicolive_log_debug("load viqo check modified");
	if (obs_data_get_bool(settings, "load_viqo")) {
		obs_property_set_enabled(
				obs_properties_get(props, "mail"), false);
		obs_property_set_enabled(
				obs_properties_get(props, "password"), false);
		obs_property_set_enabled(
				obs_properties_get(props, "session"), false);
	} else {
		obs_property_set_enabled(
				obs_properties_get(props, "mail"), true);
		obs_property_set_enabled(
				obs_properties_get(props, "password"), true);
		obs_property_set_enabled(
				obs_properties_get(props, "session"), true);
	}
	return true;
}

// static bool auto_start_modified(obs_properties_t *props,
// 	obs_property_t *prop, obs_data_t *settings)
// {
// 	UNUSED_PARAMETER(prop);
// 	nicolive_log_debug("auto start check modified");
// 	if (obs_data_get_bool(settings, "auto_start")) {
// 		obs_property_set_enabled(obs_properties_get(props,
// 			"watch_interval"), true);
// 	} else {
// 		obs_property_set_enabled(obs_properties_get(props,
// 			"watch_interval"), false);
// 	}
// 	return true;
// }

static obs_properties_t *rtmp_nicolive_properties(void *data)
{
	UNUSED_PARAMETER(data);
	obs_property_t *prop;
	obs_properties_t *ppts = obs_properties_create();

	obs_properties_add_text(ppts, "mail", obs_module_text("MailAddress"),
			OBS_TEXT_DEFAULT);
	obs_properties_add_text(ppts, "password", obs_module_text("Password"),
			OBS_TEXT_PASSWORD);
	obs_properties_add_text(ppts, "session", obs_module_text("Session"),
			OBS_TEXT_PASSWORD);

	prop = obs_properties_add_bool(ppts, "load_viqo",
			obs_module_text("LoadViqoSettings"));
	obs_property_set_modified_callback(prop, load_viqo_modified);

	obs_properties_add_bool(ppts, "adjust_bitrate",
			obs_module_text("AdjustBitrate"));

	prop = obs_properties_add_bool(ppts, "auto_start",
			obs_module_text("AutoStart"));
	// obs_property_set_modified_callback(prop, auto_start_modified);
	obs_properties_add_int(ppts, "watch_interval",
			obs_module_text("WatchInterval"),
			10, 300, 1);

	obs_properties_add_bool(ppts, "cmd_server",
			obs_module_text("CmdServer"));
	obs_properties_add_int(ppts, "cmd_server_port",
			obs_module_text("CmdServerPort"),
			1, 65535, 1);

	return ppts;
}

static void rtmp_nicolive_defaults(obs_data_t *settings)
{
	nicolive_log_debug("default settings");
	obs_data_set_default_string(settings, "mail",            "");
	obs_data_set_default_string(settings, "password",        "");
	obs_data_set_default_string(settings, "session",         "");
	obs_data_set_default_bool  (settings, "load_viqo",       false);
	obs_data_set_default_bool  (settings, "adjust_bitrate",  true);
	obs_data_set_default_bool  (settings, "auto_start",      false);
	obs_data_set_default_int   (settings, "watch_interval",  60);
	obs_data_set_default_bool  (settings, "cmd_server",      false);
	obs_data_set_default_int   (settings, "cmd_server_port", 25083);
}

static const char *rtmp_nicolive_url(void *data)
{
	return nicolive_get_live_url(data);
}

static const char *rtmp_nicolive_key(void *data)
{
	return nicolive_get_live_key(data);
}

struct obs_service_info rtmp_nicolive_service = {
	.id             = "rtmp_nicolive",
	.get_name       = rtmp_nicolive_getname,
	.create         = rtmp_nicolive_create,
	.destroy        = rtmp_nicolive_destroy,
	.update         = rtmp_nicolive_update,
	.initialize     = rtmp_nicolive_initialize,
	.activate       = rtmp_nicolive_activate,
	.deactivate     = rtmp_nicolive_deactivate,
	.get_defaults   = rtmp_nicolive_defaults,
	.get_properties = rtmp_nicolive_properties,
	.get_url        = rtmp_nicolive_url,
	.get_key        = rtmp_nicolive_key
};

/* module initialize */

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("rtmp-nicolive", "en-US")
OBS_MODULE_AUTHOR("raccy")

bool obs_module_load(void)
{
	obs_register_service(&rtmp_nicolive_service);
	return true;
}

const char *obs_module_name(void)
{
	return obs_module_text("NiconicoLiveModule");
}

const char *obs_module_description(void)
{
	return obs_module_text("NiconicoLiveDescription");
}
