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
        CPPUNIT_ASSERT( sd.e == jString("is"));
        CPPUNIT_ASSERT( sd.f == jString(""));
        CPPUNIT_ASSERT( sd.g == jString("string"));
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
    void testNoEqualCompare(){
        CPPUNIT_ASSERT(sd.d!=sd.a);
    }
    
    CPPUNIT_TEST_SUITE(jStringTestClass);
        CPPUNIT_TEST(testDefaultCtorIsEmpty);
        CPPUNIT_TEST(testcopyConstructor);
        CPPUNIT_TEST(testInitialisers);
        CPPUNIT_TEST(testEqualityCompare);
        CPPUNIT_TEST(testLessCompare);
        CPPUNIT_TEST(testLessEqualCompare);
        CPPUNIT_TEST(testGreaterCompare);
        CPPUNIT_TEST(testGreaterEqualCompare);
        CPPUNIT_TEST(testNoEqualCompare);
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
