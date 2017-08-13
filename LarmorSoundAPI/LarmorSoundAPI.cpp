/*****************************************************************************
 * LarmorSoundAPI 1.0 2016
 * Copyright (c) 2016 Pier Paolo Ciarravano - http://www.larmor.com
 * All rights reserved.
 *
 * This file is part of LarmorSoundAPI.
 *
 * LarmorSoundAPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LarmorSoundAPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LarmorSoundAPI. If not, see <http://www.gnu.org/licenses/>.
 *
 * Licensees holding a valid commercial license may use this file in
 * accordance with the commercial license agreement provided with the
 * software.
 *
 * Author: Pier Paolo Ciarravano
 *
 ****************************************************************************/

// LarmorSound API header
#include "LarmorSoundAPI.h"

namespace Larmor {

    // Constructor
    //  Takes the file audio filename and read all the audio channels in memory and 
    //  computes the FFT for all the tracks, populating the private class members
    LarmorSound::LarmorSound(const char *filename) : initedCreation(false)
    {
        std::cout << "LarmorSound API v.1.0 Beta 04/11/2016\nAuthor: Pier Paolo \"Larmor\" Ciarravano http://www.larmor.com" << std::endl;

        // init member variables
        initedCreation = false;
        initedPlay = false;
        playing = false;
        playPosition = 0;
        // Heartbeat
        heartbeatActive = false;
        heartbeatThreshold = HEARTBEAT_THRESHOLD_DEFAULT;
        heartbeatLast = 0;

        // deactivation when date_millisec > val1 * val2 on 1st April 2017 (millisec: 1491001200000 = 1146924 * 1300000)
        // https://currentmillis.com/
        uint64_t time_ms_to_1 = 1146924;
        uint64_t time_ms_now = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
        uint64_t time_ms_to_2 = 1300000;
        uint64_t time_ms_to = time_ms_to_1 * time_ms_to_2;
        //std::cout << "TIME NOW: " << time_ms_now << std::endl;
        //std::cout << "TIME DEPRECATED: " << time_ms_to << std::endl;
        if (time_ms_now > time_ms_to) {
            std::cout << "LarmorSound API v.1.0 Beta is deprecated, please visit the author website for further information: http://www.larmor.com" << std::endl;
            return;
        }

        // Aubio FFT
        uint32_t win_s = AUBIO_SAMPLE_BUFFER_SIZE; // window size
        fvec_t * in = new_fvec(win_s); // input buffer
        cvec_t * fftgrain = new_cvec(win_s); // FFT norm and phase
        // create FFT object
        aubio_fft_t * fft = new_aubio_fft(win_s);
        if (!fft) {
            std::cout << "LarmorSound:: Error: could not create fft object!" << std::endl;
            return;
        }

        // Input from Aubio
        uint32_t samplerate_read = 0;
        uint32_t n_channels = 0;
        aubio_source_t *this_source = NULL;
        std::stringstream filename_str;
        filename_str << filename;
        this_source = new_aubio_source(filename_str.str().c_str(), samplerate_read, win_s);
        n_channels = aubio_source_get_channels(this_source);
        numChannels = n_channels;
        fmat_t *mat_in = new_fmat(n_channels, win_s);
        if (this_source == NULL) {
            std::cout << "LarmorSound:: Error: could not open input file: " << filename_str.str() << std::endl;
            return;
        }
        if (samplerate_read == 0) {
            samplerate_read = aubio_source_get_samplerate(this_source);
            samplerate = samplerate_read;
        }
        
        // Prepare channels_samples and FFT spectrum samples
        for (uint8_t channel = 0; channel < n_channels; channel++)
        {
            // audio sample
            vect_smpl channel_sample;
            channels_samples.push_back(channel_sample);
            // FFT spectrum sample
            vect_vect_smpl spectrum_sample;
            spectrum_samples.push_back(spectrum_sample);
        }

        std::cout << "LarmorSound:: Reading input file and computing spectrum..." << std::endl;
        uint32_t read = 0;
        uint32_t total_read = 0;
        uint32_t blocks = 0;

        do
        {
            // read from source
            aubio_source_do_multi(this_source, mat_in, &read);

            for (uint8_t channel = 0; channel < n_channels; channel++)
            {

                fmat_get_channel(mat_in, channel, in);

                // Store track sample
                for (uint32_t i = 0; i < read; i++) {
                    channels_samples[channel].push_back(in->data[i]);
                }

                // Clean previous values
                cvec_zeros(fftgrain);

                // Compute FFT
                aubio_fft_do(fft, in, fftgrain);

                // Store FFT sample
                vect_smpl spectrum_values;
                for (uint32_t j = 0; j < fftgrain->length; j++)
                {
                    smpl_t val = sqrt(fftgrain->norm[j]*fftgrain->norm[j] + fftgrain->phas[j]*fftgrain->phas[j]);
                    spectrum_values.push_back(val);
                }
                spectrum_samples[channel].push_back(spectrum_values);
            }

            blocks++;
            total_read += read;

        } while (read == win_s);

        numSamples = total_read;

        std::cout << "LarmorSound:: read " << (numSamples * 1.0 / samplerate)
            << "s (" << numSamples
            << " samples in " << blocks
            << " blocks of " << win_s
            << ") from " << filename_str.str()
            << " at " << samplerate << "Hz" << std::endl;

        // Close resources
        del_aubio_fft(fft);
        //del_fvec(in); // return "double free or corruption" error
        del_fmat(mat_in);
        del_cvec(fftgrain);
        del_aubio_source(this_source);
        aubio_cleanup();

        initedCreation = true;
    }

