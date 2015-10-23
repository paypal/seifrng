/** @file interfaceMicrophone.cpp
 *  @brief Definition of the class functions in interfaceMicrophone.h
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

// -----------------
// standard includes
// -----------------
#include <algorithm>

// ----------------
// library includes
// ----------------
#include "interfaceMicrophone.h"

// -----------
// Constructor
// -----------

/**
 * Constructor
 * @brief Creates InterfaceMicrophone object and initilizes properties.
 */
InterfaceMicrophone::InterfaceMicrophone():
	_samplingRate(44100),   // Set sampling rate of audio signal.
	_streamInUse(false),    // Reset recording state.
	_stopCalled(false),     // Reset recording state.
	_bitEntropy(16,0.0f),   // Bit occurrence probabilities initialized to 0.
	_bitCountCache(65536) { // Cache to accommodate 16bit sample space.

}

// ----------
// Destructor
// ----------

/**
 * Destructor
 * @brief Releases microphone if stream is active and resets state.
 */
InterfaceMicrophone::~InterfaceMicrophone() {

	// If audio capture is active, stop recording before destroying object.
	if (_streamInUse) {
		stopFlow();
	}
}

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
void InterfaceMicrophone::appendData(std::vector<uint8_t>& data) {

	if (_streamInUse) {
		std::cerr << "Stream still running";
		return;
	}

	try {
		// Reserve space for data to be appended.
		data.reserve(data.size() + (_microphoneData.size() * 2));
	} catch (const std::bad_alloc& ba) {
		std::cerr << "[Memory Error] Cannot Append: " << std::endl;
		return;
	}

	// Convert samples to bytes and copy into data.
	int16toBytes(
		_microphoneData.begin(),
		_microphoneData.end(),
		std::back_inserter(data)
	);

	// Clear entropic data
	_microphoneData.clear();
	_bitEntropy.clear();
}

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
std::vector<double> InterfaceMicrophone::bitEntropy() {
	// Compute present entropy estimate.
	auto tempEntropy = _bitEntropy;

	// Normalize to compute bit occurrence probabilities.
	double normalizer = _microphoneData.size();

	if (std::fabs(normalizer)<0.01) {
		normalizer = 1.0f;
	}

	std::transform(
		tempEntropy.begin(),
		tempEntropy.end(),
		tempEntropy.begin(),
		[normalizer] (double val) {
			return val / normalizer;
		}
	);

	// Return bit occurrence probabilities of entropic data.
	return tempEntropy;
}

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
int InterfaceMicrophone::initFlow() {
	// If stream has already been initialized, return 0 for continuing stream.
	if (_streamInUse) {
		return 0;
	}

	// Prepare port audio to begin interaction with mic.
	_err = Pa_Initialize();

	if (_err != paNoError) {
		std::cerr << "Pa_Initialize error:" << Pa_GetErrorText(_err) << std::endl;
		std::cerr << "Unable to initialize port audio" << std::endl;
		return -1;
	}

	// Attempt to open stream.
	if (openStream() == 1) {
		// Attempt to start stream, return 1 if successful and -1 otherwise.
		return startStream();
	}

	return -1;
}

// --------
// StopFlow
// --------

/**
 * @brief Stops recording audio samples and releases microphone. Must be
 *        called sucessfully before accessing bytes or entropy estimate.
 * @return true, if termination was successfull.
 */
