/** @file interfaceMicrophone.h
 *  @brief Class header tasked with recording entropic bytes from a microphone
 *         device and provide an entropy estimate per sample.
 *         Interaction with the microphone is enabled using portaudio libs.
 *         Implements RandomSource.h
 *
 *  @author Aashish Sheshadri
 *  @author Rohit Harchandani
 *  
 *  The MIT License (MIT)
 *  
 *  Copyright (c) 2015 PayPal
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to 
 *  deal in the Software without restriction, including without limitation the 
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER  
 *  DEALINGS IN THE SOFTWARE.
 */

#ifndef INTERFACEMICROPHONE_H
#define INTERFACEMICROPHONE_H

// -----------------
// standard includes
// -----------------
#include <string>
#include <vector>
#include <iterator>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <functional>

// --------------------
// third party includes
// --------------------
#include "portaudio.h"

// ----------------
// library includes
// ----------------
#include "randomSource.h"

/**
 * @class InterfaceMicrophone tasked with recording entropic bytes from a
 *        microphone device and provide an entropy estimate per sample.
 *        Inherits the abstract class RandomSource.
 */
class InterfaceMicrophone: public RandomSource {
public:

	// -----------
	// Constructor
	// -----------

	/**
	 * Constructor
     * @brief Creates InterfaceMicrophone object and initilizes properties.
     */
	InterfaceMicrophone();

	// ----------
	// Destructor
	// ----------

	/**
	 * Destructor
	 * @brief Releases microphone if stream is active and resets state.
	 */
	~InterfaceMicrophone();

	// ----------
	// appendData
	// ----------

	/**
	 * @brief Appends available entropic data to the vector byte stream.
	 *        Implementation of pure virtual function from RandomSource.
	 *
	 * @param data a reference to a byte vector to be appended with
	 *        entropic data from the microphone.
	 *
	 * @return void
	 */
	void appendData(std::vector<uint8_t>& data);

	// ----------
	// bitEntropy
	// ----------

	/**
	 * @brief Returns entropy estimate of current microphone samples as
	 *        bit occurrence probabilities.
	 *        Implementation of pure virtual function from RandomSource.
	 *
	 * @return double vector with bit occurrence probabilities.
	 */
	std::vector<double> bitEntropy();

	// --------
	// initFlow
	// --------

	/**
	 * @brief Establishes connection with a microphone to capture audio samples
	 *        asynchronously.
	 *
	 * @return int with value 1 if audio stream has been initialized, 0 if
	 *         a stream already exisits or -1 if initialization failed.
	 */
	int initFlow();

	// --------
	// StopFlow
	// --------

	/**
	 * @brief Stops recording audio samples and releases microphone. Must be
	 *        called sucessfully before accessing bytes or entropy estimate.
	 * @return true, if termination was successfull.
	 */
	bool stopFlow();

private:

	// ----------
	// paCallback
	// ----------

	/**
	 * @brief Handles data in input and output buffers corresponding to the
	 *        audio stream; triggered asynchronously.
	 *
	 * @param input pointer to recorded data.
 	 * @param output pointer to output data.
 	 * @param frameCount unsigned long with number of frames in input buffer.
 	 * @param timeInfo const pointer to PaStreamCallbackTimeInfo.
 	 * @param statusFlags PaStreamCallbackFlags with status of stream.
 	 * @param userData void pointer to calling object.
 	 *
 	 * @return int communicating paContinue for continuation or PaComplete.
 	 */
	static int paCallback(const void *input,
		void *output,
		unsigned long frameCount,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData
	) {
		// pass along data to memberCallback
		return ((InterfaceMicrophone*)userData)->memberCallback(
			input,
    		output,
			frameCount,
			timeInfo,
			statusFlags
		);
 	}

	// --------------
	// memberCallback
	// --------------

	/**
	 * @brief Interacts with static callback, paCallback, at an object specific
	 *        level.
	 *
	 * @param input pointer to recorded data.
 	 * @param output pointer to output data.
 	 * @param frameCount unsigned long with number of frames in input buffer.
 	 * @param timeInfo const pointer to PaStreamCallbackTimeInfo.
 	 * @param statusFlags PaStreamCallbackFlags with status of stream.
 	 *
 	 * @return int communicating paContinue for continuation or PaComplete.
 	 */
 	int memberCallback(
 		const void *input,
 		void *output,
 		unsigned long frameCount,
 		const PaStreamCallbackTimeInfo* timeInfo,
 		PaStreamCallbackFlags statusFlags
 	);

