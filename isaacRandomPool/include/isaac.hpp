/** @file isaac.hpp
 *  @brief C++ TEMPLATE VERSION OF Robert J. Jenkins Jr.'s
 *         ISAAC Random Number Generator.
 *
 *         Ported from vanilla C to to template C++ class
 *         by Quinn Tyler Jackson on 16-23 July 1998.
 *
 *         quinn@qtj.net
 *
 *         The function for the expected period of this
 *         random number generator, according to Jenkins is:
 *
 *         f(a,b) = 2**((a+b*(3+2^^a)-1)
 *
 *         (where a is ALPHA and b is bitwidth)
 *
 *         So, for a bitwidth of 32 and an ALPHA of 8,
 *         the expected period of ISAAC is:
 *
 *         2^^(8+32*(3+2^^8)-1) = 2^^8295
 *
 *         Jackson has been able to run implementations
 *         with an ALPHA as high as 16, or
 *
 *         2^^2097263
 *         ---------------------------------------------------
 *         [UPDATE] SEPTEMBER-OCTOBER 2015
 *         Modified by Aashish Sheshadri and Rohit Harchandani
 *         ---------------------------------------------------
 *         Functionality added with the purpose of saving and
 *         loading state with encryption if desired.
 *
 *         #Modified tag indicates sections modified over
 *         existing implementation
 *
 *         Interfaces with FileCryptopp lib to interact with
 *         file system and enable encryption.
 *
 *  @author Quinn Tyler Jackson
 *  @author Aashish Sheshadri
 *  @author Rohit Harchandani
 */

#ifndef __ISAAC_HPP
#define __ISAAC_HPP

// -----------------
// standard includes
// -----------------
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <iterator>

// ----------------
// library includes
// ----------------
#include "fileCryptopp.h"

#ifndef __ISAAC64
   typedef unsigned long int UINT32;
   const UINT32 GOLDEN_RATIO = UINT32(0x9e3779b9);
   typedef UINT32 ISAAC_INT;
#else   // __ISAAC64
typedef unsigned __int64 UINT64;
const UINT64 GOLDEN_RATIO = UINT64(0x9e3779b97f4a7c13);
typedef UINT64 ISAAC_INT;
#endif  // __ISAAC64

void getValidFile(std::string& validFileName, const std::string& file);

