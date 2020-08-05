/*************************************************************************
 * This file is part of spectralizer
 * github.con/univrsal/spectralizer
 * Copyright 2020 univrsal <universailp@web.de>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

#include "visualizer_source.hpp"
#include "../util/audio/bar_visualizer.hpp"
#include "../util/audio/wire_visualizer.hpp"
#include "../util/util.hpp"

namespace source {

visualizer_source::visualizer_source(obs_source_t *source, obs_data_t *settings)
{
	m_config.settings = settings;
	m_config.source = source;

	update(settings);
}

visualizer_source::~visualizer_source()
{
	m_config.value_mutex.lock();
	delete m_visualizer;
	m_visualizer = nullptr;

	if (m_config.buffer) {
		bfree(m_config.buffer);
		m_config.buffer = nullptr;
	}
	m_config.value_mutex.unlock();
}

void visualizer_source::update(obs_data_t *settings)
{
	m_update_mutex.lock();
	visual_mode old_mode = m_config.visual;

	m_config.visual = (visual_mode)(obs_data_get_int(settings, S_SOURCE_MODE));
	m_config.color = obs_data_get_int(settings, S_COLOR);
	m_config.fifo_path = obs_data_get_string(settings, S_FIFO_PATH);
	m_config.bar_height = obs_data_get_int(settings, S_BAR_HEIGHT);
	m_config.cx = UTIL_MAX(m_config.detail * (m_config.bar_width + m_config.bar_space) - m_config.bar_space, 10);
	m_config.cy = UTIL_MAX(m_config.bar_height + (m_config.stereo ? m_config.stereo_space : 0), 10);

	//#ifdef LINUX
	//    m_config.auto_clear = obs_data_get_bool(settings, S_AUTO_CLEAR);

	//    struct obs_video_info ovi;
	//    if (obs_get_video_info(&ovi)) {
	//        m_config.fps = ovi.fps_num;
	//    } else {
	//        m_config.fps = 30;
	//        warn("Couldn't determine fps, mpd fifo might not work as intended!");
	//    }
	//#endif

	if (old_mode != m_config.visual || !m_visualizer) {
		delete m_visualizer;

		switch (m_config.visual) {
		case VM_BARS:
			m_visualizer = new audio::bar_visualizer(settings);
			break;
		case VM_WIRE:
			m_visualizer = new audio::wire_visualizer(settings);
			break;
		}
	} else if (m_visualizer) {
		m_visualizer->update(settings);
	}

	m_update_mutex.unlock();
}

void visualizer_source::tick(float seconds)
{
	m_update_mutex.lock();

	if (m_visualizer)
		m_visualizer->tick(seconds);

	m_update_mutex.unlock();
}

void visualizer_source::render(gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);
	if (m_visualizer) {
		m_update_mutex.lock();
		gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
		gs_eparam_t *color = gs_effect_get_param_by_name(solid, "color");
		gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");

		struct vec4 colorVal;
		vec4_from_rgba(&colorVal, m_config.color);
		gs_effect_set_vec4(color, &colorVal);

		gs_technique_begin(tech);
		gs_technique_begin_pass(tech, 0);

		m_visualizer->render(solid);

		gs_technique_end_pass(tech);
		gs_technique_end(tech);
		m_update_mutex.unlock();
	}
}

static bool stereo_changed(obs_properties_t *props, obs_property_t *, obs_data_t *data)
{
	auto stereo = obs_data_get_bool(data, S_STEREO);
	auto *space = obs_properties_get(props, S_STEREO_SPACE);
	obs_property_set_visible(space, stereo);
	return true;
}

static bool visual_mode_changed(obs_properties_t *props, obs_property_t *, obs_data_t *data)
{
	visual_mode vm = (visual_mode)obs_data_get_int(data, S_SOURCE_MODE);
	auto *wire_mode = obs_properties_get(props, S_WIRE_MODE);
	auto *height = obs_properties_get(props, S_BAR_HEIGHT);
	auto *width = obs_properties_get(props, S_BAR_WIDTH);
	auto *space = obs_properties_get(props, S_BAR_SPACE);

	obs_property_set_visible(width, vm != VM_WIRE);
	obs_property_set_description(space, vm == VM_WIRE ? T_WIRE_SPACING : T_BAR_SPACING);
	obs_property_set_description(height, vm == VM_WIRE ? T_WIRE_HEIGHT : T_BAR_HEIGHT);
	obs_property_set_visible(wire_mode, vm == VM_WIRE);
	return true;
}

obs_properties_t *get_properties_for_visualiser(void *data)
{
	auto *vis = reinterpret_cast<visualizer_source *>(data);
	obs_properties_t *props = obs_properties_create();

	vis->visualizer()->properties(props);
	auto *mode =
		obs_properties_add_list(props, S_SOURCE_MODE, T_SOURCE_MODE, OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(mode, T_MODE_BARS, (int)VM_BARS);
	obs_property_list_add_int(mode, T_MODE_WIRE, (int)VM_WIRE);
	obs_property_set_modified_callback(mode, visual_mode_changed);

	auto *stereo = obs_properties_add_bool(props, S_STEREO, T_STEREO);
	auto *space = obs_properties_add_int(props, S_STEREO_SPACE, T_STEREO_SPACE, 0, UINT16_MAX, 1);
	obs_property_int_set_suffix(space, " Pixel");
	auto *dt = obs_properties_add_int(props, S_DETAIL, T_DETAIL, 1, UINT16_MAX, 1);
	obs_property_int_set_suffix(dt, " Bins");
	obs_property_set_visible(space, false);
	obs_property_set_modified_callback(stereo, stereo_changed);

	return props;
}

void register_visualiser()
{
	obs_source_info si = {};
	si.id = "spectralizer";
	si.type = OBS_SOURCE_TYPE_INPUT;
	si.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
	si.get_properties = get_properties_for_visualiser;

	si.get_name = [](void *) { return T_SOURCE; };
	si.create = [](obs_data_t *settings, obs_source_t *source) {
		return static_cast<void *>(new visualizer_source(source, settings));
	};
	si.destroy = [](void *data) { delete reinterpret_cast<visualizer_source *>(data); };
	si.get_width = [](void *data) { return reinterpret_cast<visualizer_source *>(data)->get_width(); };
	si.get_height = [](void *data) { return reinterpret_cast<visualizer_source *>(data)->get_height(); };

	si.get_defaults = [](obs_data_t *settings) {
		obs_data_set_default_int(settings, S_COLOR, 0xFFFFFFFF);
		obs_data_set_default_int(settings, S_DETAIL, defaults::detail);
		obs_data_set_default_bool(settings, S_STEREO, defaults::stereo);
		obs_data_set_default_int(settings, S_SOURCE_MODE, (int)VM_BARS);
		obs_data_set_default_string(settings, S_AUDIO_SOURCE, defaults::audio_source);
		obs_data_set_default_int(settings, S_SAMPLE_RATE, defaults::sample_rate);
		obs_data_set_default_int(settings, S_FILTER_MODE, (int)SM_NONE);
		obs_data_set_default_double(settings, S_FILTER_STRENGTH, defaults::mcat_smooth);
		obs_data_set_default_double(settings, S_GRAVITY, defaults::gravity);
		obs_data_set_default_double(settings, S_FALLOFF, defaults::falloff_weight);
		obs_data_set_default_string(settings, S_FIFO_PATH, defaults::fifo_path);
		obs_data_set_default_int(settings, S_SGS_PASSES, defaults::sgs_passes);
		obs_data_set_default_int(settings, S_SGS_POINTS, defaults::sgs_points);
		obs_data_set_default_int(settings, S_BAR_WIDTH, defaults::bar_width);
		obs_data_set_default_int(settings, S_BAR_HEIGHT, defaults::bar_height);
		obs_data_set_default_int(settings, S_BAR_SPACE, defaults::bar_space);
		obs_data_set_default_bool(settings, S_AUTO_SCALE, defaults::use_auto_scale);
		obs_data_set_default_double(settings, S_SCALE_SIZE, defaults::scale_size);
		obs_data_set_default_double(settings, S_SCALE_BOOST, defaults::scale_boost);
		obs_data_set_default_int(settings, S_WIRE_MODE, defaults::wire_mode);
		obs_data_set_default_int(settings, S_WIRE_THICKNESS, defaults::wire_thickness);
	};

	si.update = [](void *data, obs_data_t *settings) { reinterpret_cast<visualizer_source *>(data)->update(settings); };
	si.video_tick = [](void *data, float seconds) { reinterpret_cast<visualizer_source *>(data)->tick(seconds); };
	si.video_render = [](void *data, gs_effect_t *effect) {
		reinterpret_cast<visualizer_source *>(data)->render(effect);
	};

	obs_register_source(&si);
}

}
