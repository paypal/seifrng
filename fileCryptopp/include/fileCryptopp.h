/** @file fileCrypto.h
 *  @brief Class header tasked with encryption/decryption of data to be written
 *         or read from the filesystem. File encryption and decryption using
 *         AES-GCM is enabled from crypto++ to assure confidentiality and
 *         authenticity.
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

#ifndef FILECRYPTOPP_H
#define FILECRYPTOPP_H

// -----------------
// standard includes
// -----------------
#include <iterator>
#include <sstream>

// --------------------
// third party includes
// --------------------
#include <aes.h>


using namespace CryptoPP;

/**
 * @class FileCryptopp tasked with encryption/decryption of data to be written
 *        or read from the filesystem.
 */
class FileCryptopp {
public:

	// ---------
	// constants
	// ---------
	static const uint8_t AESNODE_DEFAULT_KEY_LENGTH_BYTES = 32;

	// -----------
	// Constructor
	// -----------

	/**
	 * @brief Creates FileCryptopp object and associates it with a file name.
	 *
	 * @param filename const reference to a string with the file name.
	 */
	FileCryptopp(const std::string& filename);

	// ----------
	// fileExists
	// ----------

	/**
	 * @brief Checks if file associated with the calling object exists.
	 *
	 * @return true, if file found on file system.
	 */
	bool fileExists();

	// --------
	// readFile
	// --------

	/**
	 * @brief Reads file associated with the calling object. If a valid
	 *        decryption key is available, data is decrypted (AES-GCM).
	 *
	 * @param ss stringstream taken by reference to be appended with data.
	 * @param key const reference to a byte vector with the decryption key.
	 *
	 * @return true, if file was read sucessfully.
	 */
	bool readFile(std::stringstream& ss, const std::vector<uint8_t>& key);

	// ---------
	// writeFile
	// ---------

	/**
	 * @brief Writes to file associated with the calling object. If a valid key
	 *        is available, data is encrypted (AES-GCM) prior to writing.
	 *
	 * @param ss const reference to a string stream with data.
	 * @param key const reference to a byte vector with the encryption key.
	 *
	 * @return true, if data was sucessfully written to file.
	 */
	bool writeFile(
		const std::stringstream& ss,
		const std::vector<uint8_t>& key
	);

private:

	// -------
	// encrypt
	// -------

	/**
	 * @brief Encrypts input string stream using AES in GCM mode for
	 *        confidentiality and authenticity.
	 *
	 * @param message const reference to a string stream with data.
	 * @param cipherData reference to a byte vector to hold encrypted data.
	 * @param key const reference to a byte vector with an encryption key.
	 *
	 * @return true, if encryption is successful.
	 */
	bool encrypt(
		const std::stringstream& message,
		std::vector<uint8_t>& cipherData,
		const std::vector<uint8_t>& key
	);

	// -------
	// decrypt
	// -------

	/**
	 * @brief Decrypts input string stream using AES in GCM mode for
	 *        confidentiality and authenticity.
	 *
	 * @param cipher const reference to a byte vector with encrypted data.
	 * @param messageData reference to a byte vector to hold decrypted data.
	 * @param key const reference to a byte vector with the decryption key.
	 *
	 * @return true, if decryption is successful.
	 */
	bool decrypt(
		const std::vector<uint8_t>& cipher,
		std::vector<uint8_t>& messageData,
		const std::vector<uint8_t>& key
	);
	// ----
	// data
	// ----
	std::string _filename;

};

#endif
