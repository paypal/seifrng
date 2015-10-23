/** @file interfaceCamera.cpp
 *  @brief Definition of the class functions in interfaceCamera.h
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
#include <functional>
#include <numeric>
#include <algorithm>
#include <sstream>

// ----------------
// library includes
// ----------------
#include "interfaceCamera.h"

// -----------
// Constructor
// -----------

/**
 * Constructor
 * @brief Creates InterfaceCamera object and initilizes capture properties.
 */
InterfaceCamera::InterfaceCamera():
	_contShootCount(4), // Images per frame set to 4.
	_exp(2), // Camera exposure param set to 2.
	_bitEntropy(16,0.0f), // Bit occurrence probabilities initialized to 0.
	_bitCountCache(65536) { // Cache to accommodate 16bit sample space.

}

// ----------
// appendData
// ----------

/**
 * @brief Appends available entropic data to the vector byte stream.
 *        Implementation of pure virtual function from RandomSource.
 *
 * @param data a reference to a byte vector to be appended with
 *        entropic data from the camera.
 *
 * @return void
 */
void InterfaceCamera::appendData(std::vector<uint8_t>& data) {
	try {
		// Reserve space for data to be appended.
		data.reserve(data.size() + _cameraData.size());
	} catch (const std::bad_alloc& ba) {
		std::cerr << "[Memory Error] Cannot Append: " << std::endl;
		return;
	}
	std::copy(_cameraData.begin(), _cameraData.end(), std::back_inserter(data));

	// Clear entropic data.
	_cameraData.clear();
	_bitEntropy.clear();
}

// ----------
// bitEntropy
// ----------

/**
 * @brief Returns entropy estimate of current camera samples as
 *        bit occurrence probabilities.
 *        Implementation of pure virtual function from RandomSource.
 *
 * @return double vector with bit occurrence probabilities.
 */
std::vector<double> InterfaceCamera::bitEntropy() {
	// Compute present entropy estimate.
	auto tempEntropy = _bitEntropy;

	// Normalize to compute bit occurrence probabilities.
	double normalizer = static_cast<double>(_cameraData.size() / 2.0f);

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

// -------------
// captureFrames
// -------------

/**
 * @brief Captures frames from a specific camera device. Must be called
 *        sucessfully before accessing bytes or entropy estimate.
 *
 * @param numFrames size_t with num frames to be captured (default 10).
 * @param device int camera device identifier to be used (default 0).
 *
 * @return true, if frames are sucessfully captured.
 */
bool InterfaceCamera::captureFrames(size_t numFrames, int device) {
	bool sucess = true;

	// Call helper to capture numFrames.
	while (numFrames > 0) {
		sucess = sucess && captureHelper(device);
		--numFrames;
	}

	return sucess;
}

// -------------
// captureHelper
// -------------

/**
 * @brief Helper to captureFrames to interact with the camera.
 *
 * @param device int with camera device identifier
 *
 * @return true if frames were captured sucessfully
 */
bool InterfaceCamera::captureHelper(int device) {
	// Instantiate capture device.
	cv::VideoCapture cap(device);

	if (!cap.isOpened()) {
		std::cerr << "Unable to open camera" <<std::endl;
		return false;
	}

	// Set capture exposure properties.
	cap.set(CV_CAP_PROP_EXPOSURE, _exp);

	// Set capture format to 3 channel signed 16 bit samples.
	cap.set(CV_CAP_PROP_FORMAT, CV_16SC3);

	for (int i = 0; i < _contShootCount; ++i) {

		cv::Mat streamImage;
		cap >> streamImage; // Capture image.

		// Compute storage required for image.
		size_t additionalStorage = (
			streamImage.cols   // image rows
			* streamImage.rows // image cols
			* 2                // int16 sample
			* 3               // BGR channels
		);

		// Compute total storage required.
		size_t requiredStorage = _cameraData.size() + additionalStorage;

		// Check if data can be held.
		if (_cameraData.max_size() < requiredStorage) {
			std::cerr << "[Max Capacity] Samples discarded: " << std::endl;
			break; // cannot hold new data
		}

		try {
			_cameraData.reserve(requiredStorage);
			// Convert samples to bytes to be loaded into _cameraData.
			int16toBytes(
				streamImage.begin<int16_t>(),
				streamImage.end<int16_t>(),
				std::back_inserter(_cameraData)
			);

		} catch (const std::bad_alloc& ba) {
			std::cerr << "[Memory Error] Samples discarded: " << std::endl;
			return false;
		}
	}
	return true;
}
