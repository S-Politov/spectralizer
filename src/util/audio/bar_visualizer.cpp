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

#include "bar_visualizer.hpp"
#include "../../source/visualizer_source.hpp"

namespace audio {

bar_visualizer::bar_visualizer(obs_data_t *data) : spectrum_visualizer(data) {}

void bar_visualizer::properties(obs_properties_t *props)
{
	audio_visualizer::properties(props);
	/* Bar settings */
	auto *w = obs_properties_add_int(props, S_BAR_WIDTH, T_BAR_WIDTH, 1, UINT16_MAX, 1);
	auto *h = obs_properties_add_int(props, S_BAR_HEIGHT, T_BAR_HEIGHT, 10, UINT16_MAX, 1);
	auto *s = obs_properties_add_int(props, S_BAR_SPACE, T_BAR_SPACING, 0, UINT16_MAX, 1);
	auto *sr = obs_properties_add_int(props, S_SAMPLE_RATE, T_SAMPLE_RATE, 128, UINT16_MAX, 10);
	obs_property_int_set_suffix(sr, " Hz");
	obs_property_int_set_suffix(w, " Pixel");
	obs_property_int_set_suffix(h, " Pixel");
	obs_property_int_set_suffix(s, " Pixel");

	obs_property_set_visible(sr, false); /* Sampel rate is only needed for fifo */
}

void bar_visualizer::render(gs_effect_t *effect)
{
	if (m_stereo) {
		size_t i = 0, pos_x = 0;
		uint32_t height_l, height_r;
		uint offset = m_stereo_space / 2;
		uint center = m_bar_height / 2 + offset;

		/* Just in case */
		if (m_bars_left.size() != m_detail + DEAD_BAR_OFFSET)
			m_bars_left.resize(m_detail + DEAD_BAR_OFFSET, 0.0);
		if (m_bars_right.size() != m_detail + DEAD_BAR_OFFSET)
			m_bars_right.resize(m_detail + DEAD_BAR_OFFSET, 0.0);

		for (; i < m_bars_left.size() - DEAD_BAR_OFFSET; i++) { /* Leave the four dead bars the end */
			height_l = UTIL_MAX(static_cast<uint32_t>(round(m_bars_left[i])), 1);
			height_r = UTIL_MAX(static_cast<uint32_t>(round(m_bars_right[i])), 1);

			pos_x = i * (m_bar_width + m_bar_space);

			/* Top */
			gs_matrix_push();
			gs_matrix_translate3f(pos_x, (center - height_l) - offset, 0);
			gs_draw_sprite(nullptr, 0, m_bar_width, height_l);
			gs_matrix_pop();

			/* Bottom */
			gs_matrix_push();
			gs_matrix_translate3f(pos_x, center + offset, 0);
			gs_draw_sprite(nullptr, 0, m_bar_width, height_r);
			gs_matrix_pop();
		}
	} else {
		size_t i = 0, pos_x = 0;
		uint32_t height;
		for (; i < m_bars_left.size() - DEAD_BAR_OFFSET; i++) { /* Leave the four dead bars the end */
			auto val = m_bars_left[i];
			height = UTIL_MAX(static_cast<uint32_t>(round(val)), 1);

			pos_x = i * (m_bar_width + m_bar_space);
			gs_matrix_push();
			gs_matrix_translate3f(pos_x, (m_bar_height - height), 0);
			gs_draw_sprite(nullptr, 0, m_bar_width, height);
			gs_matrix_pop();
		}
	}
	UNUSED_PARAMETER(effect);
}
}
