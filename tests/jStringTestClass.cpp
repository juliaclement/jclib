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
 * File:   jStringTestClass.cpp
 * Author: Julia Clement <Julia at Clement dot nz>
 *
 * Created on 25/10/2022, 17:05:55
 */

// #include "cstdafx"
#ifdef ONE_FIXTURE
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#endif
#include <cppunit/extensions/HelperMacros.h>
#include "../jString.hpp"
using namespace jclib;

namespace jStringTestClassNS {
    typedef struct data_ {
        jString a = "This is a string";
        jString b = a;
        const char * buf="xxxyyy";
        jString c = jString(buf,buf+3);
        jString d = b+c;
        jString e = a.substr(5,2);
        jString  f = e.substr(5,2);
        jString  g=a.right(6);
        jString  h=a.left(4);
    } struct_data;
};
using namespace jStringTestClassNS;

class jStringTestClass : public CPPUNIT_NS::TestFixture {
public:
    struct_data sd;
    jStringTestClass() {}
    virtual ~jStringTestClass() {}
    void setUp(){
    }
    void tearDown(){}
private:
    void testDefaultCtorIsEmpty() {
        jString empty;
        CPPUNIT_ASSERT(empty.len()==0);
    }
    void testInitialisers() {
        jString empty;
        CPPUNIT_ASSERT(empty.len()==0);

        // Check we have the expected values:
        CPPUNIT_ASSERT( sd.a == jString("This is a string") );
        CPPUNIT_ASSERT( sd.b == jString("This is a string") );
        CPPUNIT_ASSERT( sd.c == jString("xxx"));
        CPPUNIT_ASSERT( sd.d == jString("This is a stringxxx")); // b+c
        CPPUNIT_ASSERT( sd.e == jString("is")); // from substr
        CPPUNIT_ASSERT( sd.f == jString("")); // from substr
        CPPUNIT_ASSERT( sd.g == jString("string")); // from right()
        CPPUNIT_ASSERT( sd.h == jString("This")); // from left()
    }
    void testAssign(){
        jString a2(sd.g);
        a2=sd.a;
        CPPUNIT_ASSERT(a2==sd.a);
    }
    void testcopyConstructor(){
        jString a2(sd.a);
        CPPUNIT_ASSERT(a2==sd.a);
    }
    // Check comparison operators
    void testEqualityCompare(){
        CPPUNIT_ASSERT(sd.a==sd.b);
    }
    // Check comparison operators
    void testEqualityCompareAgainstCString(){
        CPPUNIT_ASSERT(sd.c=="xxx");
    }
    void testLessCompare(){
        CPPUNIT_ASSERT(sd.d<sd.c);
    }
    void testLessEqualCompare(){
        CPPUNIT_ASSERT(sd.d<=sd.c);
        CPPUNIT_ASSERT(sd.a<=sd.b);
    }
    void testGreaterCompare(){
        CPPUNIT_ASSERT(sd.d>sd.a);
        CPPUNIT_ASSERT(sd.d>sd.b);
    }
    void testGreaterEqualCompare(){
        CPPUNIT_ASSERT(sd.d>=sd.a);
        CPPUNIT_ASSERT(sd.a>=sd.b);
    }
    void testFindFound(){
        // "This is a string" contains "is" at position 2
        CPPUNIT_ASSERT(sd.b.find(sd.e) == 2 );
    }
    void testFindNotFound(){
        // "This is a string" does not contain "xxx"
        CPPUNIT_ASSERT(sd.b.find(sd.c) == -1 );
    }
    void testJoin(){
        jString a=", ";
        const char * words[] = {"One", "Two", "Three"};
        jString b=a.join(words);
        CPPUNIT_ASSERT(b==jString("One, Two, Three"));
    }
    void testNoEqualCompare(){ 
        CPPUNIT_ASSERT(sd.d!=sd.a);
    }
    void teststring_view(){
#ifdef __cpp_lib_string_view
        std::string_view sv( sd.d );
        jString svs( sv );
        CPPUNIT_ASSERT(sd.d==svs);
#else
        CPPUNIT_FAIL("std::string_view not implemented, check compiler options.\n"
                     "    If using clang, try clang++");
#endif
    }
    
    CPPUNIT_TEST_SUITE(jStringTestClass);
        CPPUNIT_TEST(testDefaultCtorIsEmpty);
        CPPUNIT_TEST(testcopyConstructor);
        CPPUNIT_TEST(testInitialisers);
        CPPUNIT_TEST(testEqualityCompare);
        CPPUNIT_TEST(testLessCompare);
        CPPUNIT_TEST(testLessEqualCompare);
        CPPUNIT_TEST(testEqualityCompareAgainstCString);
        CPPUNIT_TEST(testGreaterCompare);
        CPPUNIT_TEST(testGreaterEqualCompare);
        CPPUNIT_TEST(testNoEqualCompare);
        CPPUNIT_TEST(testJoin);
        CPPUNIT_TEST(testFindFound);
        CPPUNIT_TEST(testFindNotFound);
        CPPUNIT_TEST(teststring_view);
    CPPUNIT_TEST_SUITE_END();


};

CPPUNIT_TEST_SUITE_REGISTRATION(jStringTestClass);

#ifdef ONE_FIXTURE
int main(int argc, char* argv[])
{
    // Get the top level suite from the registry
    CPPUNIT_NS::Test *suite = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();

    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);
    bool wasSucessful = runner.run();
    char* c = (char*) malloc(10 * sizeof (char));
    //scanf (c,"%c");
    return wasSucessful ? 0 : 1;
}
#endif
