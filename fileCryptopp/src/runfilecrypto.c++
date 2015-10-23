/** @file runfilecrypto.c++
 *  @brief Test public functions from fileCrypto.h
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
#include <sstream>
#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>

// ----------------
// library includes
// ----------------
#include "fileCryptopp.h"

// ---------
// writeFile
// ---------

/**
 * @brief Attempt to encrypt and write to filesystem.
 *
 * @return true, if test passed.
 */
int writeFile () {
	std::cerr << "**Running test writeFile**" << std::endl;
	// Generate encryption key.
	std::vector<uint8_t> key;
	key.reserve(32);
	for (int i = 0; i < 32; ++i) {
		key.push_back(i);
	}

	// Initialize.
	FileCryptopp fc("./test");

	std::stringstream ss;

	std::vector<char> clearText;
	clearText.reserve(26);

	for (int i = 0; i < 26; ++i) {
		clearText.push_back(i + 'a');
	}

	// Copy clear text into ss.
	std::copy(
		clearText.begin(),
		clearText.end(),
		std::ostream_iterator<char> (ss)
	);

	// Write ss encrypted with key to file.
	bool retVal = fc.writeFile(ss, key);

	if (!retVal) {
		std::cerr << "!!Failed writeFile test!!" << std::endl;
	} else {
		std::cerr << "--Passed--" << std::endl;
	}

	return retVal;
}

// --------
// readFile
// --------

/**
 * @brief Attempt to read from the filesystem and decrypt data.
 *
 * @return true, if test passed.
 */
int readFile () {
	std::cerr << "**Running test readFile**" << std::endl;
	// Generate decryption key.
	std::vector<uint8_t> key;
	key.reserve(32);
	for (int i = 0; i < 32; ++i) {
		key.push_back(i);
	}

	// Initialize.
	FileCryptopp fc("./test");

	std::vector<char> expectedClearText;
	expectedClearText.reserve(26);

	for (int i = 0; i < 26; ++i) {
		expectedClearText.push_back(i + 'a');
	}

	// Create stream to save clear text.
	std::stringstream ss;

	fc.readFile(ss, key);

	// Compare clear text with expected clear text.
	bool retVal = std::lexicographical_compare(
		expectedClearText.begin(),
		expectedClearText.end(),
		std::istream_iterator<char> (ss),
		std::istream_iterator<char> (),
		[] (char c1, char c2) {
			return (c1 == c2);
		}
	);

	if (!retVal) {
		std::cerr << "!!Failed readFile test!!" << std::endl;
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
	passed += writeFile();
	passed += readFile();

	std::cerr << std::endl;
	std::cerr << "--Passed " << passed << "/2" << " tests--" << std::endl;

	// Assert passing all tests.
	assert(passed == 2);
	return 0;
}