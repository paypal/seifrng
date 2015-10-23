seifrng
=======
A library tasked to enable the following functionality:
1. Mine entropy from random sources to generate a truly random seed. 
2. Generate bytes from a Cryptographically Secure Pseudo Random Number Generator (CPRNG).  
3. Securely encrypt/decrypt data to the file system with authentication.


Installation
============
###Linux and OSX
The library uses the cmake (https://cmake.org) build system. Install cmake before proceeding.

1. Install dependencies 

    — Cryptopp/Crypto++: https://www.cryptopp.com/ 

    — OpenCV: http://opencv.org/ 

    — PortAudio: http://www.portaudio.com/ 
2. Run mkdir build & cd build
3. Run cmake ../
4. Run make and make install
5. Additionally run make test if you would like to test the installation.

Description and Usage
=====================

###Mining Entropy and Generating a Seed

Entropy is mined from three sources 1) Microphone 2) Camera 3) Operating System (OS).
Each implements functions appendData and bitEntropy from RandomSource.h.

appendData: Retrieves random bytes from the source.
bitEntropy: Provides an entropy estimate per bit of the collection of random samples.

**Generating a Seed**
The static library *libseedGenerator* enables generation of a seed from an entropy pool. The following functionality enables populating the entropy pool and seed generation.

**processFromSource**: Interacts with functions *appendData* and *bitEntropy* to populate entropy pool. Entropy pool is populated only if the bit entropy estimate meets a threshold of 0.25 bit occurrence probability over the contributing set of samples.

**generateSeed**: Computes SHA3-512 hashes on the entropy pool to populate a seed.

**copySeed**: Copies seed bytes into a an array of seed terms. Seed terms can be ints of any size.

**resetState**: Resets the entropy pool to generate a new seed.


Accumulating entropy can vary for each source. We describe functions which enable this for the implemented sources.

