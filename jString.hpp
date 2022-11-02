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
     * dynamic invariant strings in a re2cc/lemon based project
     * which will be passing a ridiculous number of strings around.
    */

    // If you get strange crashes, enable this #define
    // To double the number of heap allocations / frees
    // but do it safely
    //#define SAFE_JSTRING 1

    struct jStringBase {
        /**
         * A class to store the string data.
        */
        private:
            friend class jString;
            friend class CountedPointer<jStringBase>; // = jString's parent class;
            friend std::ostream& operator<<(std::ostream& os, const class jString & string);
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
            , m_data( new char[len+1])
#endif
            , m_len( len )
            {
                strncpy(m_data, data, len);
                m_data[len] = '\0';
            }
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
#ifdef SAFE_JSTRING
            virtual ~jStringBase() {
                delete m_data;
            }
#endif
            std::size_t m_len;
#ifdef SAFE_JSTRING
            // Not the most efficient, but portable.
            char * m_data;
#else
            // **** This MUST be the last data member in the struct as
            // we deliberately overrun the declared length of the array.
            //
            // I don't recall seeing any guarantee that struct members
            // are laid out in declaration order, but every compiler I
            // know works that way. 
            // If you get strange crashes, try defining SAFE_JSTRING
            char m_data[1];
#endif
        public:
            operator const char *() const { return m_data;};
            auto len() const { return m_len;}
    };

    class jString: /* private */ CountedPointer<jStringBase> {
        /***
         * The user interface to the strings
        */
        public:
            /** return an empty string.
             *  We use so many it makes sense to pre-create one
            */
            static jString get_empty() {
                static jString empty_base("");
                return empty_base;
            } 

            const char * data() const { return get()->m_data;}
            
            operator const char *() const { return (*this)->m_data;};

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
            jString( jStringBase *base )
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
                if( lhs_base->m_len == 0 )
                    return rhs;
                if( rhs.len() == 0 )
                    return *this;
                auto len = lhs_base->m_len + rhs->m_len;
                jStringBase * new_base = jStringBase::create(lhs_base->m_data,len );
                strncpy(new_base->m_data + lhs_base->m_len,rhs->m_data, rhs->m_len );
                new_base->m_data[len]='\0';
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
                auto ilen=std::min(len,get()->m_len);
                if(ilen == get()->m_len)
                    return *this;
                return jStringBase::create(get()->m_data, ilen );
            }
            jString right(std::size_t len) const {
                auto ilen=std::min(len,get()->m_len);
                if(ilen == get()->m_len)
                    return *this;
                auto start=get()->m_len-ilen;
                return jStringBase::create(get()->m_data+start, ilen );
            }
            jString substr(std::size_t start, std::size_t len) const {
                if( start >= get()->m_len ){
                    return get_empty();
                }
                auto ilen=std::min(get()->m_len-start, len);
                if(ilen == get()->m_len)
                    return *this;
                return jStringBase::create(get()->m_data+start,ilen);
            }

            jString pad(std::size_t length, char pad_char = ' ' ) const {
                if( length <= len())
                    return *this;
                
                jStringBase * new_stringBase = jStringBase::create(get()->m_data, length );
                
                char * pad_ptr = new_stringBase->m_data + len();
                for( ; pad_ptr < new_stringBase->m_data + length; ++ pad_ptr ) {
                    *pad_ptr = pad_char;
                }
                *pad_ptr = '\0';
                return new_stringBase;
            }
            
            std::vector<jString> split(const char * delim, int max_splits=std::numeric_limits<int>::max() ) const {
                std::vector<jString> answer;
                const char * buffer = get()->m_data;
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
            bool operator < (const jString &rhs) const {
                return strcmp(get()->m_data, rhs->m_data) < 0;
            }
            bool operator <= (const jString &rhs) const {
                return strcmp(get()->m_data, rhs->m_data) <= 0;
            }
            bool operator > (const jString &rhs) const {
                return strcmp(get()->m_data, rhs->m_data) > 0;
            }
            bool operator >= (const jString &rhs) const {
                return strcmp(get()->m_data, rhs->m_data) >= 0;
            }
            bool operator == (const jString &rhs) const {
                return strcmp(get()->m_data, rhs->m_data) == 0;
            }
            bool operator != (const jString &rhs) const {
                return strcmp(get()->m_data, rhs->m_data) != 0;
            }
    };


    inline std::ostream& operator<<(std::ostream& os, const jString & string) {
        os << string.data();
        return os;
    }
};

#endif
