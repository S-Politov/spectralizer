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

#pragma once

#define BUFFER_SIZE 1024
#include <string>
#include <obs-module.h>
#include "../../util/util.hpp"

namespace source {
struct config;
}

namespace audio {
/* Base class for audio reading */
class audio_source {
protected:
	std::string m_source_id;
	pcm_stereo_sample *m_buffer = nullptr;
	long long m_sample_size;
	long long m_sample_rate;

public:
	explicit audio_source(obs_data_t *data) { update(data); }

	virtual ~audio_source()
	{
		if (m_buffer)
			bfree(m_buffer);
		m_buffer = nullptr;
	}

	/* obs_source methods */
	virtual void update(obs_data_t *);
	long long sample_size() const { return m_sample_size; }
	long long sample_rate() const { return m_sample_rate; }

	virtual bool tick(float seconds) = 0;
	virtual void resize_buffer();
	virtual void source_changed() = 0;
	virtual void properties(obs_properties_t *props);
	pcm_stereo_sample *buffer() { return m_buffer; }
};
}
