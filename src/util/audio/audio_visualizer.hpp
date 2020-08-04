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

#include <obs-module.h>
#include <graphics/graphics.h>
#include <string>
#include <memory>

using namespace std;

namespace source {
struct config;
}

namespace audio {
class audio_source;

class audio_visualizer {
protected:
	shared_ptr<audio_source> m_source = nullptr;
	string m_source_id = "none"; /* where to read audio from */
	bool m_data_read = false;    /* Audio source will return false if reading failed */

	void free_audio_source();

public:
	audio_visualizer(obs_data_t *);
	virtual ~audio_visualizer();

	virtual void update(obs_data_t *);

	/* Active is set to true, if the current tick is in sync with the
     * user configured fps */
	virtual void tick(float);

	virtual void render(gs_effect_t *) = 0;

	virtual void properties(obs_properties_t *) = 0;
};
}
