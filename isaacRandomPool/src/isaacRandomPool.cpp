/** @file isaacRandomPool.cpp
 *  @brief Definition of the class functions in isaacRandomPool.h
 *
 *  @author Aashish Sheshadri
 *  @author Rohit Harchandani
 *
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2015, 2016, 2017 PayPal
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
#include <iostream>
#include <cmath>

// --------------------
// third party includes
// --------------------
#include <sha3.h>

// ----------------
// library includes
// ----------------
#include "isaacRandomPool.h"
#include "seedGenerator.h"
#include "interfaceOSRNG.h"

#ifndef WITH_OPENCV
	#define WITH_OPENCV 0
	#include "ignoreCamera.hpp"
#else
	#include "interfaceCamera.h"
#endif

#ifndef WITH_PORTAUDIO
	#define WITH_PORTAUDIO 0
	#include "ignoreMicrophone.hpp"
#else
	#include "interfaceMicrophone.h"
#endif



// -------------
// GenerateBlock
// -------------

/**
 * @brief Interacts with ISAAC generator to generate a random block.
 *        Virtual function override from base class RandomNumberGenerator
 *
 * @param output byte pointer, pointing to a random block of bytes of length
 *        size.
 * @param size size_t with requested length of random byte block.
 *
 * @throw runtime_error if call is made before successful initialization.
 *
 * @return void
 */
void IsaacRandomPool::GenerateBlock(byte *output, size_t size) {
	if (!_isaacrng.initialized()) {
		throw std::runtime_error("RNG has not been initialized.");
	}

	if (size <= 0) {
		return;
	}

	/* Compute number of hashes required for size bytes.
	 * i.e. 32 bytes per 256bit hash => two hashes for two int32 terms.
	 */
	int numHashes = (size % 32) ? (size / 32) + 1 : (size / 32);

	// Vector to store random bytes from rng for each hash.
	std::vector<std::vector<int32_t> > randomDataVec(numHashes);

	for (int j = 0; j < numHashes; ++j) {
		/* Assuming 0.5 bits of entropy per byte of data from rng; we generate
		 * 16 bytes of random data for each byte. To distribute entropy evenly
		 * we hash random bytes; hence for a 32 byte hash (256 bits) we generate
		 * 32 * 16 = 512 random bytes. _isaacrng generates 32bit uint samples,
		 * hence 128 uints.
		 */
		for (size_t i = 0; i < 128; ++i) {
			randomDataVec[j].push_back(_isaacrng.rand());
		}
	}

	int total = 0; // Track bytes generated.
	for (int i = 0; i < numHashes; ++i) {
		std::vector<uint8_t> bytes;
		bytes.reserve(randomDataVec[i].size() * 4);

		// Convert uint32s from rng to bytes.
		int32toBytes(randomDataVec[i].begin(),
			         randomDataVec[i].end(),
					 std::back_inserter(bytes));

		// Hash bytes from rng to generate 32 byte hash.
		std::array<uint8_t, CryptoPP::SHA3_256::DIGESTSIZE> digest;
		CryptoPP::SHA3_256 hash;
		hash.Update(bytes.data(), bytes.size());
		hash.Final(digest.data());

		// Check if total bytes + new bytes is in excess of bytes requested.
		if (total + digest.size() > size) {
			// Copy only (size - total) bytes from hash into output.
			output = std::copy(
				digest.begin(),
				digest.begin() + (size - total),
				output
			);
			break;
		}
		// Copy bytes from hash into output.
		output = std::copy(digest.begin(), digest.end(), output);

		total += digest.size();

	}

}

// ---------------
// EntropyStrength
// ---------------

/**
 * @brief Returns the possible strength of entropy avaible for mining.
 *
 * @return A string with values "WEAK", "MEDIUM" or "STRONG". If the only
 *		   source of entropy is the OS this makes the module's strength
 *		   WEAK w.r.t entropy, access to either the microphone or camera
 *		   results in Medium strength and finally access to the OS, camera,
 *		   microphone and more enables STRONG strength.
 */
std::string IsaacRandomPool::EntropyStrength() {
	if (WITH_OPENCV && WITH_PORTAUDIO) {
		return "STRONG";
	}

	if (WITH_OPENCV || WITH_PORTAUDIO) {
		return "MEDIUM";
	}

	return "WEAK";
}


// -------------
// IsInitialized
// -------------

/**
 * @brief Checks if the internal ISAAC generator has been previously
 *        initialized.
 *
 * @param file constant reference to a string holding file name of the file
 *        containing ISAAC state.
 * @param key vector of uint8_t containing decryption key (AES-GCM) of
 *        valid length. Key updates internal ISAAC generator to use
 *        encryption.
 *        Empty by default: assumes state does not require decryption.
 *
 * @return an enum of type STATUS.
 */
