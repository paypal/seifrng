/** @file seedGenerator.cpp
 *  @brief Definition of the class functions in seedGenerator.h
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
#include <numeric>

// ----------------
// library includes
// ----------------
#include "seedGenerator.h"

/* Initialize bit occurrence probabilities for bytes.
 * i.e. probability(255) = 1 and probability(0) = 0.
 */
const double SeedGenerator::byteBitProbs[] =
{0, 0.125, 0.125,0.25, 0.125, 0.25, 0.25, 0.375, 0.125, 0.25, 0.25, 0.375, 0.25,
 0.375, 0.375, 0.5,  0.125, 0.25, 0.25, 0.375, 0.25, 0.375, 0.375, 0.5, 0.25,
 0.375, 0.375, 0.5, 0.375, 0.5, 0.5, 0.625,  0.125, 0.25, 0.25, 0.375, 0.25,
 0.375, 0.375, 0.5, 0.25, 0.375, 0.375, 0.5, 0.375, 0.5, 0.5, 0.625, 0.25,
 0.375, 0.375, 0.5, 0.375, 0.5, 0.5, 0.625, 0.375, 0.5, 0.5, 0.625, 0.5, 0.625,
 0.625, 0.75, 0.125, 0.25, 0.25, 0.375, 0.25, 0.375, 0.375, 0.5, 0.25, 0.375,
 0.375, 0.5, 0.375, 0.5, 0.5, 0.625, 0.25, 0.375, 0.375, 0.5, 0.375, 0.5, 0.5,
 0.625, 0.375, 0.5, 0.5, 0.625, 0.5, 0.625, 0.625, 0.75, 0.25, 0.375, 0.375,
 0.5, 0.375, 0.5, 0.5, 0.625, 0.375, 0.5, 0.5, 0.625, 0.5, 0.625, 0.625, 0.75,
 0.375, 0.5, 0.5, 0.625, 0.5, 0.625, 0.625, 0.75, 0.5, 0.625, 0.625, 0.75,
 0.625, 0.75, 0.75, 0.875, 0.125, 0.25, 0.25, 0.375, 0.25, 0.375, 0.375, 0.5,
 0.25, 0.375, 0.375, 0.5, 0.375, 0.5, 0.5, 0.625, 0.25, 0.375, 0.375, 0.5,
 0.375, 0.5, 0.5, 0.625, 0.375, 0.5, 0.5, 0.625, 0.5, 0.625, 0.625, 0.75, 0.25,
 0.375, 0.375, 0.5, 0.375, 0.5, 0.5, 0.625, 0.375, 0.5, 0.5, 0.625, 0.5, 0.625,
 0.625, 0.75, 0.375, 0.5, 0.5, 0.625, 0.5, 0.625, 0.625, 0.75, 0.5, 0.625,
 0.625, 0.75, 0.625, 0.75, 0.75, 0.875, 0.25, 0.375, 0.375, 0.5, 0.375, 0.5,
 0.5, 0.625, 0.375, 0.5, 0.5, 0.625, 0.5, 0.625, 0.625, 0.75, 0.375, 0.5, 0.5,
 0.625, 0.5, 0.625, 0.625, 0.75, 0.5, 0.625, 0.625, 0.75, 0.625, 0.75, 0.75,
 0.875, 0.375, 0.5, 0.5, 0.625, 0.5, 0.625, 0.625, 0.75, 0.5, 0.625, 0.625,
 0.75, 0.625, 0.75, 0.75, 0.875, 0.5, 0.625, 0.625, 0.75, 0.625, 0.75, 0.75,
 0.875, 0.625, 0.75, 0.75, 0.875, 0.75, 0.875, 0.875, 1};

// -----------
// Constructor
// -----------

/**
 * Constructor
 * @brief Creates SeedGenerator object and initilizes internal properties.
 * @param numDivs int indicating number of independent hashes to compute
 *        on data.
 */
SeedGenerator::SeedGenerator(int numDivs):
	_numDivs(numDivs),
	_hashVec(numDivs),
	_digests(numDivs),
	_seedReady(false) {

}

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
bool SeedGenerator::processFromSource(RandomSource* randomSource) {

	// Check if seed can already been computed.
	if (_seedReady) {
		return false; // Cannot process data until seed is flushed or reset.
	}

	// Compute avg. bit occurrence in a sample from randomSource.
	std::vector<double> sampleAvgVec = randomSource->bitEntropy();
	double sum = std::accumulate(sampleAvgVec.begin(),sampleAvgVec.end(),0.0f);
	double avgSampleEntropy = sum/static_cast<double>(sampleAvgVec.size());

	// Check if estimate meets threshold.
	if (avgSampleEntropy < SeedGenerator::ENTROPYTHRESHOLD) {
		// Data not good enough.
		std::cerr << "[Entropy Error] Sample entropy estimate low" << std::endl;
		return false;
	}

	// Load bytes from randomsource into randomData.
	std::vector<uint8_t> randomData;
	randomSource->appendData(randomData);

	// Split random bytes into _numDivs to compute _numDivs hashes.
	auto it = randomData.data();
	int stepSize = randomData.size() / _numDivs;
	int excess = randomData.size() % _numDivs;

	// Loop through batches of data.
	for (int i = 0; i < (_numDivs - 1); ++i) {
		/* Compute avg. bit occurrence in a byte for this batch and check if
		 * it meets the threshold.
		 */
		if (!entropy(it, it + stepSize)) {
			// Data not good enough
			std::cerr << "[Error] Byte entropy estimate low" << std::endl;
			return false;
		}

		// Compute rolling hash for this batch.
		_hashVec[i].Update(it, stepSize);

		it = it + stepSize;
	}

	// Final batch
	if (!entropy(it, it + stepSize + excess)) {
		// Data not good enough
		std::cerr << "[Error] Byte entropy estimate low" << std::endl;
		return false;
	}
	_hashVec[_numDivs-1].Update(it, stepSize + excess);

	return true;
}

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
void SeedGenerator::generateSeed() {

	// Check if seed bytes have already been generated.
	if (_seedReady) {
		return; // Seed bytes available.
	}

	/* Loop through rolling hashes to generate final hashes and load them into
	 * _digests.
	 */
	auto itD = _digests.begin();

	for (
		auto it = _hashVec.begin();
		it != _hashVec.end(); ++it,
		++itD
	) {
		(*it).Final((*itD).data());
	}

	_seedReady = true;
}

// ----------
// resetState
// ----------

/**
 * @brief Resets seed state, enabling generation of a new seed.
 *
 * @return void
 */
void SeedGenerator::resetState() {
	// Check if seed is ready.
	if (_seedReady) {
		// Discard seed.
		_seedReady = false;
	}
}

