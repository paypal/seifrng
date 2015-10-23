/** @file randomSource.h
 *  @brief Abstract class to be implemented by random sources to interface
 *         with seed generator.
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

#ifndef RANDOMSOURCE_H
#define RANDOMSOURCE_H

// -----------------
// standard includes
// -----------------
#include <vector>
#include <cmath>
#include <cstdint>

// ------------
// RandomSource
// ------------

/*
 * @class Abstract class required to be implemented by random sources for
 *        entropy gathering.
 */
class RandomSource {
public:

	/**
	 * @brief A pure virtual function to be implemented in a way to load
	 *        entropic data from the random source.
	 *
      * @param entropicData reference to a byte vector to be populated
      *        with entropic data from the random source.
	 */
	virtual void appendData(std::vector<uint8_t>& entropicData) = 0;

	/**
	 * @brief A pure virtual function to be implemented in a way to load
	 *        bitwise entropy estimate of data from the random source.
      *
	 * @return double vector with size equal to sample size of the
	 *         random source. The vector holds bit occurrence probabilities.
	 */
	virtual std::vector<double> bitEntropy() = 0;
};

#endif