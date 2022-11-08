/*
Copyright (c) 2022 Julia Ingleby Clement

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

/* 
 * File:   jString.hpp
 * Author: Julia Clement <Julia at Clement dot nz>
 * 
 * Part of the JCLib project https://github.com/juliaclement/jclib
 *
 * Created on 15 October 2022, 11:58 AM
 */
#ifndef jString_HPP
#define jString_HPP
#include <limits>
#include <cctype>
#include <cstring>
#include <string_view>
#include <ostream>
#include <vector>
#include "countedPointer.hpp"

namespace jclib {
    /**
     * Implementation of a reference counted string
     * This string is lighweight with the string class
     * itself being little more than a slightly smart pointer.
     * 
     * Primarily created to satisfy a need I have for
     * dynamic invariant strings in a re2c/lemon based project
     * which will be passing a ridiculous number of strings around.
    */

    class jString;
    namespace Private {
        // If you get strange crashes, enable this #define
        // It doubles the number of heap allocations / frees
        // but does it safely
        //#define SAFE_JSTRING 1

        struct jStringBase {
            /**
             * A class to store the string data.
            */
            private:
                friend class jclib::jString;
                friend class jclib::CountedPointer<jStringBase>; // = jString's parent class;
                friend std::ostream& operator<<(std::ostream& os, const class jclib::jString & string);
                // no copy constructor
                jStringBase( const jStringBase & ) = delete;
                // suppress assignment into the string
                jStringBase& operator = (const jStringBase&) = delete;
                // Implement the requirements for CountedPointer
                int counter_;
                inline void CountedPointerAttach() { ++counter_; }
                inline void CountedPointerDetach() { if( !--counter_) delete this;}
                // ctor private so only jString can access it
                jStringBase( const char * data, std::size_t len )
                : counter_(0)
    #ifdef SAFE_JSTRING
                , data_( new char[len+1])
    #endif
                , len_( len )
                {
                    strncpy(data_, data, len);
                    data_[len] = '\0';
                }
                /** create a jStringBase with data */
                static jStringBase * create(const char * data, std::size_t len) {
    #ifdef SAFE_JSTRING
                    // Double create on heap
                    return new jStringBase(data, len);
    #else
                    void * new_store = new unsigned char[len + sizeof(jStringBase)];
                    jStringBase * new_object= new (new_store) jStringBase(data, len);
                    return new_object;
    #endif
                }
                /** create an empty jStringBase */
                static jStringBase * create(std::size_t len) {
    #ifdef SAFE_JSTRING
                    // Double create on heap
                    return new jStringBase(data, len);
    #else
                    void * new_store = new unsigned char[len + sizeof(jStringBase)];
                    jStringBase * new_object= new (new_store) jStringBase("", len);
                    return new_object;
    #endif
                }
    #ifdef SAFE_JSTRING
                virtual ~jStringBase() {
                    delete data_;
                }
    #endif
                std::size_t len_;
    #ifdef SAFE_JSTRING
                // Not the most efficient, but portable.
                char * data_;
    #else
                // **** This MUST be the last data member in the struct as
                // we deliberately overrun the declared length of the array.
                //
                // I don't recall seeing any guarantee that struct members
                // are laid out in declaration order, but every compiler I
                // know works that way. 
                // If you get strange crashes, try defining SAFE_JSTRING
                char data_[1];
    #endif
            public:
                operator const char *() const { return data_;};
                auto len() const { return len_;}
        };
    };

    class jString: /* private */ CountedPointer<Private::jStringBase> {
        /***
         * The user interface to the strings
        */
        private:
            typedef Private::jStringBase jStringBase;
        public:
            /** return an empty string.
             *  We use so many it makes sense to pre-create one
            */
            static jString get_empty() {
                static jString empty_base("");
                return empty_base;
            } 

            const char * data() const { return get()->data_;}
            
            operator const char *() const { return (*this)->data_;};

            size_t len() const { return get()->len();}

            friend std::ostream& operator<<(std::ostream& os, const class jString & string);
            
            bool isvalid() const {
                return isset();
            }
            
            // create from an existing base
            // shouldn't really happen, but who knows
            // what changes will occur in the future or
            // either lemon or re2c does something weird
            // in its generated code?
            jString( Private::jStringBase *base )
                : CountedPointer<jStringBase>(base)
             {}
            // Create empty string
            jString( ):
                CountedPointer<jStringBase>(get_empty().get())
                {}
            // Create from c string
            jString( const char * source ):
                CountedPointer<jStringBase>(jStringBase::create(source, strlen(source)))
                {}
            // Create from start of c string
            jString( const char * source, std::size_t len ):
                CountedPointer<jStringBase>(jStringBase::create(source, len))
                {}
            // Create from two pointers to the same buffer
            // NB: This is what re2c gives on a match
            jString( const char * start, const char * end ):
                CountedPointer<jStringBase>(jStringBase::create(start, end-start))
                {}
#ifdef __cpp_lib_string_view
            // Create from std::string_view
            // NB: Same as previous, really
            jString( const std::string_view & view ):
                CountedPointer<jStringBase>(jStringBase::create(view.begin(), view.length()))
                {}
            // Cast to std::string_view
            operator std::string_view () const {
               return std::string_view( data(), len() );
            }
#endif
            // string concatenation
            jString operator + (const jString &rhs) {
                jStringBase * lhs_base = get();
                if( lhs_base->len_ == 0 )
                    return rhs;
                if( rhs.len() == 0 )
                    return *this;
                auto len = lhs_base->len_ + rhs->len_;
                jStringBase * new_base = jStringBase::create(lhs_base->data_,len );
                strncpy(new_base->data_ + lhs_base->len_,rhs->data_, rhs->len_ );
                new_base->data_[len]='\0';
                return new_base;
            }
            // extract substrings (left, right, any) with
            // silent bounds checks.
            // E.g. if z=="123", z.substr(2,999) will quietly return "3"
            // NB: Const but when the whole string is to be 
            // returned, we return a copy of ourself not a
            // whole new string. This saves creating a new 
            // jStringBase.
            jString left(std::size_t len) const {
                auto ilen=std::min(len,get()->len_);
                if(ilen == get()->len_)
                    return *this;
                return jStringBase::create(get()->data_, ilen );
            }
            jString right(std::size_t len) const {
                auto ilen=std::min(len,get()->len_);
                if(ilen == get()->len_)
                    return *this;
                auto start=get()->len_-ilen;
                return jStringBase::create(get()->data_+start, ilen );
            }
            jString substr(std::size_t start, std::size_t len) const {
                if( start >= get()->len_ ){
                    return get_empty();
                }
                auto ilen=std::min(get()->len_-start, len);
                if(ilen == get()->len_)
                    return *this;
                return jStringBase::create(get()->data_+start,ilen);
            }

