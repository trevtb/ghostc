#ifndef SPACECRYPTOWRAPPER_H
#define SPACECRYPTOWRAPPER_H

/*Some Wrappers For Crypto++*/
/*============================================================================================================
                                            SPACE CRYPTO
                                      A Simple Crypto++ Wrapper
==============================================================================================================
                    by Space Soft (Stefano Guerrini)
                        Release Date: 26/06/10

This library, helps you to develop your applications, in a very easy way, with Crypto++.
It permits a developer with a minimal understanding of C++ to rapid use a crypting library!

The SpaceCrypto is divided in Two Class: The Crypt one and the Hash one.
The first one, allow you to perform rapid encrypt\decrypt operations with the most common Methods (AES, Twofish, DES,...)
using the c++ string (std::string).

CRYPTO CLASS USAGE:
After declaring an istance of the Crypto Class, you must set the plain Text, the Key, and optionally the IV.
The function parameters are self explainatory.
Remember that you must specify the incoming string format type (HEX or normal), or it will automatically turn into normal input.
You can also specify the output type.
(setInputMode & set OutputMode)
When you're ready to encrypt (or decrypt) your string, you need to call Encrypt() (or decrypt()) method, this
will return you the processed string.

Error Handling:
getStatus() function, will return a boolean value, rapresenting the current status of obj (true = error; false=no errors)
getErrorMsg() function, will return a string, indicating the current error msg

You can only perform one encrypt\decrypt function at time.
To perform another crypt operation with the same Crypto object, you need to call reset() function (For security reason too..)
This is a security reason too, but it has another explaination...
When you call for the first time the Encrypt function (or decrypt one), the obj proceed to encrypt all data.
So if you call the Encrypt Function again (to retrieve another time the coded string), he doesn't call again crypto++ crypting function,
but will return the already encoded string.

SECURITY ADVISE: For this reason, we Strongly reccomend you to call reset() function, immediatly after the conversion, if you don't need to call
other functions again, because the plain string and the plain key, will remain in RAM, until you delete the object!

HASH CLASS USAGE:
The hash function, work similar to the crypto class, with some exception.
- Remember that hashes functions are one-way crypt..so you can't retrieve the string back...
- Hash Class offers the addStr function that allow you to add some text to the plain string
*/

#include "modes.h"
#include "hex.h"
#include "filters.h"
#include "aes.h"
#include "sha.h"
#include "blowfish.h"
#include "twofish.h"
#include "des.h"
#include "serpent.h"
#include "base64.h"
#include "osrng.h"
#include "md5.h"

//Function to convert unsigned char to string of length 2
void Char2Hex(const unsigned char ch, char* szHex) {
    unsigned char byte[2];
    byte[0] = ch/16;
    byte[1] = ch%16;

    for(int i=0; i<2; i++) {
        if(byte[i] >= 0 && byte[i] <= 9) {
            szHex[i] = '0' + byte[i];
        } else {
            szHex[i] = 'A' + byte[i] - 10;
        } //endif
    } //endfor

    szHex[2] = 0;
} //endfunction Char2Hex

//Function to convert string of length 2 to unsigned char
void Hex2Char(const char* szHex, unsigned char& rch) {
    rch = 0;

    for(int i=0; i<2; i++) {
        if(*(szHex + i) >='0' && *(szHex + i) <= '9') {
            rch = (rch << 4) + (*(szHex + i) - '0');
        } else if(*(szHex + i) >='A' && *(szHex + i) <= 'F') {
            rch = (rch << 4) + (*(szHex + i) - 'A' + 10);
        } else {
            break;
        } //endif
    } //endfor
} //endfunction Hex2Char

//Function to convert string of unsigned chars to string of chars
void CharStr2HexStr(const unsigned char* pucCharStr, char* pszHexStr, int iSize) {
    int i;
    char szHex[3];
    pszHexStr[0] = 0;

    for(i=0; i<iSize; i++) {
        Char2Hex(pucCharStr[i], szHex);
        strcat(pszHexStr, szHex);
    } //endfor
} //endfunction CharStr2HexStr

//Function to convert string of chars to string of unsigned chars
void HexStr2CharStr(const char* pszHexStr, unsigned char* pucCharStr, int iSize) {
    int i;
    unsigned char ch;
    for(i=0; i<iSize; i++) {
        Hex2Char(pszHexStr+2*i, ch);
        pucCharStr[i] = ch;
    } //endfor
} //endfunction HexStr2CharStr

namespace SpaceCrypto {
    enum OUT_TYPE { //OUT&IN TYPE (HEX OR NORMAL ONE)
        HEX = 0,
        NORMAL = 1,
    };

    template <typename T>

    class Crypt {
        public:
            const int getKeyLengthMultple() { return T::KEYLENGTH_MULTIPLE;}
            Crypt() { //Default Constructor
                reset(); //Start with resetting our data variables
            } //endconstructor

