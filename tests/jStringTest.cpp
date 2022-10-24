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
 * File:   jStringTest.cpp
 *         Tests jString.hpp
 * Author: Julia Clement <Julia at Clement dot nz>
 *
 * Created on 15 October 2022, 11:58 AM
 */

#include "../jString.hpp"
using namespace jclib;
#include <iostream>
#include <cassert>

int main(int argc, char ** argv) {
    jString a = "This is a string";
    assert( strcmp( (const char *)a, "This is a string") == 0);
    assert( strcmp( (const char *)a.left(4), "This") == 0);
    jString b(a);
    auto buf="xxxyyy";
    auto c = jString(buf,buf+3);
    auto d = b+c;
    std::cout << d << "\n";
    auto e = a.substr(5,2);
    assert( e == jString("is"));
    auto f = e.substr(5,2);
    assert( f == jString(""));
    auto g=a.right(6);

    // Check we have the expected values:

    assert( a == jString("This is a string") );
    assert( b == jString("This is a string") );
    assert( c == jString("xxx"));
    assert( d == jString("This is a stringxxx")); // b+c
    assert( e == jString("is"));
    assert( f == jString(""));
    assert( g==jString("string"));

    // Check comparison operators
    assert(a==b);
    assert(d<c);
    assert(d<=c);
    assert(d>a);
    assert(d>b);
    assert(d>=a);
    assert(a<=d);
    assert(d!=a);
    std::cout << "OK (0)\n";
    return 0;
}