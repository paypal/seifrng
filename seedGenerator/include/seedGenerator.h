/** @file seedGenerator.h
 *  @brief Class header tasked with entropy minning and seed generation.
 *         Interacts with objects of type RandomSource to gather entropic
 *         data.
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

#ifndef SEEDGENERATOR_H
#define SEEDGENERATOR_H

// -----------------
// standard includes
// -----------------
#include <iterator>
#include <vector>
#include <array>
#include <iostream>

// --------------------
// third party includes
// --------------------
#include <sha3.h>

// ----------------
// library includes
// ----------------
#include "randomSource.h"


/**
 * @class SeedGenerator tasked with entropy minning and seed generation.
 */
class SeedGenerator {
public:

	// ---------
	// constants
	// ---------

	// Threshold on entropy estimate.
	static constexpr double ENTROPYTHRESHOLD = 0.25;

	// -----------
	// Constructor
	// -----------

	/**
	 * Constructor
	 * @brief Creates SeedGenerator object and initilizes internal properties.
	 * @param numDivs int indicating number of independent hashes to compute
	 *        on data.
	 */
	SeedGenerator(int numDivs);

	// --------
	// copySeed
	// --------

	/**
	 * @brief Writes seed (len terms) into memory pointed to by seed.
	 *
	 * @param seed pointer of type T (template type), pointing to memory to
	 *        store the seed.
	 * @param len size_t with number of seed terms required.
	 *
	 * @return void
	 */
	template <typename T>
	void copySeed(T* seed, size_t len);

	// ----------------
	// processFromSource
	// ----------------

	/**
	 * @brief Computes rolling hash on entropic data from a randomSource if data
	 *        meets threshold on entropy estimate.
	 *
	 * @param randomSource pointer to a RandomSource.
	 *
	 * @return true, if data was entropic enough to be processed.
	 */
	bool processFromSource(RandomSource* randomSource);

	// ------------
	// generateSeed
	// ------------

	/**
	 * @brief Computes final hashes to generate seed bytes. Cannot process more
	 *        data after this invocation, unless seed is copied or resetState
	 *        is invoked.
	 *
	 * @return void
	 */
	void generateSeed();

	// ----------
	// resetState
	// ----------

	/**
	 * @brief Resets seed state, enabling generation of a new seed.
	 *
	 * @return void
	 */
	void resetState();

private:

	// ----------
	// groupBytes
	// ----------

	/**
	 * @brief Groups bytes to generate terms of desired type, i.e. int32, int16
	 *        etc.
	 *
	 * @param begin input iterator to the beginning of the byte stream.
	 * @param end input iterator to the end of the byte stream.
	 * @param out output iterator to record grouped bytes.
	 * @param numBytes int with value equal to grouping size.
	 *
	 * @return output iterator pointing to 1 + last term written.
	 */
	template <typename II, typename OI>
	OI groupBytes(II begin, II end, OI out, size_t numBytes);

	// -------
	// entropy
	// -------

	/**
	 * @brief Computes avg. probabilites of bit occurrence in byte stream.
	 *
	 * @param begin input iterator to the beginning of the byte stream.
	 * @param end input iterator to the end of the byte stream.
	 *
	 * @return true, if entropy estimate of byte stream is acceptable.
	 */
	template <typename II>
	bool entropy(II begin, II end);

	// ----
	// data
	// ----
	std::vector<std::array<uint8_t, CryptoPP::SHA3_512::DIGESTSIZE> > _digests;
	std::vector<CryptoPP::SHA3_512> _hashVec;
	static const double byteBitProbs[];
	int _numDivs;
	bool _seedReady;
};

// --------
// copySeed
// --------

/**
 * @brief Writes seed (len terms) into memory pointed to by seed.
 *
 * @param seed pointer of type T (template type), pointing to memory to
 *        store the seed.
 * @param len size_t with number of seed terms required.
 *
 * @return void
 */
template <typename T>
void SeedGenerator::copySeed(T* seed, size_t len) {

	// Check if seed is ready i.e. generateSeed has been called.
	if (!_seedReady) {
		return;
	}

	size_t numBytes = sizeof(T); // Number of bytes per seed term.

	// Check if numBytes is a power of 2.
	if ((numBytes & (numBytes - 1)) != 0) {
		return; // Cannot write seed terms of this type.
	}

	// Seed terms possible per hash.
	int possibleGroups = (_digests[0]).size() / numBytes;

	// Check if sufficient hashes have been computed to accommodate len terms.
	if (len > (possibleGroups * _digests.size())) {
		return; // Insufficient data to compute len terms.
	}

	// Loop through computed hashes.
	for (auto it = _digests.begin(); it != _digests.end(); ++it) {

		auto endIt = (*it).end(); // end iterator

		// Check if fewer than possibleGroups need to be written.
		if (possibleGroups > len) {
			// Update end iterator to write only len terms.
			endIt = (*it).begin() + len;
			// Group hash bytes into terms and write to seed.
			seed = groupBytes((*it).begin(), endIt, seed, numBytes);
			break; // done
		}
		// Group hash bytes into terms and write to seed.
		seed = groupBytes((*it).begin(), endIt, seed, numBytes);

		// Decrement len to update remaining terms to be written.
		len = len - possibleGroups;
	}

	// Reset state of _seedReady so that a new seed can be generated.
	_seedReady = false;
}

// ----------
// groupBytes
// ----------

/**
 * @brief Groups bytes to generate terms of desired type, i.e. int32, int16
 *        etc.
 *
 * @param begin input iterator to the beginning of the byte stream.
 * @param end input iterator to the end of the byte stream.
 * @param out output iterator to record grouped bytes.
 * @param numBytes int with value equal to grouping size.
 *
 * @return output iterator pointing to 1 + last term written.
 */
template <typename II, typename OI>
OI SeedGenerator::groupBytes(II begin, II end, OI out, size_t numBytes) {

	// Loop through bytes.
	while (begin != end) {
		*out = 0; // reset term

		// Read numBytes from byte stream.
		for (int j = 0; j < numBytes; ++j) {
			// Load bytes into term.
			*out = ((*out) << 8) + *begin;
			++begin;
		}
		++out; // Proceed to next term.
	}
	return out;
}

// -------
// entropy
// -------

/**
 * @brief Computes avg. probabilites of bit occurrence in byte stream.
 *
 * @param begin input iterator to the beginning of the byte stream.
 * @param end input iterator to the end of the byte stream.
 *
 * @return true, if entropy estimate of byte stream is acceptable.
 */
template <typename II>
bool SeedGenerator::entropy(II begin, II end) {
	double byteProbSum = 0.0f; // Reset accumulator of byte stream.
	size_t size = end - begin; // Number of bytes for normalization.

	// Loop through byte stream.
	while (begin != end) {
		// Access byte bit occurrence probability from cache and accumulate.
		byteProbSum += SeedGenerator::byteBitProbs[*begin];
		++begin;
	}

	// Normalize
	double byteAvgProb = byteProbSum / static_cast<double>(size);

	// Check threshold.
	return byteAvgProb > SeedGenerator::ENTROPYTHRESHOLD;
}

#endif