template <int ALPHA = (8), class T = ISAAC_INT>
   class QTIsaac
   {
   public:

      typedef unsigned char byte;

      struct randctx
      {
         randctx(void)
         {
            randrsl = static_cast<T*>(std::malloc(N * sizeof(T)));
            randmem = static_cast<T*>(std::malloc(N * sizeof(T)));
         }

         ~randctx(void)
         {
            ::free((void*)randrsl);
            ::free((void*)randmem);
         }

         T randcnt;
         T* randrsl;
         T* randmem;
         T randa;
         T randb;
         T randc;
      };
      enum {N = (1<<ALPHA)};

      // -----------
      // Constructor
      // -----------

      /**
       * Constructor
       * @brief Creates QTIsaac object an instance of ISAAC generator.
       *        Modified to reset state and not seed.
       */
      QTIsaac();

      // -----------
      // Destructor
      // -----------

      /**
       * Destructor
       * @brief Destroys ISAAC generator. Modified to save state if initialized.
       */
      virtual ~QTIsaac(void);

      T rand(void);
      virtual void srand(T a = 0, T b = 0, T c = 0, T* s = NULL);

      virtual void randinit(randctx* ctx, bool bUseSeed);

      // New functionality added with the purpose of saving and loading state.

      // -------------
      // setIdentifier
      // -------------

      /**
       * @brief Sets filename (with path) to save or load state from, file name
       *        is truncated at 32 bytes.
       *
       * @param file const reference to a string with file path and name.
       */
      void setIdentifier(const std::string& file);

      // ------
      // setKey
      // ------

      /**
       * @brief Sets encryption/decryption key as required by FileCrypto.
       *
       * @param key const reference to a vector of uint8_t with key bytes.
       *
       * @return void
       */
      void setKey(const std::vector<uint8_t>& key);

      // ----------
      // initialize
      // ----------

      /**
       * @brief Initializes internal state from file.
       *
       * @param file const reference to a string with file path and name.
       * @param key const reference to a vector of uint8_t with key bytes.
       *
       * @return int with value -2 if decryption failed, -1 if file was not
       *         found and 0 if state was read sucessfully.
       */
      int initialize(const std::string& file, std::vector<uint8_t> key);

      // ---------
      // saveState
      // ---------

      /**
       * @brief Encrypts and saves the current RNG state to disk.
       *
       * @return bool true, if saving the state is successful, false if not
       */
      bool saveState();

      // -------
      // destroy
      // -------

      /**
       * @brief Saves current state and resets internal state to prepare for
       *        reseeding or resumption from an old state.
       *
       * @return void
       */
      void destroy();

      // -----------
      // initialized
      // -----------

      /**
       * @brief Checks if state exists in memory.
       * @return true, if state exists in memory.
       */
      inline bool initialized() { return _initialized; }

   protected:

      virtual void isaac(randctx* ctx);

      T ind(T* mm, T x);
      void rngstep(T mix, T& a, T& b, T*& mm, T*& m, T*& m2, T*& r, T& x, T& y);
      virtual void shuffle(T& a, T& b, T& c, T& d, T& e, T& f, T& g, T& h);

   private:

      // New functionality added to support saving and loading state from file.

      // ---------------
      // saveStateToFile
      // ---------------

      /**
       * @brief Saves state to file. If a valid encryption key (AES-GCM) is
       *        set, encryption is initialized before writing to file.
       * @return true, if file was written to sucessfully.
       */
      bool saveStateToFile();

      // -----------------
      // loadStateFromFile
      // -----------------

      /**
       * @brief Reads state from file; decryts if a valid key is available.
       *
       * @param file const reference to a string with file path and name.
       * @param key const reference to a vector of uint8_t with key bytes.
       *
       * @return int with value -2 if decryption failed, -1 if file was
       *         not found and 0 if state was read sucessfully.
       */
      int loadStateFromFile(const std::string& file, std::vector<uint8_t> key);

      // ------------
      // getValidFile
      // ------------

      /**
       * @brief Changes the given file name to a valid file name. Also truncates
       *        the file name to 32 bytes to avoid errors.
       *
       * @param validFileName string reference to store valid file name.
       * @param file const reference to a file name.
       *
       * @return void
       */
      void getValidFile(std::string& validFileName, const std::string& file);


      // ----
      // data
      // ----

      randctx m_rc;

      // Variables added to enable state management.
      std::string _stateFileName;
      std::vector<uint8_t> _key;
      bool _initialized;
   };

// -----------
// Constructor
// -----------

/**
 * Constructor
 * @brief Creates QTIsaac object an instance of ISAAC generator.
 *        Modified to reset state and not seed.
 */
template<int ALPHA, class T>
   QTIsaac<ALPHA,T>::QTIsaac():
    _stateFileName("./.isaacrngstate"),
    _initialized(false) {
   }

// -----------
// Destructor
// -----------

/**
 * Destructor
 * @brief Destroys ISAAC generator. Modified to save state if initialized.
 */
template<int ALPHA, class T>
   QTIsaac<ALPHA,T>::~QTIsaac(void)
   {
      // #Modified

      // Save state if initialized.
      if (_initialized) {
        saveStateToFile();
      }

      // #end
   }


template<int ALPHA, class T>
   void QTIsaac<ALPHA,T>::srand(T a, T b, T c, T* s)
   {
      // #Modified

      // Absent seed rng can only resume state read from corresponding file.
      if (s == NULL) {
        // Check if rng is already initialized.
        if (_initialized) {
          return; // Return since state cannot be re-initialized.
        }

        /* Attempt to resume state by reading from corresponding state file
         * (_stateFileName).
         */
        if (loadStateFromFile(_stateFileName, _key) == 0) {
          // State resumption successfull.
          _initialized = true;
          return;
        }
        // Failed to resume state.
        _initialized = false;
        return;
      }

      _initialized = true; // Initialized using seed s.

      // #end

      for (int i = 0; i < N; i++)
      {
         m_rc.randrsl[i] = s != NULL ? s[i] : 0;
      }

      m_rc.randa = a;
      m_rc.randb = b;
      m_rc.randc = c;

      randinit(&m_rc, true);
   }


template<int ALPHA, class T>
   inline T QTIsaac<ALPHA,T>::rand(void)
   {
      // #Modified

      /* If rng has not been seeded or if state hasn't been
       * successfully resumed then return 0's instead of random bytes
       */
      if (!_initialized) {
        return T(0);
      }

      // #end

      return(!m_rc.randcnt--
             ? (isaac(&m_rc), m_rc.randcnt=(N-1), m_rc.randrsl[m_rc.randcnt])
             : m_rc.randrsl[m_rc.randcnt]);
   }


