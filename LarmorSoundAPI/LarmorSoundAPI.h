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

#ifndef LARMORSOUNDAPI_H_
#define LARMORSOUNDAPI_H_

#include <vector>
#include <mutex> 
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>

// Aubio
#include <aubio.h>
#include "config.h"

// SLD2 for audio play
#include <SDL2/SDL.h>

#define AUBIO_SAMPLE_BUFFER_SIZE 1024
#define HEARTBEAT_THRESHOLD_DEFAULT 500

namespace Larmor {

    // smpl_t is float as defined in Aubio API
    typedef std::vector<smpl_t> vect_smpl;
    typedef std::vector<vect_smpl> vect_vect_smpl;

    class LarmorSound
    {

        private:

            bool initedCreation;
            bool initedPlay;
            bool playing;
            uint32_t numSamples;
            uint32_t samplerate;
            uint8_t numChannels;
            uint32_t playPosition;
            vect_vect_smpl channels_samples;
            std::vector<vect_vect_smpl> spectrum_samples;
            std::mutex mutex;

            // Heartbeat 
            bool heartbeatActive;
            uint64_t heartbeatThreshold;
            uint64_t heartbeatLast;

        public:

            // Constructor
            //  Takes the file audio filename and read all the audio channels in memory and 
            //  computes the FFT for all the tracks, populating the private class members
            LarmorSound(const char *filename);

            // Destructor
            ~LarmorSound();

            uint32_t getNumSamples();

            uint32_t getSamplerate();

            uint8_t getNumChannels();

            vect_smpl* getChannelSample(uint8_t numChannel);

            vect_smpl* getChannelSpectrum(uint8_t numChannel, uint32_t position);

            smpl_t getChannelEnergy(uint8_t numChannel, uint32_t position);

            // It could take as parameter the pointer to a call back function:
            //    void (*userCallback)()
            //  and save userCallback in a member variable.
            //  In this case to call this funtion use:
            //    (*userCallback)();
            bool initPlay();

            bool play(uint32_t startPosition);

            bool play();

            bool stop();

            bool isInitedPlay();

            bool isPlaying();

            uint32_t getPlayPosition();

            bool closePlay();

            void setHeartbeatActive(bool active, uint64_t heartbeatThresholdParam = 0);

            bool isHeartbeatActive();

            void heartbeat();

        private:

            static void forwardSDLCallback(void *userdata, uint8_t *stream, int len);

            void memberSDLCallback(uint8_t *stream, int len);

    };

}

#endif /* LARMORSOUNDAPI_H_ */