    // Destructor
    LarmorSound::~LarmorSound()
    {
        std::cout << "LarmorSound:: Destructor" << std::endl;
        if (initedPlay) {
            SDL_CloseAudio();
        }
    }

    uint32_t LarmorSound::getNumSamples()
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return 0;
        }
        return numSamples;
    }

    uint32_t LarmorSound::getSamplerate()
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return 0;
        }
        return samplerate;
    }

    uint8_t LarmorSound::getNumChannels()
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return 0;
        }
        return numChannels;
    }

    vect_smpl* LarmorSound::getChannelSample(uint8_t numChannel)
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return NULL;
        }
        if (numChannel >= numChannels) {
            std::cout << "LarmorSound:: error channel: " << numChannel << " does not exist!" << std::endl;
            return NULL;
        }
        return &channels_samples[numChannel];
    }

    vect_smpl* LarmorSound::getChannelSpectrum(uint8_t numChannel, uint32_t position)
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return NULL;
        }
        if (numChannel >= numChannels) {
            std::cout << "LarmorSound:: error channel: " << numChannel << " does not exist!" << std::endl;
            return NULL;
        }
        if (position >= numSamples) {
            std::cout << "LarmorSound:: error position: " << position << " does not exist!" << std::endl;
            return NULL;
        }

        uint32_t block = position / AUBIO_SAMPLE_BUFFER_SIZE;
        return &spectrum_samples[numChannel][block];
    }

    smpl_t LarmorSound::getChannelEnergy(uint8_t numChannel, uint32_t position)
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return 0.0;
        }
        if (numChannel >= numChannels) {
            std::cout << "LarmorSound:: error channel: " << numChannel << " does not exist!" << std::endl;
            return 0.0;
        }
        if (position >= numSamples) {
            std::cout << "LarmorSound:: error position: " << position << " does not exist!" << std::endl;
            return 0.0;
        }

        uint32_t block = position / AUBIO_SAMPLE_BUFFER_SIZE;
        smpl_t channelEnergy = 0.0;
        for (uint32_t i = 0; i < spectrum_samples[numChannel][block].size(); i++)
        {
            channelEnergy += spectrum_samples[numChannel][block][i];
        }
        return channelEnergy;
    }

    void LarmorSound::setHeartbeatActive(bool active, uint64_t heartbeatThresholdParam) {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return;
        }
        mutex.lock();

        heartbeatActive = active;
        if (heartbeatThresholdParam != 0) {
            heartbeatThreshold = heartbeatThresholdParam;
        }

        mutex.unlock();
    }

    bool LarmorSound::isHeartbeatActive() {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return false;
        }
        bool result = false;
        mutex.lock();

        result = heartbeatActive;

        mutex.unlock();
        return result;
    }

    void LarmorSound::heartbeat()
    {
        mutex.lock();
        if (playing && heartbeatActive) {
            heartbeatLast = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
            // Prevent SDL race condition issue
            if (SDL_GetAudioStatus() != SDL_AUDIO_PLAYING) {
                SDL_PauseAudio(0);
            }
            //std::cout << "heartbeatLast:: " << heartbeatLast << std::endl;
        }
        mutex.unlock();
    }

    void LarmorSound::forwardSDLCallback(void *userdata, Uint8 *stream, int len)
    {
        static_cast<LarmorSound*>(userdata)->memberSDLCallback(stream, len);
    }

    void LarmorSound::memberSDLCallback(Uint8 *stream, int len)
    {
        mutex.lock();
        //std::cout << "playPosition:"<< playPosition << std::endl;

        // check if nowheartbeat - heartbeatLast > heartbeatThreshold then pause(1) else pause(0)
        if (heartbeatActive) {
            uint64_t heartbeatNow= std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
            //std::cout << "heartbeat elapesed:: " << (heartbeatNow - heartbeatLast) << std::endl;
            if ( (heartbeatNow - heartbeatLast) > heartbeatThreshold ) {
                if (SDL_GetAudioStatus() == SDL_AUDIO_PLAYING) {
                    SDL_PauseAudio(1); //Stop
                }
                mutex.unlock();
                //std::cout << "STOP" << std::endl;
                return;
            } else {
                if (SDL_GetAudioStatus() != SDL_AUDIO_PLAYING) {
                    SDL_PauseAudio(0); //Continue
                }
                //std::cout << "PLAY" << std::endl;
            }
        }

        if (playPosition >= numSamples) {
            playing = false;
            SDL_PauseAudio(1);
            mutex.unlock();
            return;
        }

        // divide 4 because float is 4 bytes, I should use sizeof for best portability
        float *audio_pos = new float[len / 4 * numChannels]; //tracks_sample.size()
        Uint32 idx = 0;
        for (Uint32 i = 0; i < len / 4 / numChannels; i++) // loop per samples
        {
            for (uint8_t c = 0; c < numChannels; c++) // loop per channels
            {
                if ((playPosition + i) < numSamples) {
                    audio_pos[idx++] = channels_samples[c][playPosition + i];
                } else {
                    audio_pos[idx++] = 0.0; //if out of vector element, put 0 values
                }
            }
        }

        //if you want to use the following line then you have to comment 2 lines following
        //SDL_memcpy (stream, (uint8_t *)audio_pos, len); // simply copy from one buffer into the other
        memset(stream, 0, len); // clear buffer
        SDL_MixAudio(stream, (uint8_t *)audio_pos, len, SDL_MIX_MAXVOLUME); // mix from one buffer into another

        len = (numSamples - playPosition) > (len / 4 / numChannels) ? len : ((numSamples - playPosition) * 4 * numChannels);
        playPosition += len / 4 / numChannels;

        delete[] audio_pos;

        if (playPosition >= numSamples) {
            playing = false;
            SDL_PauseAudio(1);
        }

        mutex.unlock();
    }

    bool LarmorSound::initPlay()
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return false;
        }

        mutex.lock();

        // Initialize SDL.
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            std::cout << "LarmorSound:: Error: SDL_Init error!" << std::endl;
            mutex.unlock();
            return false;
        }

        SDL_AudioSpec want, have;

        SDL_memset(&want, 0, sizeof(want));
        want.freq = samplerate;
        want.format = AUDIO_F32;
        want.channels = numChannels;
        want.samples = 4096;
        want.callback = LarmorSound::forwardSDLCallback;
        want.userdata = this;

        if (SDL_OpenAudio(&want, &have) < 0) {
            std::cout << "LarmorSound:: Error: couldn't open audio: " << SDL_GetError() << std::endl;
            mutex.unlock();
            return false;
        } else if (have.format != want.format) {
                std::cout << "LarmorSound:: Error: not AUDIO_F32 audio format available!" << std::endl;
                mutex.unlock();
                return false;
        }

        playPosition = 0;
        playing = false;
        initedPlay = true;

        mutex.unlock();
    }

    bool LarmorSound::play(uint32_t startPosition)
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return false;
        }
        if (!initedPlay) {
            std::cout << "LarmorSound:: error: LarmorSound::initPlay has not been called, nothing to do!" << std::endl;
            return false;
        }
        if (startPosition >= numSamples) {
            std::cout << "LarmorSound:: error startPosition: " << startPosition << " does not exist!" << std::endl;
            return false;
        }
        // removed: without following block you can skip from a position to another in playback
        //if (playing) {
        //    std::cout << "LarmorSound:: stream is already playing!" << std::endl;
        //    return true;
        //}
        bool result = false;
        mutex.lock();

        playPosition = startPosition;
        playing = true;
        SDL_PauseAudio(0);
        result = true;

        mutex.unlock();
        return result;
    }

    bool LarmorSound::play()
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return false;
        }
        if (!initedPlay) {
            std::cout << "LarmorSound:: error: LarmorSound::initPlay has not been called, nothing to do!" << std::endl;
            return false;
        }
        if (playing) {
            std::cout << "LarmorSound:: stream is already playing!" << std::endl;
            return true;
        }
        bool result = false;
        mutex.lock();

        //playPosition = 0;
        playing = true;
        SDL_PauseAudio(0);
        result = true;

        mutex.unlock();
        return result;
    }

    bool LarmorSound::stop()
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return false;
        }
        if (!initedPlay) {
            std::cout << "LarmorSound:: error: LarmorSound::initPlay has not been called, nothing to do!" << std::endl;
            return false;
        }
        if (!playing) {
            std::cout << "LarmorSound:: stream is already stopped!" << std::endl;
            return false;
        }
        bool result = false;
        mutex.lock();

        SDL_PauseAudio(1);
        playing = false;
        result = true;

        mutex.unlock();
        return result;
    }

    bool LarmorSound::isInitedPlay()
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return false;
        }
        bool result = false;
        mutex.lock();

        result = initedPlay;

        mutex.unlock();
        return result;
    }

    bool LarmorSound::isPlaying()
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return false;
        }
        if (!initedPlay) {
            std::cout << "LarmorSound:: error: LarmorSound::initPlay has not been called, nothing to do!" << std::endl;
            return false;
        }
        bool result = false;
        mutex.lock();

        result = playing;

        mutex.unlock();
        return result;
    }

    uint32_t LarmorSound::getPlayPosition()
    {
        if (!initedCreation) {
            std::cout << "LarmorSound:: error was in object creation, nothing to do!" << std::endl;
            return 0;
        }
        uint32_t result = 0;
        mutex.lock();

        result = playPosition;

        mutex.unlock();
        return result;
    }

    bool LarmorSound::closePlay()
    {
        if (!initedPlay) {
            std::cout << "LarmorSound:: error: LarmorSound::initPlay has not been called, nothing to do!" << std::endl;
            return false;
        }
        if (playing) {
            std::cout << "LarmorSound:: stream is playing, cant call closePlay!" << std::endl;
            return false;
        }
        mutex.lock();
        SDL_CloseAudio();
        initedPlay = false;
        std::cout << "LarmorSound:: Close Audio: SDL_CloseAudio" << std::endl;
        mutex.unlock();
        return true;
    }

}
