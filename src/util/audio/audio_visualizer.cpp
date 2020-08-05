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

#include "audio_visualizer.hpp"
#include "../../source/visualizer_source.hpp"
#include "audio_source.hpp"
#include "fifo.hpp"
#include "obs_internal_source.hpp"

namespace audio {

audio_visualizer::audio_visualizer(obs_data_t *data)
{
	update(data);
}

audio_visualizer::~audio_visualizer()
{
	free_audio_source();
}

void audio_visualizer::free_audio_source()
{
	if (m_source.use_count() > 1) {
		warn("Audio source '%s' is still in use (%li references)", m_source_id.c_str(), m_source.use_count());
	}
	m_source = nullptr;
}

void audio_visualizer::update(obs_data_t *data)
{
	if (m_source)
		m_source->update(data);

	auto *new_id = obs_data_get_string(data, S_AUDIO_SOURCE);

	if (!m_source || new_id != m_source_id) {
		m_source_id = new_id;
		if (m_source)
			free_audio_source();

		if (m_source_id.empty() || m_source_id == defaults::audio_source) {
			free_audio_source();
		} else if (m_source_id == "mpd") {
			m_source = make_shared<fifo>(data);
		} else {
			m_source = make_shared<obs_internal_source>(data);
		}
	}
}

void audio_visualizer::properties(obs_properties_t *props)
{
	if (m_source)
		m_source->properties(props);
}

void audio_visualizer::tick(float seconds)
{
	if (m_source)
		m_data_read = m_source->tick(seconds);
	else
		m_data_read = false;

#ifdef LINUX
//    if (m_cfg->auto_clear && !m_data_read) {
//        /* Clear buffer */
//        memset(m_cfg->buffer, 0, m_cfg->sample_size * sizeof(pcm_stereo_sample));
//    }
#endif
}
}
