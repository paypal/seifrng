/** @file runisaacrandompool.c++
 *  @brief Test public functions from isaacRandomPool.h
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

// ----------------
// library includes
// ----------------
#include "isaacRandomPool.h"

// ----------------
// runUnInitialized
// ----------------

/**
 * @brief Attempt generating bytes in an uninitialized state.
 *
 * @return true, if test passed.
 */
int runUnInitialized() {
	std::cerr << "**Running test runUnInitialized**" << std::endl;
	std::vector<uint8_t> output(32,0);
	IsaacRandomPool g_PRNG;
	size_t sum;

	try {
		// Attempt to generate before initialization.
		g_PRNG.GenerateBlock(output.data(), 32);
	} catch (std::runtime_error& e) {
		std::cerr << "Caught exception: " << e.what() << std::endl;
		std::cerr << "--Passed--" << std::endl;
		return true;
	}

	std::cerr << "!!Failed initializeRNG test!!" << std::endl;

    return false;
}

// -------------
// initializeRNG
// -------------

/**
 * @brief Attempt to initialize rng; force generation of a seed.
 *
 * @return true, if test passed.
 */
int initializeRNG() {
	std::cerr << "**Running test initializeRNG**" << std::endl;
	std::string file(".test");
	std::vector<uint8_t> output(32,0);
	IsaacRandomPool g_PRNG;
	size_t sum;

	// Initialize RNG by generating a new seed and save state to file.
	try {
		if (g_PRNG.Initialize(file)) {
			std::cerr << "--Passed--" << std::endl;
			return true;
		} else {
			return false;
		}
	} catch (std::runtime_error& e) {
		std::cerr << "Caught exception: " << e.what() << std::endl;
		std::cerr << "!!Failed initializeRNG test!!" << std::endl;
		return false;
	}
}

// -------------
// loadRNGNoFile
// -------------

/**
 * @brief Attempt to initialize rng from a non-existent state.
 *
 * @return true, if test passed.
 */
int loadRNGNoFile() {
	std::cerr << "**Running test loadRNGNoFile**" << std::endl;
	std::string dummyFile("dummy_file_name");
	IsaacRandomPool g_PRNG;

	// attempt loading state from an non-existent file
	bool testVal = (
		g_PRNG.IsInitialized(dummyFile)
		== IsaacRandomPool::STATUS::FILE_NOT_FOUND
	);

	if (!testVal) {
		std::cerr << "!!Failed loadRNGNoFile test!!" << std::endl;
	} else {
		std::cerr << "--Passed--" << std::endl;
	}

	return testVal;
}

// ----------------
// loadRNGFromState
// ----------------

/**
 * @brief Attempt to initialize rng from a valid saved state.
 *
 * @return true, if test passed.
 */
int loadRNGFromState() {
	std::cerr << "**Running test loadRNGFromState**" << std::endl;
	std::string file(".test");
	std::vector<uint8_t> output(32,0);
 	IsaacRandomPool g_PRNG;

 	// Attempt loading state from a valid file with state data.
	bool testVal = (
		g_PRNG.IsInitialized(file)
		== IsaacRandomPool::STATUS::SUCCESS
	);

	if (!testVal) {
		std::cerr << "!!Failed loadRNGFromState test!!" << std::endl;
	} else {
		std::cerr << "--Passed--" << std::endl;
	}

	return testVal;
}

// -------------
// saveEncrypted
// -------------

/**
 * @brief Write encrypted state to file.
 *
 * @return void
 */
void saveEncrypted() {
	std::string file(".test");
	std::vector<uint8_t> output(32,0);
	std::vector<uint8_t> key(32,1);
	IsaacRandomPool g_PRNG;

	g_PRNG.IsInitialized(file);

	// Add encryption key to save encrypted state.
	g_PRNG.InitializeEncryption(key);
}

// ----------------
// loadRNGEncrypted
// ----------------

/**
 * @brief Attempt to initialize rng from a valid saved (encrypted) state.
 *
 * @return true, if test passed.
 */
int loadRNGEncrypted() {
	std::cerr << "**Running test loadRNGEncrypted**" << std::endl;
	std::string file(".test");
	std::vector<uint8_t> output(32,0);
	std::vector<uint8_t> key(32,1);
	IsaacRandomPool g_PRNG;

	// Attempt to load state from an encrypted file with a valid key
	bool testVal = (
		g_PRNG.IsInitialized(file, key)
		== IsaacRandomPool::STATUS::SUCCESS
	);

	if (!testVal) {
		std::cerr << "!!Failed loadRNGEncrypted test!!" << std::endl;
	} else {
		std::cerr << "--Passed--" << std::endl;
	}

	return testVal;
}


// ---------------
// loadRNGWrongKey
// ---------------

/**
 * @brief Attempt to initialize rng with a wrong decryption key
 *
 * @return true, if test passed.
 */
int loadRNGWrongKey() {
	std::cerr << "**Running test loadRNGWrongKey**" << std::endl;
	IsaacRandomPool g_PRNG;
	std::string file(".test");
	std::vector<uint8_t> key(32,2);

	// Attempt to load state from an encrypted file with an invalid key.
	bool testVal = (
		g_PRNG.IsInitialized(file, key)
		== IsaacRandomPool::STATUS::DECRYPTION_ERROR
	);

	if (!testVal) {
		std::cerr << "!!Failed loadRNGWrongKey test!!" << std::endl;
	} else {
		std::cerr << "--Passed--" << std::endl;
	}

	return testVal;
}

int main(){
	IsaacRandomPool g_PRNG;

	/* Run tests and count passed.
	 * Order matters.
	 */
	int passed = 0;
	passed += runUnInitialized();
	passed += initializeRNG();
	passed += loadRNGNoFile();
	passed += loadRNGFromState();
	saveEncrypted();
	passed += loadRNGEncrypted();
	passed += loadRNGWrongKey();

	std::cerr << std::endl;
	std::cerr << "--Passed " << passed << "/6" << " tests--" << std::endl;
	std::cerr << "Entropy strength: " << g_PRNG.EntropyStrength() << std::endl;

	// Assert passing all tests.
	assert(passed == 6);
	return 0;
}
