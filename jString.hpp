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
 * Created on 15 October 2022, 11:58 AM
 */
#ifndef jString_HPP
#define jString_HPP
#include <cstddef>
#include <cstring>
#include <ostream>
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

    class jStringBase: public CountedPointerTarget {
        /**
         * A class to store the string data.
        */
        private:
            friend class jString;
            friend std::ostream& operator<<(std::ostream& os, const class jString & string);
            std::size_t m_len;
            // Not the most efficient, but portable.
            char * m_data;
            // suppress assignment into the string
            jStringBase & operator=(const jStringBase &);
            // ctor private so only jString can access it
            jStringBase( const char * data, std::size_t len ){
                m_data = new char[len+1];
                m_len = len;
                strncpy(m_data, data, len);
                m_data[len] = '\0';
            }
        public:
            operator const char *() const { return m_data;};
            auto len() const { return m_len;}
    };

    class jString: /* private */ CountedPointer<jStringBase> {
        /***
         * The user interface to the strings
        */
        private:
            static jString get_empty() {
                static jString empty_base("");
                return empty_base;
            }
            const char * data() const { return get()->m_data;}
            friend std::ostream& operator<<(std::ostream& os, const class jString & string);
        public:
            // create from an existing base
            // shouldn't really happen, but who knows
            // what changes will occur in the future or
            // either lemon or re2c does something weird
            // in its generated code?
            jString( jStringBase *base )
                : CountedPointer<jStringBase>(base)
             {}
            // Create from c string
            jString( const char * source ):
                CountedPointer<jStringBase>(new jStringBase(source, strlen(source)))
                {}
            // Create from start of c string
            jString( const char * source, std::size_t len ):
                CountedPointer<jStringBase>(new jStringBase(source, len))
                {}
            // Create from two pointers to the same buffer
            // NB: This is what re2c gives on a match
            jString( const char * start, const char * end ):
                CountedPointer<jStringBase>(new jStringBase(start, end-start))
                {}
            // string concatenation
            jString operator + (const jString &rhs) {
                jStringBase * lhs_base = get();
                if( lhs_base->m_len == 0 )
                    return *this;
                auto len = lhs_base->m_len + rhs->m_len;
                jStringBase * new_base = new jStringBase(lhs_base->m_data,len + 1 );
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
                return new jStringBase(get()->m_data, ilen );
            }
            jString right(std::size_t len) const {
                auto ilen=std::min(len,get()->m_len);
                if(ilen == get()->m_len)
                    return *this;
                auto start=get()->m_len-ilen;
                return new jStringBase(get()->m_data+start, ilen );
            }
            jString substr(std::size_t start, std::size_t len) const {
                if( start >= get()->m_len ){
                    return get_empty();
                }
                auto ilen=std::min(get()->m_len-start, len);
                if(ilen == get()->m_len)
                    return *this;
                return new jStringBase(get()->m_data+start,ilen);
            }
            bool operator < (const jString &rhs) const {
                return strcmp(get()->m_data, rhs->m_data) < 0;
            }
            bool operator > (const jString &rhs) const {
                return strcmp(get()->m_data, rhs->m_data) > 0;
            }
            bool operator == (const jString &rhs) const {
                return strcmp(get()->m_data, rhs->m_data) == 0;
            }
            operator const char *() const { return (*this)->m_data;};
    };


    std::ostream& operator<<(std::ostream& os, const jString & string) {
        os << string.data();
        return os;
    }
};

#endif
