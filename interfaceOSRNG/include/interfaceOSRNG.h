/** @file interfaceOSRNG.h
 *  @brief Class header tasked with recording entropic bytes from the
 *         Operating System (OS) and provide an entropy estimate per sample.
 *         OS specific interaction is enabled using Crypto++ libs.
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

#ifndef INTERFACEOSRNG_H
#define INTERFACEOSRNG_H

// -----------------
// standard includes
// -----------------

#include <vector>
#include <iterator>
#include <iostream>

// --------------------
// third party includes
// --------------------
#include <osrng.h>

// ----------------
// library includes
// ----------------
#include "randomSource.h"

/**
 * @class InterfaceOSRNG tasked with recording entropic bytes from the OS and
 *        provide an entropy estimate per byte.
 *        Inherits the abstract class RandomSource
 */
class InterfaceOSRNG: public RandomSource {
public:

	// -----------
	// Constructor
	// -----------

	/**
	 * Constructor
	 * @brief Creates InterfaceOSRNG object and initializes properties.
     */
	InterfaceOSRNG();

	// ----------
	// appendData
	// ----------

	/**
	 * @brief Appends available entropic data to the vector byte stream.
	 *        Implementation of pure virtual function from RandomSource.
	 *
	 * @param data a reference to a byte vector to be appended with
	 *        entropic data from the OS.
	 *
	 * @return void
	 */
	void appendData(std::vector<uint8_t>& data);

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
	std::vector<double> bitEntropy();

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
	bool generateRandomBytes(size_t numBytes = 1024*1024);

private:

	// ----------------
	// copyNCompEntropy
	// ----------------

	/**
	 * @brief Copies samples from buffer to a container and updates
	 *        bit occurrence counts.
	 *
	 * @param begin input iterator to the beginning of the byte stream.
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
	std::vector<uint8_t> _osrngData; // Vector of random bytes from OS.
	CryptoPP::AutoSeededRandomPool _generator; // OS random bytes generator.
	std::vector<double> _bitEntropy; // Bit occurrence probabilites of data.
	std::vector<std::vector<uint8_t> > _bitCountCache; /* Cache: Bit occurrences
													    * in sample space.
													    */


};


// ----------------
// copyNCompEntropy
// ----------------

/**
 * @brief Copies samples from buffer to a container and updates
 *        bit occurrence counts.
 *
 * @param begin input iterator to the beginning of the byte stream.
 * @param end input iterator to the end of the int16 stream.
 * @param out output iterator to record bytes from the int16 stream.
 *
 * @return void
 */
template <typename II, typename OI>
void InterfaceOSRNG::copyNCompEntropy(II begin, II end, OI out) {

	// Loop through byte stream.
	while (begin!=end) {
		*out = *begin;
		uint8_t sample = static_cast<uint8_t>(*begin);

		// Check if sample has been encountered before.
		if (_bitCountCache[sample].empty()) {
			uint8_t temp = sample;
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