            /*Encrypt Function*/
            std::string Encrypt() {
                if(_error == true) {
                    return errormsg;
                } //endif

                if(_key.size() == 0 || _plain.size() == 0) {
                    _error = true;
                    errormsg = "You need to set key and\or plain text";
                } //endif

                if(enc == false) { //We haven't already crypted our string..so we need to encrypt it!
                    if(in_mode==HEX) { //Before normalizing data we need to convert HEX strings into Normal one...
                        char* normstr;
                        normstr = new char [_plain.size()+1];
                        HexStr2CharStr(_plain.c_str(),(unsigned char*)normstr,_plain.size());
                        _plain = normstr;
                        delete [] normstr;
                    } //endif

                    this->initializeIV(); //Initializing IV to a padding string (Only '0' chars)
                    try {
                        _enc.SetKeyWithIV((byte*)_key.c_str(),_key.length(),(byte*)_iv.c_str(),_iv.length()); //Set KEY & IV
                        CryptoPP::StringSource( _plain, true,new CryptoPP::StreamTransformationFilter( _enc, new CryptoPP::StringSink( _encrypted ))); //Crypting and converting to string...

                        if(out_mode == HEX) { //If user request to convert string to HEX...
                            char * hexstr; //Output String
                            hexstr = new char[_encrypted.size()*2+1];
                            CharStr2HexStr((unsigned char*)_encrypted.c_str(),hexstr,_encrypted.size()); //Converting string...
                             _encrypted = hexstr; //Prepariamo il nostro oggetto di ritorno
                            delete[]hexstr;
                        } //endif

                    } catch (CryptoPP::Exception& e) {
                        std::string errText = "Bad encrypt";
                        errormsg = e.GetWhat();
                        _error = true;
                        return errormsg;
                    } //endtry
                } //endif

                enc = true; //Successful!
                return _encrypted;

            } //endfunction Ecrypt

            std::string Decrypt() {
                if(_error == true) {
                    return errormsg;
                } //endif

                if(_key.size() == 0 || _encrypted.size() == 0) {
                    _error = true;
                    errormsg = "You need to set key and\or plain text";
                    return errormsg;
                } //endif

                if(dec == false && enc == false) { //Process only if we've got an 'empty' SpaceCrypto OBJ
                    if(in_mode==HEX) {
                        char* normstr;
                        normstr = new char [_encrypted.size()+1];
                        HexStr2CharStr(_encrypted.c_str(),(unsigned char*)normstr,_encrypted.size());
                        _encrypted = normstr;
                        delete [] normstr;
                    } //endf

                    this->initializeIV();

                    try {
                        _dec.SetKeyWithIV((byte*)_key.c_str(),_key.length(),(byte*)_iv.c_str());

                        CryptoPP::StringSource( _encrypted, true,new CryptoPP::StreamTransformationFilter( _dec, new CryptoPP::StringSink( _plain )));

                        if(out_mode == HEX) {
                            char * hexstr; //Dichiariamo la stringa di output esadecimale
                            hexstr = new char[_encrypted.size()*2+5]; //Allochiamo lo spazio del doppio (data la grandezza esadecimale), prendendo qualche carattere in piu per sicurezza
                            CharStr2HexStr((unsigned char*)_plain.c_str(),hexstr,_plain.size()); //Convertiamo da Stringa a stringa esadecimale
                            _plain = hexstr; //Prepariamo il nostro oggetto di ritorno
                            delete[]hexstr;
                        } //endif

                    } catch (CryptoPP::Exception& e) {
                        std::string errText = "Bad Decrypt";
                        errormsg = e.GetWhat();
                        _error = true;
                        return errormsg;
                    } //endtry
                } //endif

                return _plain;
            } //endfunction Decrypt

            void setKey(std::string key) {
                if(key.size() == 0 || key.size()%T::KEYLENGTH_MULTIPLE != 0) { //Non valid Key Length...
                    _error = true;
                    errormsg="No valid Key Length";
                } //endif

                if(_error == false) { //No error existing...
                    key = key;
                } //endf
            } //endfunction setKey

            void setIV(std::string iv) {
                if(iv.size() == 0 || iv.size()%T::IV_LENGTH != 0 ) { //Non valid IV Lenght
                    _error = true;
                    errormsg="No valid IV Length";
                } //endif

                if(_error == false) {
                    ivsetted = true;
                    _iv = iv;
                } //endif
            } //endfunction setIV

            void setPlainString(std::string plainstring) {
                if(plainstring.size() == 0 ) { //Empty string??
                    _error = true;
                    errormsg= "No valid Plain String Length";
                } //endif

                if(_error == false) {
                    _plain = plainstring;
                } //endif
            } //endfunction setPlainString

            void setEncString(std::string encryptedstring) {
                if(encryptedstring.size() == 0 ) {
                    _error = true;
                    errormsg= "No valid Encrypted String Length";
                } //endif

                if(_error == false) {
                    _encrypted = encryptedstring;
                } //endif
            } //endfunction setEncString