bool InterfaceMicrophone::stopFlow() {

	// Attempt to close stream.
	if (closeStream()) {
		// End port audio interaction with mic.
		_err = Pa_Terminate();
	}

	if (_err != paNoError) {
		std::cerr << "Pa_Terminate error:" << Pa_GetErrorText(_err) << std::endl;
		std::cerr << "Unable to terminate audio stream" << std::endl;
		return false;
	}

	// Reset recording state.
	_streamInUse = false;
	_stopCalled = false;

	return true;
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
int InterfaceMicrophone::memberCallback(
	const void *input,
	void *output,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags
) {

	// Cast input buffer to configured sample type.
	const int16_t* recordedData = (const int16_t*)input;

	// Check if buffer has data.
	if (recordedData == NULL) {
		return paContinue;
	}

	// Compute total storage required.
	size_t requiredStorage = _microphoneData.size() + frameCount;

	// Check if data can be held.
	if (_microphoneData.max_size() < requiredStorage) {
		std::cerr << "[Max Capacity] Samples discarded: " << std::endl;
		return paComplete; // cannot hold new data stop recording
	}

	try {
		// Reserve space to record samples from buffer.
		_microphoneData.reserve(requiredStorage);

		// Copy data from buffer and update bit occurrence in samples.
		copyNCompEntropy(
			recordedData,
			recordedData + frameCount,
			std::back_inserter(_microphoneData)
		);
	} catch (const std::bad_alloc& ba) {
		std::cerr << "[Memory Error] Samples discarded: " << std::endl;
		return paComplete;
	}

	// Check if the stream has been requested to end.
	if (_stopCalled) {
		// Return paComplete to stop recording more samples.
		return paComplete;
	}

	// Continue recording.
	return paContinue;
}

// ----------
// openStream
// ----------

/**
 * @brief Sets up connection to a microphone device, prepares and opens
 *        stream parameters.
 *
 * @return int with value 1 for opening stream sucessfully or -1 otherwise.
 */
int InterfaceMicrophone::openStream() {

    // Find default recording device.
    _inputParameters.device = Pa_GetDefaultInputDevice();

    if (_inputParameters.device == paNoDevice) {
        std::cerr << "Unable to find audio device" << std::endl;
        return -1;
    }

    // Setup recording parameters.
    _inputParameters.channelCount = 2; // Left and Right channels.
    _inputParameters.sampleFormat = paInt16; //16bit Int samples.

    _inputParameters.suggestedLatency =
    	Pa_GetDeviceInfo(_inputParameters.device)->defaultLowInputLatency;

    _inputParameters.hostApiSpecificStreamInfo = NULL;

  	// Open an audio I/O stream.
    _err = Pa_OpenStream(
    	&_stream,						  // stream
		&_inputParameters,				  // input parameters
		NULL,							  // output paramteres
		_samplingRate,					  // sampling rate
		paFramesPerBufferUnspecified,     // samples per callback
		paClipOff,					      // unfiltered
		&InterfaceMicrophone::paCallback, // callback
		this
	);

    if (_err != paNoError) {
    	std::cerr << "Unable to open an audio stream" << std::endl;
    	return -1;
    }

    return 1;
}

// -----------
// startStream
// -----------

/**
 * @brief Starts a configured stream.
 *
 * @return int with value 1 for a successful stream initiation else -1.
 */
int InterfaceMicrophone::startStream() {

	// Start configured stream.
	_err = Pa_StartStream(_stream);

	if (_err!=paNoError) {
		std::cerr << "Pa_StartStream error:" << Pa_GetErrorText(_err) << std::endl;
		return -1;
	}
	_streamInUse = true;

	return 1;
}

// -----------
// closeStream
// -----------

/**
 * @brief Closes a running audio stream. Including, releasing microphone
 *        device and resetting internal state.
 *
 * @return true, if stream was sucessfully stopped.
 */
bool InterfaceMicrophone::closeStream() {
	// Set stop called for stream callback to flush.
	_stopCalled = true;

	// Wait for stream callback to call paComplete.
	while (( _err = Pa_IsStreamActive(_stream)) == 1) {
		Pa_Sleep(1000);
	}

	// Stop stream.
	_err = Pa_StopStream(_stream);

	if (_err != paNoError) {
		std::cerr << "Pa_StopStream error:" << Pa_GetErrorText(_err) << std::endl;
    	std::cerr << "Unable to stop audio stream" << std::endl;
    	return false;
    }

    // Close stream.
    _err = Pa_CloseStream(_stream);

    if (_err != paNoError) {
    	std::cerr << "Unable to close audio stream" << std::endl;
    	return false;
    }
    return true;
}
