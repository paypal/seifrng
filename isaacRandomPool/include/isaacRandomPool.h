/** @file isaacRandomPool.h
 *  @brief Class header tasked with generating random bytes with evenly
 *         distributed entropy over bits.
 *         Generates random bytes as returned by a hash over random
 *         integers from an ISAAC generator.
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

#ifndef CRYPTOPP_ISAACRNG_H
#define CRYPTOPP_ISAACRNG_H

// -----------------
// standard includes
// -----------------
#include <iterator>

// --------------------
// third party includes
// --------------------
#include <randpool.h>
#include <rng.h>

// ----------------
// library includes
// ----------------
#include "isaac.hpp"

/**
 * @class IsaacRandomPool tasked with generating random bytes with evenly
 *        distributed entropy over bits.
 *        Inherits the RandomNumberGenerator from crypto++.
 */
class CRYPTOPP_DLL IsaacRandomPool : public RandomNumberGenerator
{
public:
	// ---------
	// Constants
	// ---------

	// Number of frames to capture from camera.
	static const size_t NUM_CAPTURE_FRAMES = 15;

	// Number of bytes from OS rng.
	static const size_t NUM_OS_RANDOM_BYTES = 1024*1024*25;

	// Sleep time in milliseconds for a microphone device to capture audio.
	static const size_t NUM_MIC_SLEEP_MS = 1*1000;

	// Seed for ISAAC generator (256 int32 terms).
	static const size_t SEEDTERMS = 256;

	// Alpha for ISAAC generator  (2^8 = 256)
	static const size_t ALPHA = 8;

	// Number of 512bit hashes required to generate seed terms.
	static const size_t ENTROPYSPLIT = 16;

	// Number of random bytes to burn.
	static const size_t BURN = 512;

	// ------
	// STATUS
	// ------

	// Status enum for different types of errors.
	enum class STATUS:int {
		SUCCESS = 0, 			// Success
		FILE_NOT_FOUND = -1, 	// RNG state file not found.
		DECRYPTION_ERROR = -2, 	// Error Decrypting RNG state file.
		ENTROPY_ERROR = -3,		// Error gathering entropy.
		RNG_INIT_ERROR = -4		// RNG not initialized.
	};

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
	 * @return void
	 */
	void GenerateBlock(byte *output, size_t size);

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
	STATUS IsInitialized(
		const std::string& file,
		std::vector<uint8_t> key = std::vector<uint8_t>()
	);

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
	 * @return true, if rng was seeded and initialized successfully.
	 */
	bool Initialize(
		const std::string& file,
		size_t multiplier = 0,
		std::vector<uint8_t> key = std::vector<uint8_t>()
	);

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
	void InitializeEncryption(const std::vector<uint8_t>& key);

	// -------
	// Destroy
	// -------

	/**
	 * @brief Saves current state and resets internal ISAAC generator to an
	 *        uninitialized state.
	 *
	 * @return void
	 */
	void Destroy();

private:
	// ------------
	// int32toBytes
	// ------------

	/**
	 * @brief Converts an int32 stream to a byte stream.
	 *
	 * @param begin input iterator pointing to the beginning of the int32
	 *        stream.
	 * @param end input iterator pointing to the end of the int32 stream.
	 * @param out output iterator to record bytes from the int32 stream.
	 *
	 * @return void
	 */
	template <typename II, typename OI>
	void int32toBytes(II begin, II end, OI out);

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
	 * @return true, if entropy minning was successfull (causally seeding).
	 */
	bool GatherEntropyAndSeed(int multiplier);

	// ----
	// data
	// ----

	QTIsaac<IsaacRandomPool::ALPHA, uint32_t> _isaacrng;

};

// ------------
// int32toBytes
// ------------

/**
 * @brief Converts an int32 stream to a byte stream.
 *
 * @param begin input iterator pointing to the beginning of the int32
 *        stream.
 * @param end input iterator pointing to the end of the int32 stream.
 * @param out output iterator to record bytes from the int32 stream.
 *
 * @return void
 */
template <typename II, typename OI>
void IsaacRandomPool::int32toBytes(II begin, II end, OI out){
	// Loop through int32 stream.
	while (begin!=end){
		// Extract four bytes from int32.
		for (int i = 0; i<4; ++i) {
			*out = static_cast<uint8_t>(
				(*begin & (0x000000FF << (8 * i))) >> (8 * i)
			);
			++out;
		}
		++begin;
	}
}


#endif
