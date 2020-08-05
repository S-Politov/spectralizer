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
#include "../util.hpp"
#include "audio_visualizer.hpp"
#include <kiss_fftr.h>
#include <vector>

#define DEAD_BAR_OFFSET 5 /* The last five bars seem to always be silent, so we cut them off */

/* Save some writing */
using doublev = std::vector<double>;
using uint32v = std::vector<uint32_t>;

namespace audio {

class spectrum_visualizer : public audio_visualizer {
protected:
	size_t m_fft_size, m_fft_overlap, m_bands, m_fft_idx, m_band_idx;
	double m_freq_min, m_freq_max, m_gain_peak, m_gain_rms, m_sens;
	int m_env_peak[2], m_env_rms[2], m_env_fft[2];
	float m_kfft[2], m_kpeak[2], m_kRMS[2];
	kiss_fftr_cfg m_fftCfg[MAX_AUDIO_CHANNELS]; // FFT states for each channel
	float *m_fftIn[MAX_AUDIO_CHANNELS];         // buffer for each channel's FFT input
	float *m_fftOut[MAX_AUDIO_CHANNELS];        // buffer for each channel's FFT output
	float *m_fftKWdw;                           // window function coefficients
	float *m_fftTmpIn;                          // temp FFT processing buffer
	float *m_bandFreq;                          // buffer of band max frequencies
	float *m_bandOut[MAX_AUDIO_CHANNELS];       // buffer of band values
	kiss_fft_cpx *m_fftTmpOut;                  // temp FFT processing buffer
	int m_fftBufW;                              // write index for input ring buffers
	int m_fftBufP;

	/*-------*/
	uint32_t m_last_bar_count;
	bool m_sleeping = false, m_stereo = false, m_auto_scale = false;
	float m_sleep_count = 0.f;
	double m_gravity;
	double m_falloff_weight;
	double m_scale_size;
	double m_low_freq_cutoff, m_high_freq_cutoff;
	smooting_mode m_smoothing;
	float m_mcat_smoothing_factor;
	int m_sgs_passes;
	int m_sgs_points;
	int m_bar_min_height;
	size_t m_detail;
	int m_bar_height;
	int m_bar_space;
	int m_stereo_space;
	int m_bar_width;

	/* fft calculation vars */
	size_t m_fftw_results;
	double *m_fftw_input_left;
	double *m_fftw_input_right;

	/* Frequency cutoff variables */
	uint32v m_low_cutoff_frequencies;
	uint32v m_high_cutoff_frequencies;
	doublev m_frequency_constants_per_bin;

	uint64_t m_silent_runs; /* determines sleep state */

	//    bool prepare_fft_input(pcm_stereo_sample *buffer, uint32_t sample_size, double *fftw_input,
	//                           channel_mode channel_mode);

	//    void create_spectrum_bars(fftw_complex *fftw_output, size_t fftw_results, int32_t win_height,
	//                              uint32_t number_of_bars, doublev *bars, doublev *bars_falloff);

	//    void generate_bars(uint32_t number_of_bars, size_t fftw_results, const uint32v &low_cutoff_frequencies,
	//                       const uint32v &high_cutoff_frequencies, const fftw_complex *fftw_output, doublev *bars) const;

	void recalculate_cutoff_frequencies(uint32_t number_of_bars, uint32v *low_cutoff_frequencies,
										uint32v *high_cutoff_frequencies, doublev *freqconst_per_bin);
	void smooth_bars(doublev *bars);
	void apply_falloff(const doublev &bars, doublev *falloff_bars) const;
	void calculate_moving_average_and_std_dev(double new_value, size_t max_number_of_elements, doublev *old_values,
											  double *moving_average, double *std_dev) const;
	void maybe_reset_scaling_window(double current_max_height, size_t max_number_of_elements, doublev *values,
									double *moving_average, double *std_dev);
	void scale_bars(int32_t height, doublev *bars);
	void sgs_smoothing(doublev *bars);
	void monstercat_smoothing(doublev *bars);

	/* New values are smoothly copied over if smoothing is used
     * otherwise they're directly copied */
	doublev m_bars_left, m_bars_right, m_bars_left_new, m_bars_right_new;
	doublev m_bars_falloff_left, m_bars_falloff_right;
	doublev m_previous_max_heights;
	doublev m_monstercat_smoothing_weights;

public:
	explicit spectrum_visualizer(obs_data_t *);

	~spectrum_visualizer() override;

	void update(obs_data_t *) override;

	void tick(float) override;

	void properties(obs_properties_t *) override;
};

}
