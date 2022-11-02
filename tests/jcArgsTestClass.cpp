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
 * File:   jcArgsTestClass.cpp
 * Author: Julia Clement <Julia at Clement dot nz>
 *
 * Created on 31/10/2022, 18:05:55
 */

// #include "cstdafx"
#ifdef ONE_FIXTURE
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#endif
#include <cppunit/extensions/HelperMacros.h>
#include "../jString.hpp"
#include "../jcargs.hpp"
using namespace jclib;

namespace jcArgsTestClassNS {
    typedef struct options_ {
        jString str_ = "";
        int int_ = 0;
        unsigned int unsigned_ = 0;
        bool bool_ = false;
    } struct_options;
};
using namespace jcArgsTestClassNS;

class jcArgsTestClass : public CPPUNIT_NS::TestFixture {
public:
    struct_options so;
    jString version_ = "jcargs v 0.0.0";
    jString help_prefix_ = "Help Text";
    jString help_suffix_ = "Help ends";
    std::ostringstream out_stream_;
    jcArgsTestClass() {}
    virtual ~jcArgsTestClass() {}
    void setUp(){
    }
    void tearDown(){}
private:
    void standard_args( arguments & args ) {
        args.load({
            arg(so.str_,"s","string","String Arg",true,false),
            arg(so.bool_,"b","bool","Bool Arg",false,false),
            arg(so.int_,"i","int","Int Arg",true,false),
            arg(so.unsigned_,"u","unsigned","Unsigned Arg",true,false),
            arg(args.show_help_,"h", "help", "Show help",false,false),
            arg(args.show_version_,"v","version", "Show version info", false, false)
        });
        args.version_string_ = version_;
        args.help_prefix_ = help_prefix_;
        args.help_suffix_ = help_suffix_;
    }
    void testStringShortArgSeparate(){
        arguments args(out_stream_);
        standard_args( args );
        const char * argv[] = {"X","-s","String",nullptr};
        CPPUNIT_ASSERT(args.process_args(3,argv));
        CPPUNIT_ASSERT(so.str_ == jString("String"));
        CPPUNIT_ASSERT(out_stream_.str().length() == 0);
    }
    void testStringShortArgJoined(){
        arguments args(out_stream_);
        standard_args( args );
        const char * argv[] = {"X","-sString",nullptr};
        CPPUNIT_ASSERT(args.process_args(2,argv));
        CPPUNIT_ASSERT(so.str_ == jString("String"));
        CPPUNIT_ASSERT(out_stream_.str().length() == 0);
    }
    void testBoolShort(){
        arguments args(out_stream_);
        standard_args( args );
        const char * argv[] = {"X","-b",nullptr};
        CPPUNIT_ASSERT(args.process_args(2,argv));
        CPPUNIT_ASSERT(so.bool_);
        CPPUNIT_ASSERT(out_stream_.str().length() == 0);
    }
    void testBoolLong(){
        arguments args(out_stream_);
        standard_args( args );
        const char * argv[] = {"X","--bool",nullptr};
        CPPUNIT_ASSERT(args.process_args(2,argv));
        CPPUNIT_ASSERT(so.bool_);
        CPPUNIT_ASSERT(out_stream_.str().length() == 0);
    }
    void testIntShort(){
        arguments args(out_stream_);
        standard_args( args );
        const char * argv[] = {"X","-i12",nullptr};
        CPPUNIT_ASSERT(args.process_args(2,argv));
        CPPUNIT_ASSERT(so.int_ == 12 );
        CPPUNIT_ASSERT(out_stream_.str().length() == 0);
    }
    void testIntLongEq(){
        arguments args(out_stream_);
        standard_args( args );
        const char * argv[] = {"X","--int=21",nullptr};
        CPPUNIT_ASSERT(args.process_args(2,argv)); 
        CPPUNIT_ASSERT(so.int_==21);
        CPPUNIT_ASSERT(out_stream_.str().length() == 0);
    }
    void testIntLongSep(){
        arguments args(out_stream_);
        standard_args( args );
        const char * argv[] = {"X","--int","2112",nullptr};
        CPPUNIT_ASSERT(args.process_args(3,argv));
        CPPUNIT_ASSERT(so.int_==2112);
        CPPUNIT_ASSERT(out_stream_.str().length() == 0);
    }
    void testVersion(){
        arguments args(out_stream_);
        standard_args( args );
        const char * argv[] = {"X","-v",nullptr};
        CPPUNIT_ASSERT(args.process_args(2,argv) == false); // False because version_ set
        CPPUNIT_ASSERT(args.show_version_);
        CPPUNIT_ASSERT(jString(out_stream_.str()).left(version_.len())==version_ );
    }
    void testHelpPrefix(){
        arguments args(out_stream_);
        standard_args( args );
        const char * argv[] = {"X","-h",nullptr};
        CPPUNIT_ASSERT(args.process_args(2,argv) == false); // False because version_ set
        CPPUNIT_ASSERT(args.show_help_);
        CPPUNIT_ASSERT(jString(out_stream_.str()).find(help_prefix_) == 0 );
    }
    void testHelpOption(){
        arguments args(out_stream_);
        standard_args( args );
        const char * argv[] = {"X","-h",nullptr};
        CPPUNIT_ASSERT(args.process_args(2,argv) == false); // False because version_ set
        CPPUNIT_ASSERT(args.show_help_);
        auto stream_string= out_stream_.str();
        jString help_text = jString(stream_string.c_str());
        CPPUNIT_ASSERT(help_text.find("unsigned") > 0 );
        CPPUNIT_ASSERT(help_text.find("Unsigned Arg") > 0 );
    }
    void testHelpSuffix(){
        arguments args(out_stream_);
        standard_args( args );
        const char * argv[] = {"X","--help",nullptr};
        CPPUNIT_ASSERT(args.process_args(2,argv) == false); // False because version_ set
        CPPUNIT_ASSERT(args.show_help_);
        CPPUNIT_ASSERT(jString(out_stream_.str()).find(help_suffix_) > 0 );
    }
    
    CPPUNIT_TEST_SUITE(jcArgsTestClass);
        CPPUNIT_TEST(testStringShortArgSeparate);
        CPPUNIT_TEST(testStringShortArgJoined);
        CPPUNIT_TEST(testBoolShort);
        CPPUNIT_TEST(testBoolLong);
        CPPUNIT_TEST(testIntShort);
        CPPUNIT_TEST(testIntLongEq);
        CPPUNIT_TEST(testIntLongSep);
        CPPUNIT_TEST(testVersion);
        CPPUNIT_TEST(testHelpPrefix);
        CPPUNIT_TEST(testHelpOption);
        CPPUNIT_TEST(testHelpSuffix);
    CPPUNIT_TEST_SUITE_END();


};

CPPUNIT_TEST_SUITE_REGISTRATION(jcArgsTestClass);

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
