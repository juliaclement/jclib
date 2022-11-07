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
 * File:   SaveTestClass.cpp
 * Author: Julia Clement <Julia at Clement dot nz>
 *
 * Created on 7/11/2022
 */

#ifdef ONE_FIXTURE
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#endif
#include <cppunit/extensions/HelperMacros.h>
#include "../jString.hpp"
#include "../Save.hpp"
using namespace jclib;

namespace jcArgsTestClassNS {
    typedef struct data {
        jString str_ = "";
        int int_ = 0;
        unsigned int unsigned_ = 0;
        bool bool_ = true;
    } struct_data;
};
using namespace jcArgsTestClassNS;

class SaveTestClass : public CPPUNIT_NS::TestFixture {
public:
    struct_data so;
    jString version_ = "jcargs v 0.0.0";
    jString help_prefix_ = "Help Text";
    jString help_suffix_ = "Help ends";
    std::ostringstream out_stream_;
    SaveTestClass() {}
    virtual ~SaveTestClass() {}
    void setUp(){
    }
    void tearDown(){}
private:
    void testBool(){
        bool b = true;
        if( b ) {
            auto svr = Save(b);
            b = false;
        }
        CPPUNIT_ASSERT(b);
    }
    void testInt(){
        int i = 12;
        if( i > 1 ) {
            auto svr = Save(i);
            i = 7;
        }
        CPPUNIT_ASSERT(i == 12);
    }
    void testStruct_bool(){
        struct_data so;
        if( so.bool_ ) {
            auto save_so = Save(so);
            so.bool_ = false;
        }
        CPPUNIT_ASSERT(so.bool_ == true);
    }
    void testStruct_int(){
        struct_data so;
        so.int_ = 12;
        if( so.bool_ ) {
            auto save_so = Save(so);
            so.int_ = 7;
        }
        CPPUNIT_ASSERT(so.int_ == 12);
    }
    /*** Test disable works */
    void testDisable(){
        struct_data so;
        so.int_ = 12;
        if( so.bool_ ) {
            auto save_so = Save(so);
            so.int_ = 7;
            save_so.disable();
        }
        CPPUNIT_ASSERT(so.int_ == 7);
    }
    /*** Test enable works */
    void testEnable(){
        struct_data so;
        so.int_ = 12;
        if( so.bool_ ) {
            auto save_so = Save(so);
            so.int_ = 5;
            // need to disable first to demonstrate
            // enable isn't just a no-op
            save_so.disable();
            so.int_ = 7;
            save_so.enable();
        }
        CPPUNIT_ASSERT(so.int_ == 12);
    }
    /*** Test restore works & destructor's restore still works*/
    void testRestore(){
        struct_data so;
        so.int_ = 12;
        if( so.bool_ ) {
            auto save_so = Save(so);
            so.int_ = 7;
            save_so.restore();
            CPPUNIT_ASSERT(so.int_ == 12);
            so.int_ = 8;
        }
        CPPUNIT_ASSERT(so.int_ == 12);
    }
    void testStruct_string(){
        struct_data so;
        so.str_ = "OK";
        if( so.bool_ ) {
            auto save_so = Save(so);
            so.str_ = "BAD";
        }
        CPPUNIT_ASSERT(so.str_ == "OK");
    }
    
    CPPUNIT_TEST_SUITE(SaveTestClass);
        CPPUNIT_TEST(testBool);
        CPPUNIT_TEST(testInt);
        CPPUNIT_TEST(testStruct_bool);
        CPPUNIT_TEST(testStruct_int);
        CPPUNIT_TEST(testStruct_string);
        CPPUNIT_TEST(testDisable);
        CPPUNIT_TEST(testEnable);
        CPPUNIT_TEST(testRestore);
    CPPUNIT_TEST_SUITE_END();


};

CPPUNIT_TEST_SUITE_REGISTRATION(SaveTestClass);

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