IsaacRandomPool::STATUS IsaacRandomPool::IsInitialized(
	const std::string& file,
	std::vector<uint8_t> key
) {

	/* Attempt to read and decrypt (if key is not empty) state data; initialize
	 * rng with state if successful.
	 */
	int status = _isaacrng.initialize(file, key);

	if (status == 0) {
		// Successful initialization of rng from previous state.
		return STATUS::SUCCESS;
	}

	if (status == -1) {
		// File does not exist.
		return STATUS::FILE_NOT_FOUND;
	}

	if (status == -2) {
		// Failed to read state/load state of rng sucessfully.
		return STATUS::DECRYPTION_ERROR;
	}

	return STATUS::RNG_INIT_ERROR;
}

// ----------
// Initialize
// ----------

/**
 * @brief Initializes ISAAC generator and sets state encryption key.
 *        Function performs entropy estimation to generate a seed.
 *
 * @param file const reference to a string with the file name (with path);
 *        will be created to save state.
 * @param multiplier size_t value increasing entropy mining params as an
 *        exponent of 2.
 * @param key vector of uint8_t containing encryption key (AES-GCM) of
 *        valid length. Key updates internal ISAAC generator to use
 *        encryption.
 *        Empty by default: does not encrypt state data on saving to disk.
 *
 * @throw runtime_error if entropy source fails to be accessed.
 *
 * @return true, if rng was seeded and initialized successfully.
 */
bool IsaacRandomPool::Initialize(
	const std::string& file,
	size_t multiplier,
	std::vector<uint8_t> key
) {

	/* Destroy state of ISAAC generator, prepare for re-initialization if already
	 * valid.
	 */
	_isaacrng.destroy();

	// Set filename of file to store ISAAC generator state.
	_isaacrng.setIdentifier(file);

	// set decryption key
	_isaacrng.setKey(key);

	// gather entropy and seed to initialize ISAAC generator.
	bool result;
	try {
		result = GatherEntropyAndSeed(multiplier);
	} catch (std::runtime_error& e) {
		throw e;
	}

	return result;

}

// --------------------
// InitializeEncryption
// --------------------

/**
 * @brief Sets an encryption key for internal the ISAAC generator.
 *
 * @param key const reference to a vector of uint8_t containing an
 *        encryption key of valid length.
 *
 * @return void
 */
void IsaacRandomPool::InitializeEncryption(const std::vector<uint8_t>& key) {
	// Set encryption key.
	_isaacrng.setKey(key);
}


// ---------
// SaveState
// ---------

/**
 * @brief Encrypts and saves the RNG state to the disk.
 *
 * @return an enum of type STATUS
 */
IsaacRandomPool::STATUS IsaacRandomPool::SaveState() {
	// Save the state to the disk and return the status.
	bool status = _isaacrng.saveState();

	if (status == true) {
		return STATUS::SUCCESS;
	}

	return STATUS::RNG_INIT_ERROR;
}


// -------
// Destroy
// -------

/**
 * @brief Saves current state and resets internal ISAAC generator to an
 *        uninitialized state.
 *
 * @return void
 */
void IsaacRandomPool::Destroy() {
	// Save and destroy ISAAC generator state.
	_isaacrng.destroy();
}

// --------------------
// GatherEntropyAndSeed
// --------------------

/**
 * @brief Interacts with entropic sources to collect random bytes to seed
 *        ISAAC generator.
 *
 * @param mulriplier int value increasing entropy mining params as an
 *        exponent of 2.
 *
 * @throw runtime_error if entropy source fails to be accessed.
 *
 * @return true, if entropy minning was successfull (causally seeding).
 */
