/** @file fileCrypto.cpp
 *  @brief Definition of the class functions in fileCrypto.h
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
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

// --------------------
// third party includes
// --------------------
#include <sha3.h>
#include <modes.h>
#include <aes.h>
#include <filters.h>
#include <hex.h>
#include <gcm.h>

// ----------------
// library includes
// ----------------
#include "fileCryptopp.h"

// -----------
// Constructor
// -----------

/**
 * @brief Creates FileCryptopp object and associates it with a file name.
 *
 * @param filename const reference to a string with the file name.
 */
FileCryptopp::FileCryptopp(const std::string& filename): _filename(filename) {

}

// ----------
// fileExists
// ----------

/**
 * @brief Checks if file associated with the calling object exists.
 *
 * @return true, if file found on file system.
 */
bool FileCryptopp::fileExists() {

    // Create file stream for file, _filename.
    std::ifstream fileStream(_filename);

    // Check if file was sucessfully opened.
    if (fileStream.is_open()) {
        // Close stream and return.
        fileStream.close();
        return true;
    }

	return false;
}

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
bool FileCryptopp::readFile(
    std::stringstream& ss,
    const std::vector<uint8_t>& key
) {

    // Start a file stream.
    std::ifstream fileStream(_filename, std::ios::binary);
    fileStream.unsetf(std::ios::skipws);

    // Check if file stream is open.
    if (!fileStream.is_open()) {
        return false;
    }

    // Check if decryption key is provided.
    if (key.empty()) {
        // Copy file stream as is to string stream ss.
        std::copy(
            std::istreambuf_iterator<char>(fileStream),
            std::istreambuf_iterator<char>(),
            std::ostreambuf_iterator<char>(ss)
        );
        return true;
    }

    // Check if decryption key has length equal to default AES key length.
    if (key.size() != FileCryptopp::AESNODE_DEFAULT_KEY_LENGTH_BYTES) {
        // Invalid key.
        std::cout << "Invalid key length";
        return false;
    }

    // Get size of file.
    fileStream.seekg(0, std::ios::end);
    size_t fileSize = fileStream.tellg();
    fileStream.seekg(0, std::ios::beg);

    std::vector<uint8_t> encryptedBytes;
    encryptedBytes.reserve(fileSize);

    std::copy(
        std::istream_iterator<uint8_t>(fileStream),
        std::istream_iterator<uint8_t>(),
        std::back_inserter(encryptedBytes)
    );

    std::vector<uint8_t> messageData; // vector to hold plain text as bytes

    // Attempt to decrypt and load decrypted bytes into cipherData.
	if (!decrypt(encryptedBytes, messageData, key)) {
        // Failed decryption.
        return false;
    }

    // Loop through cipherData bytes and convert to chars for ss.
    for (auto it = messageData.begin(); it != messageData.end(); ++it) {
        // Load char bytes into string stream ss.
        ss << static_cast<char>(*it);
    }

    // Close file stream.
    fileStream.close();

	return true;
}

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
bool FileCryptopp::writeFile(
    const std::stringstream& ss,
    const std::vector<uint8_t>& key
) {

    // Start a file stream.
    std::ofstream fileStream(_filename, std::ios::binary);

    // Check if file stream is open.
    if (!fileStream.is_open()) {
        return false;
    }

    // Initialize an ostream_iterator to write to the file stream.
    std::ostream_iterator<uint8_t> oit(fileStream);

    // Check if encyption key is provided.
    if (!key.empty()) {
        // Check if encryption key has length equal to default AES key length.
        if (key.size() != FileCryptopp::AESNODE_DEFAULT_KEY_LENGTH_BYTES) {
            // Invalid key.
            std::cout << "Invalid key length";
            return false;
        }

        // Vector to store encrypted bytes of data.
        std::vector<uint8_t> cipherData;

        // Attempt to encrypt input stream ss and load bytes into cipherData.
        if (!encrypt(ss, cipherData, key)) {
            // Failed encryption.
            return false;
        }
        // Write encrypted bytes from cipherData to file as chars.
        std::copy(cipherData.begin(), cipherData.end(), oit);
    } else {
        // Write to file without encryption.
        fileStream << ss.str();
    }

    fileStream.close();
	return true;
}

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
bool FileCryptopp::encrypt(
    const std::stringstream& message,
    std::vector<uint8_t>& cipherData,
    const std::vector<uint8_t>& key
) {

    // Initial Vector (IV) for AES to XOR.
    std::vector<uint8_t> iv(CryptoPP::AES::BLOCKSIZE);

    std::string plaintext = message.str(); // store message
    std::string ciphertext; // store encrypted message


    try {
        // Initialize AES.
        CryptoPP::GCM<AES>::Encryption e;

        // Set AES Key and load IV.
        e.SetKeyWithIV(key.data(), key.size(), iv.data());

        /* Load plain text into string source, then encrypt stream using
         * a transformation filter. Dump the transformation using a
         * string sink into ciphertext.
         */
        StringSource ss1(plaintext,
            true,
            new CryptoPP::AuthenticatedEncryptionFilter(
                e,
                new StringSink(ciphertext)
            ) // StreamTransformationFilter
        ); // StringSource

    } catch (const CryptoPP::Exception& e) {
        // Failed Encryption.
        std::cerr << "Error encrypting file: " << e.what() << std::endl;
        return false;
    }

    // Append cipherData with bytes from ciphertext.
    for (int i = 0; i < ciphertext.size(); i++) {
        cipherData.push_back(
            0xFF & static_cast<uint8_t>(ciphertext[i])
        );
    }

    return true;
}

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
bool FileCryptopp::decrypt(
    const std::vector<uint8_t>& cipher,
    std::vector<uint8_t>& messageData,
    const std::vector<uint8_t>& key
) {

    // Initial Vector (IV) for AES to XOR.
    std::vector<uint8_t> iv(CryptoPP::AES::BLOCKSIZE);

    std::string decryptedtext; // store decrypted message


    try {
        // Initialize AES.
        CryptoPP::GCM< AES >::Decryption e;

        // Set AES Key and load IV.
        e.SetKeyWithIV(key.data(), key.size(), iv.data());

        /* Load encrypted data into an array source, then decrypt data using
         * a transformation filter. Dump the transformation using a string
         * sink into decryptedtext.
         */
        ArraySource ss2(
            cipher.data(),
            cipher.size(),
            true,
            new CryptoPP::AuthenticatedDecryptionFilter(
                e,
                new StringSink(decryptedtext)
            ) // StreamTransformationFilter
        ); // StringSource

    } catch (const CryptoPP::Exception& e) {
        // Failed decryption.
        std::cerr << "Error decrypting file: " << e.what() << std::endl;
        return false;
    }

    // Append messageData with bytes from decryptedtext.
    for (int i = 0; i < decryptedtext.size(); i++) {
        messageData.push_back(
            0xFF & static_cast<uint8_t>(decryptedtext[i])
        );
    }

    return true;
}