            jString pad(std::size_t length, char pad_char = ' ' ) const {
                if( length <= len())
                    return *this;
                
                jStringBase * new_stringBase = jStringBase::create(get()->data_, length );
                
                char * pad_ptr = new_stringBase->data_ + len();
                for( ; pad_ptr < new_stringBase->data_ + length; ++ pad_ptr ) {
                    *pad_ptr = pad_char;
                }
                *pad_ptr = '\0';
                return new_stringBase;
            }
            
            /*** Split string on delim, retuirning answers as an array of strings,
             *   once max_splits copies of the delimiter have been found, the 
             * remainder if any is a single string at the end of the array.
            */
            std::vector<jString> split(const char * delim, int max_splits=std::numeric_limits<int>::max() ) const {
                std::vector<jString> answer;
                const char * buffer = get()->data_;
                const char * buffer_end = buffer+len();
                const int step = strlen(delim);
                auto position = strstr( buffer, delim );
                if( position == nullptr ){
                    answer.push_back(*this);
                    return answer;
                }
                for(    ;
                        position != nullptr && max_splits != 0;
                        --max_splits ){
                    jString chunk(buffer,position);
                    answer.push_back( chunk );
                    buffer += chunk.len() + step;
                    position = (buffer < buffer_end) ? strstr( buffer, delim ) : nullptr;
                }
                if( buffer < buffer_end ) {
                    answer.push_back(jString(buffer));
                }
                return answer;
            }

            /** Return position of first occurrence of findee */
            int find( jString findee ) {
                auto buffer = data();
                auto findee_chars = findee.data();
                auto position = strstr( buffer, findee_chars );
                if( position == nullptr )
                    return -1;
                return position - buffer;
            }

            /** Check is only all alphabetic ascii characters */
            bool isalpha() {
                if( len() == 0 )
                    return false;
                for( const char * c = data(); *c; ++c) {
                    if( ! std::isalpha(*c) )
                        return false;
                }
                return true;
            }

            /** Check all alphabetic ascii characters in string are upper case */
            bool isupper() {
                if( len() == 0 )
                    return false;
                for( const char * c = data(); *c; ++c) {
                    if( std::isalpha(*c) && ! std::isupper(*c))
                        return false;
                }
                return true;
            }

            /** Check all alphabetic ascii characters in string are upper case */
            bool islower() {
                if( len() == 0 )
                    return false;
                for( const char * c = data(); *c; ++c) {
                    if( std::isalpha(*c) && ! std::islower(*c))
                        return false;
                }
                return true;
            }

            /** Join elements in an iterable into a new string
             *  with the elements separated by our value.
             *  Elements must be valid for jString constructors.
             *  Based on Python's str.join() method
             */
            template<class T> jString join( const T & input ) {
                size_t total_length = 0;
                std::vector<jString> strings; // No guarantee that input can be parsed twice
                for( jString bit : input ) {
                    strings.push_back( bit );
                    total_length += bit.len() + len();
                }
                total_length = total_length + 1 - len();
                jStringBase * answer = jStringBase::create(total_length);
                char * out = const_cast<char *>(answer->data_);
                bool first = true;
                for( jString bit : strings ) {
                    if( first )
                        first = false;
                    else {
                        memmove(out, data(), len());
                        out+=len();
                    }
                    memmove(out, bit.data(), bit.len());
                    out+=bit.len();
                }
                *out='\0';
                return answer;
            }

            bool operator < (const jString &rhs) const {
                return strcmp(get()->data_, rhs->data_) < 0;
            }
            bool operator <= (const jString &rhs) const {
                return strcmp(get()->data_, rhs->data_) <= 0;
            }
            bool operator > (const jString &rhs) const {
                return strcmp(get()->data_, rhs->data_) > 0;
            }
            bool operator >= (const jString &rhs) const {
                return strcmp(get()->data_, rhs->data_) >= 0;
            }
            bool operator == (const jString &rhs) const {
                return strcmp(get()->data_, rhs->data_) == 0;
            }
            bool operator == (const char *rhs) const {
                return strcmp(get()->data_, jString(rhs)->data_) == 0;
            }
            bool operator != (const jString &rhs) const {
                return strcmp(get()->data_, rhs->data_) != 0;
            }
    };


    inline std::ostream& operator<<(std::ostream& os, const jString & string) {
        os << string.data();
        return os;
    }
};

#endif