template<int ALPHA, class T>
   void QTIsaac<ALPHA,T>::randinit(randctx* ctx, bool bUseSeed)
   {
      T a,b,c,d,e,f,g,h;

      a = b = c = d = e = f = g = h = GOLDEN_RATIO;

      T* m = (ctx->randmem);
      T* r = (ctx->randrsl);

      if (!bUseSeed)
      {
         ctx->randa = 0;
         ctx->randb = 0;
         ctx->randc = 0;
      }

      // scramble it
      for (int i=0; i < 4; ++i)
      {
         shuffle(a,b,c,d,e,f,g,h);
      }

      if (bUseSeed)
      {
        // initialize using the contents of r[] as the seed

         for (int i=0; i < N; i+=8)
         {
            a+=r[i  ]; b+=r[i+1]; c+=r[i+2]; d+=r[i+3];
            e+=r[i+4]; f+=r[i+5]; g+=r[i+6]; h+=r[i+7];

            shuffle(a,b,c,d,e,f,g,h);

            m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
            m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
         }

        // do a second pass to make all of the seed affect all of m

         for (int i=0; i < N; i += 8)
         {
            a+=m[i  ]; b+=m[i+1]; c+=m[i+2]; d+=m[i+3];
            e+=m[i+4]; f+=m[i+5]; g+=m[i+6]; h+=m[i+7];

            shuffle(a,b,c,d,e,f,g,h);

            m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
            m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
         }
      }
      else
      {
        // fill in mm[] with messy stuff
        for (int i=0; i < N; i += 8)
        {
          shuffle(a,b,c,d,e,f,g,h);

          m[i  ]=a; m[i+1]=b; m[i+2]=c; m[i+3]=d;
          m[i+4]=e; m[i+5]=f; m[i+6]=g; m[i+7]=h;
        }
      }

      isaac(ctx);         // fill in the first set of results
      ctx->randcnt = N;   // prepare to use the first set of results
   }


template<int ALPHA, class T>
   inline T QTIsaac<ALPHA,T>::ind(T* mm, T x)
   {
    #ifndef __ISAAC64
      return (*(T*)((byte*)(mm) + ((x) & ((N-1)<<2))));
    #else   // __ISAAC64
    return (*(T*)((byte*)(mm) + ((x) & ((N-1)<<3))));
    #endif  // __ISAAC64
   }


template<int ALPHA, class T>
   inline void QTIsaac<ALPHA,T>::rngstep(
    T mix,
    T& a,
    T& b,
    T*& mm,
    T*& m,
    T*& m2,
    T*& r,
    T& x,
    T& y) {
      x = *m;
      a = (a^(mix)) + *(m2++);
      *(m++) = y = ind(mm,x) + a + b;
      *(r++) = b = ind(mm,y>>ALPHA) + x;
   }


template<int ALPHA, class T>
   void QTIsaac<ALPHA,T>::shuffle(
    T& a,
    T& b,
    T& c,
    T& d,
    T& e,
    T& f,
    T& g,
    T& h
  ) {
    #ifndef __ISAAC64
      a^=b<<11; d+=a; b+=c;
      b^=c>>2;  e+=b; c+=d;
      c^=d<<8;  f+=c; d+=e;
      d^=e>>16; g+=d; e+=f;
      e^=f<<10; h+=e; f+=g;
      f^=g>>4;  a+=f; g+=h;
      g^=h<<8;  b+=g; h+=a;
      h^=a>>9;  c+=h; a+=b;
    #else // __ISAAC64
      a-=e; f^=h>>9;  h+=a;
      b-=f; g^=a<<9;  a+=b;
      c-=g; h^=b>>23; b+=c;
      d-=h; a^=c<<15; c+=d;
      e-=a; b^=d>>14; d+=e;
      f-=b; c^=e<<20; e+=f;
      g-=c; d^=f>>17; f+=g;
      h-=d; e^=g<<14; g+=h;
    #endif // __ISAAC64
   }