**1) Microphone** - The static library libmicrophone implements functions *initFlow* and *stopFlow* to enable asynchronous capture of audio samples from an available microphone. The library is complemented by PortAudio (http://www.portaudio.com/) to enable device independent interaction with a microphone. 

**2) Camera** — The static library libcamera implements *captureFrames* to enable capture of frames from an available camera. The library is complemented by OpenCV (http://opencv.org/) to enable device independent interaction with a camera.

**3) OS** — The static library lobosrng implements *generateRandomBytes* to tap into the OS random number generator. The library is complemented by Crypto++ (https://www.cryptopp.com/) to enable device independent access to random numbers from the OS, even if an hardware source is available within the processor architecture.

**Example Usage**
```c++
    // Set up microphone to record samples.
    InterfaceMicrophone interfaceMicrophone;

    // Start recording and collecting data (async).
    status = interfaceMicrophone.initFlow();

    if(!status) {
    	throw std::runtime_error("Cannot open microphone device.");
    }

    /* Setup SeedGenerator to generate a seed of length SEEDTERMS.
     * Parameter to the constructor determines number of splits to data
     * collected to generate independent hashes to fill SEEDTERMS i.e.
     * 2 splits => 2 hashes of 512 bytes => 32, 32bit terms or 64, 16bit terms
     * and so on.
     */
    SeedGenerator seedGenerator(16);

    // Set up camera to record samples.
    InterfaceCamera interfaceCamera;

    // Set up OS rng to record samples.
    InterfaceOSRNG interfaceOSRNG;

    // Capture samples from camera, increase num samples as an exponent of 2
    // determined by multiplier.
    status = interfaceCamera.captureFrames(5);

	if(!status) {
    	throw std::runtime_error("Cannot open camera device.");
    }

    // Capture bytes from os randomness, increase num samples as an exponent
    // of 2 determined by multiplier.
    status = interfaceOSRNG.generateRandomBytes(1000);

    if(!status) {
    	throw std::runtime_error("Cannot tap OS entropy.");
    }

    // Sleep (ms) to gather more samples from microphone.
    Pa_Sleep(1000);

    // Stop listening on the microphone.
    interfaceMicrophone.stopFlow();

    /* Load data from sources as random bytes to seedGenerator.
     * A false result indicates the source failed to gather sufficient entropy.
     */
    bool result = seedGenerator.processFromSource(&interfaceOSRNG);
    result = result && seedGenerator.processFromSource(&interfaceCamera);
    result = result && seedGenerator.processFromSource(&interfaceMicrophone);

    // Check if data is entropic enough.
    if (!result) {
    	// Not enough entropy.
    	return false;
    }

    uint32_t seed[256];

    // Generate and copy seed from random bytes loaded to seedGenerator.
    seedGenerator.generateSeed();
    seedGenerator.copySeed(seed, 256);
```

###Generating Random Bytes

The objective of the dynamic library *libisaacRandomPool* is to generate cryptographically safe random bytes. To that end the library builds on a c++ implementation of ISAAC (http://burtleburtle.net/bob/rand/isaacafa.html).

The random number generator inherits RandomNumberGenerator from Crypto++ (https://www.cryptopp.com/) implementing GenerateBlock to enable interaction with cryptographic functions.

We describe public functions which facilitate managing the CPRNG and generating random bytes.

**Generating Random Bytes**


**GenerateBlock**: Generates a block of random bytes from an initialized generator. Blocks are composites of SHA3-256 hashes computed on random bytes generated from ISAAC. Hashing is performed to evenly distribute entropy over a sample.  

**Managing the CPRNG**


**Initialize**: Mines entropy to generate a seed and initializes a ISAAC generator. If an encryption key is available, the ISAAC generator is updated to encrypt state before saving to the file system.

**IsInitialized**: Checks if a previously initialized ISAAC generator is available to load. Loading is enabled from saved state on a file. A key can be provided if the state need to be decrypted before loading.

**InitializeEncryption**: The ISAAC generator is updated to encrypt state before saving to the file system.

**Destroy**: The ISAAC generator is triggered to destroy. ISAAC before destroying saves state to the file system. 

**Example Usage**
```c++
    IsaacRandomPool g_PRNG;
    std::string file(“.state”);
    IsaacRandomPool::STATUS status;
    status = g_PRNG.IsInitialized(file)
    if (status == IsaacRandomPool::STATUS::FILE_NOT_FOUND) {
        g_PRNG.Initialize(file);
    } else if (status == IsaacRandomPool::STATUS::SUCCESS) {
        std::vector<uint8_t> output(32,0);
        g_PRNG.GenerateBlock(output.data(), 32);
        std::vector<uint8_t> key(32,1);
        g_PRNG.InitializeEncryption(key);
        g_PRNG.Destroy();
    }
```

###Secure access to the File System

The objective of the static library fileCryptopp is to enable a authenticated and secure encrypted channel to the file system. To that end the library uses AES is GCM mode to encrypt/decrypt data with an encryption key. Encryption functionality is enabled by Crypto++ (https://www.cryptopp.com/). The following functions enable encrypting and writing a stream to a file and decrypting a file stream.

**fileExists** Checks if file exists in the file system.

**readFile** Reads a file from the file system and decrypts the file stream if a key is available.

**writeFile** Writes to a file, encrypts the stream before writing if a key is available.

**Example Usage**
```c++
    // Generate encryption key.
    std::vector<uint8_t> key;
    key.reserve(32);
    for (int i = 0; i < 32; ++i) {
        key.push_back(i);
    }

    // Initialize.
    FileCryptopp fc(“./testFile”);

    std::stringstream ss;

    std::vector<char> clearText;
    clearText.reserve(26);
    
    std::cout << “Clear Text:” << std::endl;
    for (int i = 0; i < 26; ++i) {
        clearText.push_back(i + 'a');
        std::cout<<i + ‘a’;
    }
    std::cout << std::endl;

    // Copy clear text into ss.
    std::copy(
        clearText.begin(),
        clearText.end(),
        std::ostream_iterator<char> (ss)
    );

    // Write ss encrypted with key to file.
    fc.writeFile(ss, key);

    ss.clear();    

    // Read decrypted data into ss
    if (fc.readFile(ss, key)) {
        std::cout << ss.str() << std::endl;
    }

```

Dependencies
============

###1. Cryptopp/crypto++ 
https://www.cryptopp.com/

Used for all cryptographic functions. Library installed version 5.6.2 

**License:**
Crypto++ Library is copyrighted as a compilation and (as of version 5.6.2) licensed under the Boost Software License 1.0, while the individual files in the compilation are all public domain.
https://www.cryptopp.com/License.txt


###2. OpenCV
http://opencv.org/

Portable solution to access the device camera. This is a required library dependency.

**License:**
OpenCV is released under a BSD license and hence it’s free for both academic and commercial use.

###3. PortAudio
http://www.portaudio.com/

Portable solution to access device audio I/O. This is a required library dependency.

**License:**
We can use PortAudio for free in our projects or applications, even commercial applications.
This license is compatible with the GNU General Public License. In other words, PortAudio can be included in a GNU project without violating the GNU license. In terms of legal compatibility, the PortAudio licence is now a plain MIT licence
http://www.portaudio.com/license.html 

###4. Isaac
http://burtleburtle.net/bob/rand/isaacafa.html

Fast cryptopgraphic random number generator
We are using the C++ implementation with some modifications to access the RNG state.

**License:** 
Public Domain


License
=======

The MIT License (MIT)

Copyright (c) 2015 PayPal

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.