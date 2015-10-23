/** @file runcamera.c++
 *  @brief Test public functions from interfaceCamera.h
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
#include <cassert>
#include <numeric>
#include <vector>
#include <cmath>
#include <algorithm>

// ----------------
// library includes
// ----------------
#include "interfaceCamera.h"

// ------------
// captureImage
// ------------

/**
 * @brief Attempt to capture a frame on the default camera device.
 *
 * @return true, if test passed.
 */
int captureImage () {
	std::cerr << "**Running test captureImage**" << std::endl;

	InterfaceCamera camera;

	// Attempt to capture a frame.
	bool retVal = camera.captureFrames(1);

	if (!retVal) {
		// Unable to open camera.
		std::cerr << "!!Failed captureImage test!!" << std::endl;
	} else {
		std::cerr << "--Passed--" << std::endl;
	}

	return retVal;
}

// ---------------
// appendDataValid
// ---------------

/**
 * @brief Attempt to get entropic bytes (recorded from captured frames).
 *
 * @return true, if test passed.
 */
int appendDataValid () {
	std::cerr << "**Running test appendDataValid**" << std::endl;

	InterfaceCamera camera;

	// Capture frames.
	camera.captureFrames(2);
	std::vector<uint8_t> data;

	camera.appendData(data);

	// Check if sum appended data is non-zero.
	size_t sum = std::accumulate(data.begin(), data.end(), 0);
	bool retVal = (sum > 0);

	if (!retVal) {
		std::cerr << "!!Failed appendDataValid test!!" << std::endl;
	} else {
		std::cerr << "--Passed--" << std::endl;
	}

	return retVal;
}

// -------------------
// measureEntropyValid
// -------------------

/**
 * @brief Attempt to measure bit occurrence probabilities (entropy estimate) on
 *        recorded frames.
 *
 * @return true, if test passed.
 */
int measureEntropyValid () {
	std::cerr << "**Running test measureEntropyValid**" << std::endl;

	InterfaceCamera camera;
	bool retVal;

	// Capture frames.
	camera.captureFrames(4);

	// Get entropy estimate.
	std::vector<double> entropy = camera.bitEntropy();

	// Compute mean bit entropy estimate over sample size.
	double normalizer = entropy.size();
	std::transform(
		entropy.begin(),
		entropy.end(),
		entropy.begin(),
		[normalizer] (double val) {
			return val / normalizer;
		}
	);
	double meanEntropy = std::accumulate(entropy.begin(), entropy.end(), 0.0f);

	// Check if mean entropy is atleast 0.1
	retVal = (meanEntropy > 0.1);

	if (!retVal) {
		std::cerr << "!!Failed measureEntropyValid test!!" << std::endl;
	} else {
		std::cerr << "--Passed--" << std::endl;
	}

	return retVal;
}

// -----------------
// appendDataInvalid
// -----------------

/**
 * @brief Attempt to get entropic bytes before initialization of a capture
 *        device.
 * @return true, if test passed.
 */
int appendDataInvalid () {
	std::cerr << "**Running test appendDataInvalid**" << std::endl;

	InterfaceCamera camera;
	std::vector<uint8_t> data;

	// Attempt to get data from camera before capturing frames.
	camera.appendData(data);

	bool retVal = data.empty();

	if (!retVal) {
		std::cerr << "!!Failed appendDataInvalid test!!" << std::endl;
	} else {
		std::cerr << "--Passed--" << std::endl;
	}

	return retVal;
}

// ---------------------
// measureEntropyInvalid
// ---------------------

/**
 * @brief Attempt to measure bit occurrence probabilities (entropy estimate)
 *        when entropic data is not available or has been flushed.
 * @return true, if test passed.
 */
int measureEntropyInvalid () {
	std::cerr << "**Running test measureEntropyInvalid**" << std::endl;

	InterfaceCamera camera;
	bool retVal;
	std::vector<uint8_t> data;

	// Attempt to get entropy estimate before frames are captured.
	std::vector<double> entropy = camera.bitEntropy();
	double meanEntropy = std::accumulate(entropy.begin(), entropy.end(), 0.0f);
	retVal = (std::fabs(meanEntropy) < 0.01f);

	// Capture frames.
	camera.captureFrames(4);

	// Flush data.
	camera.appendData(data);

	// Attempt to get entropy after flushing (mean should ~ 0).
	entropy = camera.bitEntropy();
	meanEntropy = std::accumulate(entropy.begin(), entropy.end(), 0.0f);
	retVal = retVal && (std::fabs(meanEntropy) < 0.01f);

	if (!retVal) {
		std::cerr << "!!Failed measureEntropyInvalid test!!" << std::endl;
	} else {
		std::cerr << "--Passed--" << std::endl;
	}

	return retVal;
}

int main(){
	/* Run tests and count passed.
	 * Order matters.
	 */
	int passed = 0;
	passed += captureImage();
	passed += appendDataValid();
	passed += measureEntropyValid();
	passed += appendDataInvalid();
	passed += measureEntropyInvalid();


	std::cerr << std::endl;
	std::cerr << "--Passed " << passed << "/5" << " tests--" << std::endl;

	// Assert passing all tests.
	assert(passed == 5);
	return 0;
}