	// ----------
	// openStream
	// ----------

	/**
 	 * @brief Sets up connection to a microphone device, prepares and opens
 	 *        stream parameters.
 	 *
 	 * @return int with value 1 for opening stream sucessfully or -1 otherwise.
 	 */
 	int openStream();

	// -----------
	// startStream
	// -----------

	/**
	 * @brief Starts a configured stream.
	 *
	 * @return int with value 1 for a successful stream initiation else -1.
	 */
 	int startStream();

 	// -----------
	// closeStream
	// -----------

	/**
	 * @brief Closes a running audio stream. Including, releasing microphone
	 *        device and resetting internal state.
	 *
	 * @return true, if stream was sucessfully stopped.
	 */
	bool closeStream();

	// ------------
	// int16toBytes
	// ------------

	/**
	 * @brief Converts an int16 stream to a byte stream.
	 *
	 * @param begin input iterator to the beginning of the int16 stream.
	 * @param end input iterator to the end of the int16 stream.
	 * @param out output iterator to record bytes from the int16 stream.
	 *
	 * @return void
	 */
	template <typename II, typename OI>
	void int16toBytes(II begin, II end, OI out);

	// ----------------
	// copyNCompEntropy
	// ----------------

	/**
	 * @brief Copies audio samples from buffer to a container and updates
	 *        bit occurrence counts.
	 *
	 * @param begin input iterator to the beginning of the int16 stream.
	 * @param end input iterator to the end of the int16 stream.
	 * @param out output iterator to record bytes from the int16 stream.
	 *
	 * @return void
	 */
	template <typename II, typename OI>
	void copyNCompEntropy(II begin, II end, OI out);

	// ----
	// data
	// ----
	std::vector<int16_t> _microphoneData; // Vector of random samples from mic.
	PaStream* _stream;    // Audio stream.
	PaStreamParameters _inputParameters; // Stream paramters
	double _samplingRate; // Audio recording sampling rate.
	bool _streamInUse;    // Status of audio stream.
	bool _stopCalled;     // Status of recording.
	PaError _err;		  // Error object.
	std::vector<double> _bitEntropy; // Bit occurrence probabilites of data.
	std::vector<std::vector<uint8_t> > _bitCountCache; /* Cache: Bit occurrences
													    * in sample space.
													    */
};

// ------------
// int16toBytes
// ------------

/**
 * @brief Converts an int16 stream to a byte stream.
 *
 * @param begin input iterator to the beginning of the int16 stream.
 * @param end input iterator to the end of the int16 stream.
 * @param out output iterator to record bytes from the int16 stream.
 *
 * @return void
 */
template <typename II, typename OI>
void InterfaceMicrophone::int16toBytes(II begin, II end, OI out) {

	// Loop through int16 stream.
	while (begin != end) {
		// Convert int16 to bytes and load them into out.
		*out = static_cast<uint8_t>(*begin & 0x00FF);
		++out;
		*out = static_cast<uint8_t>((*begin & 0xFF00) >> 8);
		++begin;
	}
}

// ----------------
// copyNCompEntropy
// ----------------

/**
 * @brief Copies audio samples from buffer to a container and updates
 *        bit occurrence counts.
 *
 * @param begin input iterator to the beginning of the int16 stream.
 * @param end input iterator to the end of the int16 stream.
 * @param out output iterator to record bytes from the int16 stream.
 *
 * @return void
 */
template <typename II, typename OI>
void InterfaceMicrophone::copyNCompEntropy(II begin, II end, OI out) {

	// Loop through int16 stream.
	while (begin != end) {
		*out = *begin;

		uint16_t sample = static_cast<uint16_t>(*begin);

		// Check if sample has been encountered before.
		if (_bitCountCache[sample].empty()) {

			uint16_t temp = sample;
			uint8_t bitPos = 0;

			// Compute and record set bits in the sample.
			while (temp) {

				if (temp & 1) {
					_bitCountCache[sample].push_back(bitPos);
				}

				temp = temp >> 1;
				bitPos = bitPos + 1;
			}
		}

		/* Update _bitEntropy with occurrences of set bits in the sample from
		 * the cache.
		 */
		auto cachedSample = _bitCountCache[sample];

		std::for_each (
			cachedSample.begin(),
			cachedSample.end(),
			[this] (uint8_t val) {
				_bitEntropy[val] = _bitEntropy[val] + 1;
			}
		);

		++begin;
		++out;
	}
}

#endif