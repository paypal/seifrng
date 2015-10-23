/** @file interfaceOSRNG.cpp
 *  @brief Definition of the class functions in interfaceOSRNG.h
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

// ----------------
// library includes
// ----------------
#include "interfaceOSRNG.h"

// -----------
// Constructor
// -----------

/**
 * Constructor
 * @brief Creates InterfaceOSRNG object and initializes properties.
 */
InterfaceOSRNG::InterfaceOSRNG():
	_bitEntropy(8,0.0f),  // Bit occurrence probabilities initialized to 0.
	_bitCountCache(256) { // Cache to accommodate 16bit sample space.
}

/**
 * @brief Appends available entropic data to the vector byte stream.
 *        Implementation of pure virtual function from RandomSource.
 *
 * @param data a reference to a byte vector to be appended with
 *        entropic data from the OS.
 *
 * @return void
 */
void InterfaceOSRNG::appendData(std::vector<uint8_t>& data) {
	try {
		// Reserve space for data to be appended.
		data.reserve(data.size() + _osrngData.size());
	} catch (const std::bad_alloc& ba) {
		std::cerr << "[Memory Error] Cannot Append: " << std::endl;
		return;
	}

	// Append available entropic data.
	std::copy(_osrngData.begin(), _osrngData.end(), std::back_inserter(data));

	// Clear entropic data.
	_osrngData.clear();
	_bitEntropy.clear();
}

// ----------
// bitEntropy
// ----------

/**
 * @brief Returns entropy estimate of OS random samples as
 *        bit occurrence probabilities.
 *        Implementation of pure virtual function from RandomSource.
 *
 * @return double vector with bit occurrence probabilities.
 */
std::vector<double> InterfaceOSRNG::bitEntropy() {
	// Compute present entropy estimate.
	auto tempEntropy = _bitEntropy;

	// Normalize bit occurrence in bytes recorded.
	double normalizer = _osrngData.size();

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

// -------------------
// generateRandomBytes
// -------------------

/**
 * @brief Captures random bytes from OS generator.
 *
 * @param size_t with number of bytes to be recorded (default 1MB).
 *
 * @return true, if bytes were generated successfully.
 */
bool InterfaceOSRNG::generateRandomBytes(size_t numBytes) {
	// Compute total storage required.
	size_t requiredStorage = _osrngData.size() + numBytes;

	try {
		// Check if data can be held.
		if (_osrngData.max_size() < requiredStorage) {
			// Update numBytes to max possible.
			numBytes = _osrngData.max_size() - _osrngData.size();

			// Reserve max size.
			_osrngData.reserve(_osrngData.max_size());
		} else {
			_osrngData.reserve(requiredStorage);
		}

		std::vector<uint8_t> tempVec(numBytes);

		try {
			// Generate numBytes from OS generator.
			_generator.GenerateBlock(tempVec.data(), numBytes);
		} catch (...) {
			std::cerr << "[Failed] OS RNG failed to generate bytes" << std::endl;
			return false;
		}

		// Copy bytes and update bit occurrence.
		copyNCompEntropy(
			tempVec.begin(),
			tempVec.end(),
			std::back_inserter(_osrngData)
		);
	} catch (const std::bad_alloc& ba) {
		std::cerr << "[Memory Error] Samples discarded." << std::endl;
		return false;
	}

	return true;
}