bool IsaacRandomPool::GatherEntropyAndSeed(int multiplier) {

	bool status;
	bool result;

	/* Setup SeedGenerator to generate a seed of length SEEDTERMS.
	 * Paramter to the constructor determintes number of splits to data
	 * collected to generate independent hashes to fill SEEDTERMS i.e.
	 * 2 splits => 2 hashes of 512 bytes => 32, 32bit terms or 64, 16bit terms
	 * and so on.
	 */
	SeedGenerator seedGenerator(IsaacRandomPool::ENTROPYSPLIT);

	/* Check if access to the microphone is possible.
	 * If Not check if the camera is accessible.
	 * Rely on the OS for any compensation.
	 */
	if (WITH_PORTAUDIO == 1) {

		// Set result to true initially.
		result = true;

		// Set up microphone to record samples.
		InterfaceMicrophone interfaceMicrophone;

		// Start recording and collecting data (async).
	    status = interfaceMicrophone.initFlow();

	    if(!status) {
	    	throw std::runtime_error("Cannot open microphone device.");
	    }

		// Access more entropy from OS if neccessary.
		int entropyCompensation = 0;

		// Check if access to camera is possible.
		if (WITH_OPENCV == 1) {
			// Set up camera to record samples.
			InterfaceCamera interfaceCamera;

			// Capture samples from camera, increase num samples as an exponent of 2
		    // determined by multiplier.
		    status = interfaceCamera.captureFrames(
		    	IsaacRandomPool::NUM_CAPTURE_FRAMES
		    	* std::pow(2, multiplier)
			);

			if(!status) {
		    	throw std::runtime_error("Cannot open camera device.");
		    }

			result = seedGenerator.processFromSource(&interfaceCamera);
		} else {
			entropyCompensation = 1;
		}

		// Set up OS rng to record samples.
		InterfaceOSRNG interfaceOSRNG;

		// Capture bytes from os randomness, increase num samples as an exponent
		// of 2 determined by multiplier.
		status = interfaceOSRNG.generateRandomBytes(
			IsaacRandomPool::NUM_OS_RANDOM_BYTES
			* std::pow(2, multiplier + entropyCompensation)
		);

		if(!status) {
			throw std::runtime_error("Cannot tap OS entropy.");
		}

	    // Sleep to gather more samples from microphone.
	    Pa_Sleep(IsaacRandomPool::NUM_MIC_SLEEP_MS);

	    // Stop listening on the microphone.
	    interfaceMicrophone.stopFlow();

	    /* Load data from sources as random bytes to seedGenerator.
	     * A false result indicates the source failed to gather sufficient entropy.
	     */
	    result = result && seedGenerator.processFromSource(&interfaceOSRNG);
	    result = result && seedGenerator.processFromSource(&interfaceMicrophone);

	    // Check if data is entropic enough.
	    if (!result) {
	    	// Not enough entropy.
	    	return false;
	    }
	} else if (WITH_OPENCV == 1) {

		// Access more entropy from OS if neccessary.
		int entropyCompensation = 1;

		// Set up camera to record samples.
		InterfaceCamera interfaceCamera;

		// Capture samples from camera, increase num samples as an exponent of 2
	    // determined by multiplier.
	    status = interfaceCamera.captureFrames(
	    	IsaacRandomPool::NUM_CAPTURE_FRAMES
	    	* std::pow(2, multiplier)
		);

		if(!status) {
	    	throw std::runtime_error("Cannot open camera device.");
	    }

		// Set up OS rng to record samples.
		InterfaceOSRNG interfaceOSRNG;

		// Capture bytes from os randomness, increase num samples as an exponent
		// of 2 determined by multiplier.
		status = interfaceOSRNG.generateRandomBytes(
			IsaacRandomPool::NUM_OS_RANDOM_BYTES
			* std::pow(2, multiplier + entropyCompensation)
		);

		if(!status) {
			throw std::runtime_error("Cannot tap OS entropy.");
		}

	    /* Load data from sources as random bytes to seedGenerator.
	     * A false result indicates the source failed to gather sufficient entropy.
	     */
		result = seedGenerator.processFromSource(&interfaceCamera);
	    result = result && seedGenerator.processFromSource(&interfaceOSRNG);

	    // Check if data is entropic enough.
	    if (!result) {
	    	// Not enough entropy.
	    	return false;
	    }
	} else {

		// Access more entropy from OS if neccessary.
		int entropyCompensation = 2;

		// Set up OS rng to record samples.
		InterfaceOSRNG interfaceOSRNG;

		// Capture bytes from os randomness, increase num samples as an exponent
		// of 2 determined by multiplier.
		status = interfaceOSRNG.generateRandomBytes(
			IsaacRandomPool::NUM_OS_RANDOM_BYTES
			* std::pow(2, multiplier + entropyCompensation)
		);

		if(!status) {
			throw std::runtime_error("Cannot tap OS entropy.");
		}

	    /* Load data from sources as random bytes to seedGenerator.
	     * A false result indicates the source failed to gather sufficient entropy.
	     */
	    result = seedGenerator.processFromSource(&interfaceOSRNG);

	    // Check if data is entropic enough.
	    if (!result) {
	    	// Not enough entropy.
	    	return false;
	    }
	}

    uint32_t seed[IsaacRandomPool::SEEDTERMS];

    // Generate and copy seed from random bytes loaded to seedGenerator.
    seedGenerator.generateSeed();
    seedGenerator.copySeed(seed, IsaacRandomPool::SEEDTERMS);

    // Seed ISSAC generator with a, b, c internal paramters set to 0.
    _isaacrng.srand(0,0,0,seed);

    // Generate BURN random bytes to put ISAAC generator in a stable state.
    for (int i = 0; i < IsaacRandomPool::BURN; ++i) {
        _isaacrng.rand();
    }

    return result;
}