template<int ALPHA, class T>
   void QTIsaac<ALPHA,T>::isaac(randctx* ctx)
   {
      T x,y;

      T* mm = ctx->randmem;
      T* r  = ctx->randrsl;

      T a = (ctx->randa);
      T b = (ctx->randb + (++ctx->randc));

      T* m    = mm;
      T* m2   = (m+(N/2));
      T* mend = m2;

      for (; m<mend; )
      {
        #ifndef __ISAAC64
         rngstep((a<<13), a, b, mm, m, m2, r, x, y);
         rngstep((a>>6) , a, b, mm, m, m2, r, x, y);
         rngstep((a<<2) , a, b, mm, m, m2, r, x, y);
         rngstep((a>>16), a, b, mm, m, m2, r, x, y);
        #else   // __ISAAC64
        rngstep(~(a^(a<<21)), a, b, mm, m, m2, r, x, y);
        rngstep(  a^(a>>5)  , a, b, mm, m, m2, r, x, y);
        rngstep(  a^(a<<12) , a, b, mm, m, m2, r, x, y);
        rngstep(  a^(a>>33) , a, b, mm, m, m2, r, x, y);
        #endif  // __ISAAC64
      }

      m2 = mm;

      for (; m2<mend; )
      {
        #ifndef __ISAAC64
         rngstep((a<<13), a, b, mm, m, m2, r, x, y);
         rngstep((a>>6) , a, b, mm, m, m2, r, x, y);
         rngstep((a<<2) , a, b, mm, m, m2, r, x, y);
         rngstep((a>>16), a, b, mm, m, m2, r, x, y);
        #else   // __ISAAC64
        rngstep(~(a^(a<<21)), a, b, mm, m, m2, r, x, y);
        rngstep(  a^(a>>5)  , a, b, mm, m, m2, r, x, y);
        rngstep(  a^(a<<12) , a, b, mm, m, m2, r, x, y);
        rngstep(  a^(a>>33) , a, b, mm, m, m2, r, x, y);
        #endif  // __ISAAC64
      }

      ctx->randb = b;
      ctx->randa = a;
   }

  // -------------
  // setIdentifier
  // -------------

  /**
   * @brief Sets filename (with path) to save or load state from, file name
   *        is truncated at 32 bytes.
   *
   * @param file const reference to a string with file path and name.
   */
  template<int ALPHA, class T>
  void QTIsaac<ALPHA,T>::setIdentifier(const std::string& pwd) {

    getValidFile(_stateFileName, pwd);
  }

  // ------
  // setKey
  // ------

  /**
   * @brief Sets encryption/decryption key as required by FileCrypto.
   *
   * @param key const reference to a vector of uint8_t with key bytes.
   *
   * @return void
   */
  template<int ALPHA, class T>
  void QTIsaac<ALPHA,T>::setKey(const std::vector<uint8_t>& key) {

    // Clear previous key.
    _key.clear();

    // Copy in new key.
    std::copy(key.begin(), key.end(), std::back_inserter(_key));
  }

  // ----------
  // initialize
  // ----------

  /**
   * @brief Initializes internal state from file.
   *
   * @param file const reference to a string with file path and name.
   * @param key const reference to a vector of uint8_t with key bytes.
   *
   * @return int with value -2 if decryption failed, -1 if file was not
   *         found and 0 if state was read sucessfully.
   */
  template<int ALPHA, class T>
  int QTIsaac<ALPHA,T>::initialize(
    const std::string& file,
    std::vector<uint8_t> key
  ) {

    std::string newStateFileName;
    getValidFile(newStateFileName, file);

    if (newStateFileName == _stateFileName
        && _key == key
        && _initialized == true) {

      return 0;
    }

    return loadStateFromFile(newStateFileName, key);
  }

  // ---------
  // saveState
  // ---------

  /**
   * @brief Encrypts and saves the current RNG state to disk.
   *
   * @return bool true, if saving the state is successful, false if not
   */
  template<int ALPHA, class T>
  bool QTIsaac<ALPHA,T>::saveState() {
    // Check if internal state exists.
    if (_initialized) {
      // Save internal state to file.
      return saveStateToFile();
    }

    return false;
  }

  // -------
  // destroy
  // -------

  /**
   * @brief Saves current state and resets internal state to prepare for
   *        reseeding or resumption from an old state.
   *
   * @return void
   */
  template<int ALPHA, class T>
  void QTIsaac<ALPHA,T>::destroy() {
    // Check if internal state exists.
    if (_initialized) {
      // Save internal state to file.
      saveStateToFile();
    }
    // Clear encryption key.
    _key.clear();

    // Clear filename and set to default.
    _stateFileName = "./.isaacrngstate";

    // uninitialized state.
    _initialized = false;
  }

  // ---------------
  // saveStateToFile
  // ---------------

  /**
   * @brief Saves state to file. If a valid encryption key (AES-GCM) is
   *        set, encryption is initialized before writing to file.
   * @return true, if file was written to sucessfully.
   */
  template<int ALPHA, class T>
  bool QTIsaac<ALPHA,T>::saveStateToFile(){
      // Initialize object to write to file _stateFileName.
      FileCryptopp fileEncryptor(_stateFileName);

      // Create file stream.
      std::stringstream fileStream;

      // Output iterator over the file stream.
      std::ostream_iterator<T> oit(fileStream, " ");

      // Write internal state to fileStream.
      *oit = m_rc.randcnt;
      ++oit;
      std::copy(m_rc.randrsl, m_rc.randrsl+N, oit);
      std::copy(m_rc.randmem, m_rc.randmem+N, oit);
      *oit = m_rc.randa;
      ++oit;
      *oit = m_rc.randb;
      ++oit;
      *oit = m_rc.randc;
      ++oit;

      // Write to file, a valid key will encrypt before writing to file.
      return fileEncryptor.writeFile(fileStream, _key);
  }

  // -----------------
  // loadStateFromFile
  // -----------------

  /**
   * @brief Reads state from file; decryts if a valid key is available.
   *
   * @param file const reference to a string with file path and name.
   * @param key const reference to a vector of uint8_t with key bytes.
   *
   * @return int with value -2 if decryption failed, -1 if file was
   *         not found and 0 if state was read sucessfully.
   */
  template<int ALPHA, class T>
  int QTIsaac<ALPHA,T>::loadStateFromFile(
    const std::string& file,
    std::vector<uint8_t> key
  ) {

      // Initialize module to read and decrypt file contents.
      FileCryptopp fileDecryptor(file);

      // Check if file containing state exists.
      if (!fileDecryptor.fileExists()) {
        _initialized = false;
        return -1; // file not found
      }

      // Initialize string stream to hold state.
      std::stringstream fileStream;

      // Attempt to read and decrypt file.
      if (!fileDecryptor.readFile(fileStream, key)) {
        _initialized = false;
        return -2; // Failed decryption.
      }

      std::vector<T> stateData;
      stateData.reserve(3 * N);

      // Load decrypted data from file into vector.
      std::copy(
        std::istream_iterator<T>(fileStream),
        std::istream_iterator<T>(),
        std::back_inserter(stateData)
      );

      // Use state data to set isaac rng internal state.
      m_rc.randcnt = stateData[0];

      std::copy(
        stateData.begin() + 1,
        stateData.begin() + 1 + N,
        m_rc.randrsl
      );

      std::copy(
        stateData.begin() + 1 + N,
        stateData.begin() + 1 + N + N,
        m_rc.randmem
      );

      m_rc.randa = *(stateData.begin() + 1 + N + N + 1);
      m_rc.randb = *(stateData.begin() + 1 + N + N + 2);
      m_rc.randc = *(stateData.begin() + 1 + N + N + 3);

      // Update the object with the given file and key and initialize state.
      setIdentifier(file);
      setKey(key);
      _initialized = true;
      return 0; // Successfully loaded state.
  }


  // ------------
  // getValidFile
  // ------------

  /**
   * @brief Changes the given file name to a valid file name. Also truncates
   *        the file name to 32 bytes to avoid errors.
   *
   * @param validFileName string reference to store valid file name.
   * @param file const reference to a file name.
   *
   * @return void
   */
  template<int ALPHA, class T>
  void QTIsaac<ALPHA,T>::getValidFile(
    std::string& validFileName,
    const std::string& file
  ) {

    // Extract filename from path.
    size_t pos = file.find_last_of('/');

    // Check if only filename was given.
    if (pos == std::string::npos) {
      // Set current directory as path.
      validFileName = "./" + file;
    } else {

      std::string filename = file.substr(pos);
      std::string path = file.substr(0, pos);

      // Truncate filename to 32 bytes.
      if (filename.length() > 32) {
        filename = filename.substr(0,32);
      }

      // Set filename with file path.
      validFileName = path + filename;
    }

  }


#endif // __ISAAC_HPP