            inline void setOutputMode(OUT_TYPE type){out_mode = type;}
            inline void setInputMode(OUT_TYPE type){in_mode = type;}

            void reset() {
                out_mode=HEX;
                in_mode=NORMAL;
                enc=false;
                dec=false;
                _error = false;
                ivsetted=false;
                errormsg = "";
                _encrypted="";
                _key="";
                _plain="";
                _iv="";
            } //endfunction reset

            inline bool getStatus(){ return _error;}
            inline std::string getErrorMsg(){return errormsg;}

        private:
            void initializeIV() {
                if(ivsetted == false) {
                    for(int i=0;i<T::BLOCKSIZE;i++) {//Default IV
                        _iv.push_back('0');
                    } //endfor
                } //endif
            } //endfunction initializeIV

            std::string _key;
            std::string _plain;
            std::string _encrypted;
            std::string _iv;
            std::string errormsg;
            typename CryptoPP::CBC_Mode<T>::Encryption _enc;
            typename CryptoPP::CBC_Mode<T>::Decryption _dec;
            bool _error;
            bool dec;
            bool enc;
            bool ivsetted;
            OUT_TYPE out_mode;
            OUT_TYPE in_mode;
    }; //endclass Crypt

    typedef Crypt<CryptoPP::AES> CryptAES;
    //!Typedef for the AES Encryption\Decryption
    typedef Crypt<CryptoPP::Blowfish> CryptBlowFish;
    //!Typedef for BlowFish Encryption\Decryption
    typedef Crypt<CryptoPP::Twofish> CryptTwoFish;
    //!Typedef for BlowFish Encryption\Decryption
    typedef Crypt<CryptoPP::DES> CryptDES;
    //!Typedef for DE  Encryption\Decryption
    typedef Crypt<CryptoPP::DES_EDE3> CryptTripleDES;
    //!Typedef for 3-DES Encryption\Decryption
    typedef Crypt<CryptoPP::Serpent> CryptSerpent;
    //!Typedef for SerpentEncryption\Decryption

    template <typename T>
    class Hash {
        public:
            Hash() {
                reset();
            } //endconstructor

            void reset() {
                processed = false;
                _error = false;
                _plain.empty();
                _plain = "";
                _encrypted.empty();
                hashobj.Restart();
                out_mode = HEX;
                in_mode = NORMAL;
            } //endfunction reset

            std::string Encrypt() {
                if(processed == true) {
                    return _encrypted;
                } //endif

                if(_plain.size() == 0) {
                    _error = true;
                    errormsg = "You need to set plain text";
                    return errormsg;
                } //endif

                if(in_mode == HEX) { //Before normalizing data we need to convert HEX strings into Normal one...
                    char* normstr;
                    normstr = new char [_plain.size()+1];
                    HexStr2CharStr(_plain.c_str(),(unsigned char*)normstr,_plain.size());
                    _plain = normstr;
                    delete [] normstr;
                } //endif

                if(_error == false) {
                    try {
                        byte digest[ T::DIGESTSIZE ];
                        hashobj.Final(digest);

                        if(out_mode == HEX) {
                            char * hexout = new char[sizeof(digest)*2+1];
                            CharStr2HexStr((unsigned char*)digest,hexout,sizeof(digest));
                            _encrypted = hexout;
                            delete[]hexout;
                        } else {
                            _encrypted = (char*)digest;
                        } //endif

                        if(_encrypted != "") {
                            processed = true;
                            return _encrypted;
                        } //endif
                    } catch (CryptoPP::Exception& e) {
                        std::string errText = "Bad Hash encrypt";
                        errormsg = e.GetWhat();
                        _error = true;
                        return errText;
                    } //endtry
                } //endif

                _error = true;
                return "";
            } //endfunction Encrypt

            inline void setOutputMode(OUT_TYPE type){out_mode = type;}
            inline void setInputMode(OUT_TYPE type){in_mode = type;}

            void setPlain(std::string plain) {
                if(plain.size() == 0) {
                    _error = true;
                    errormsg = "No plain text specified";
                } //endif

                if(_plain == plain && _error != true) {
                    _error = false;
                } else {
                    errormsg = "Error during plain text assign";
                } //endif
            } //endfunction setPlain

            void addStr(std::string string) {
                if(_error != true) {
                    if(string.size() != 0) {
                        _plain += string;
                        hashobj.Update((byte*)string.c_str(),string.size());
                    } //endif
                } else {
                    _error = false;
                } //endif
            } //endfunction addStr

            inline bool getStatus(){ return _error;}
            inline std::string getErrorMsg(){return errormsg;}

        private:
            std::string _plain;
            std::string _encrypted;
            std::string errormsg;
            bool _error;
            bool processed;
            OUT_TYPE out_mode;
            OUT_TYPE in_mode;
            T hashobj;

    }; //endclass Hash

    typedef Hash<CryptoPP::SHA1> HashSHA1;
    typedef Hash<CryptoPP::MD5> HashMD5;

} //endnamespace SpaceCrypto

#endif
