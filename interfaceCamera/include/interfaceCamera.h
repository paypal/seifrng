/** @file interfaceCamera.h
 *  @brief Class header tasked with recording entropic bytes from a camera
 *         device and provide an entropy estimate per sample.
 *         Interaction with the camera is enabled using OpenCV libs.
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

#ifndef INTERFACECAMERA_H
#define INTERFACECAMERA_H

// -----------------
// standard includes
// -----------------
#include <vector>
#include <iterator>
#include <sstream>
#include <iomanip>
#include <iostream>

// --------------------
// third party includes
// --------------------
#include <opencv2/opencv.hpp>


// ----------------
// library includes
// ----------------
#include "randomSource.h"

// ---------------
// InterfaceCamera
// ---------------

/**
 * @class InterfaceCamera tasked with recording entropic bytes from a camera
 *        device and provide an entropy estimate per sample.
 *        Inherits the abstract class RandomSource.
 */
class InterfaceCamera: public RandomSource {
public:

	// -----------
	// Constructor
	// -----------

	/**
	 * Constructor
     * @brief Creates InterfaceCamera object and initilizes capture properties.
     */
	InterfaceCamera();

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
	void appendData(std::vector<uint8_t>& data);

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
	std::vector<double> bitEntropy();

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
	bool captureFrames(size_t numFrames = 10, int device = 0);

private:

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
	bool captureHelper(int device);

	// ------------
	// int16toBytes
	// ------------

	/**
	 * @brief Converts an int16 stream to a byte stream and records bit
	 *        occurrence over the int16 sample.
	 *
	 * @param begin input iterator to the beginning of the int16 stream.
	 * @param end input iterator to the end of the int16 stream.
	 * @param out output iterator to record bytes from the int16 stream.
	 *
	 * @return void
	 */
	template <typename II, typename OI>
	void int16toBytes(II begin, II end, OI out);

	// ----
	// data
	// ----

	std::vector<uint8_t> _cameraData; // Vector of random bytes from camera.
	int _contShootCount; // Number of frames per activation of the camera.
	int _exp; // Exposure of the camera.
	std::vector<double> _bitEntropy; // Bit occurrence probabilites of data.
	std::vector<std::vector<uint8_t> > _bitCountCache; /* Cache: Bit occurrences
													    * in sample space.
													    */
};

// ------------
// int16toBytes
// ------------

/**
 * @brief Converts an int16 stream to a byte stream and records bit
 *        occurrence over the int16 sample.
 *
 * @param begin input iterator to the beginning of the int16 stream.
 * @param end input iterator to the end of the int16 stream.
 * @param out output iterator to record bytes from the int16 stream.
 *
 * @return void
 */
template <typename II, typename OI>
void InterfaceCamera::int16toBytes(II begin, II end, OI out) {

	// Loop through int16 stream.
	while (begin != end){

		uint16_t sample = static_cast<uint16_t>(*begin);

		// Check if sample has been encountered before.
		if (_bitCountCache[sample].empty()) {
			uint16_t temp = sample;
			uint8_t bitPos = 0;

			// Compute and record set bits in the sample
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

		// Convert int16 to bytes and load them into out.
		*out = static_cast<uint8_t>(*begin & uint16_t(0x00FF));
		++out;
		*out = static_cast<uint8_t>((*begin & uint16_t(0xFF00)) >> 8);
		++begin;
	}
}

#endif
