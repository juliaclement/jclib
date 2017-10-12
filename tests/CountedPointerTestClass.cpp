/*
Copyright (c) 2017 Julia Ingleby Clement

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
 * File:   CountedPointerTestClass.cpp
 * Author: Julia Clement <Julia at Clement dot nz>
 *
 * Created on 10/10/2017, 7:30:55 AM
 */

#include <cppunit/extensions/HelperMacros.h>
#include "../countedPointer.hpp"
#include "../weakPointer.hpp"
using namespace jclib;


/**
 * A fairly simple class to test class CountedPointer
 */
class CountedPointerTestData {
private:
    /**
     * used by CountedPointer Attach / Detach. 
     */
    int counter_;
public:
    /**
     * What is the expected value of counter_ at destruct time
     * defaults to zero.
     */
    int expected_counter_;
    /**
     * Just a dummy value
     */
    int val_;
    double dval_;
    void CountedPointerAttach() { ++ counter_; }
    void CountedPointerDetach() { if (!--counter_) delete this; }
    inline int  CountedPointersAttached() const { return counter_; }
    CountedPointerTestData(int v = 0, double d = 0.0 )
            : counter_( 0 )
            , expected_counter_(0)
            , val_( v ), dval_( d ) {}
    CountedPointerTestData( const CountedPointerTestData & old) 
    : counter_(0)
    , expected_counter_( 0 ) // ???
    , val_( old.val_ )
    , dval_( old.dval_ )
    {}
    ~CountedPointerTestData() {
        CPPUNIT_ASSERT( counter_ == expected_counter_);
    }

    int GetCounter() const {
        return counter_;
    }
};

class WeakPointerTestData : public WeakPointerTarget< class WeakPointerTestData > {
    private:
        int counter_;
    public:
        inline void CountedPointerAttach() { ++counter_; }
        inline void CountedPointerDetach() { if( !--counter_) delete this;}
        inline int  CountedPointersAttached() const { return counter_; }
        
        int value_;
        WeakPointerTestData(int value = 0) : counter_(0), value_(value) {}
        WeakPointerTestData(const WeakPointerTestData & old) : counter_(0), value_(old.value_) {}
        virtual ~WeakPointerTestData() {}
};

class CountedPointerTestClass : public CPPUNIT_NS::TestFixture {
public:
    CountedPointerTestClass() {}
    virtual ~CountedPointerTestClass() {}
    void setUp(){}
    void tearDown(){}
private:
    void testDefaultCtorIsNull() {
        CountedPointer<CountedPointerTestData> pint;
        CPPUNIT_ASSERT(pint.isnull());
    }
    void testCtorIsNonNull() {
        CountedPointer<CountedPointerTestData> ptr = new CountedPointerTestData;
        CPPUNIT_ASSERT( !ptr.isnull());
    }
    void testCopyCtor() {
        CountedPointer<CountedPointerTestData> ptr = new CountedPointerTestData;
        ptr->val_ = 7;
        ptr = new CountedPointerTestData( *ptr);
        CPPUNIT_ASSERT( ptr->val_ == 7);
    }
    void testAssignmentToNull() {
        CountedPointer<CountedPointerTestData> ptr = new CountedPointerTestData;
        ptr = nullptr;
        CPPUNIT_ASSERT( ptr.isnull());
    }
    void testCountedPtrSafeInitialisesCounter() {
        CountedPointerSafe<CountedPointerTestData> auto_TCP1;
        CPPUNIT_ASSERT( auto_TCP1.CountedPointersAttached() == 1);
        auto_TCP1.expected_counter_ = 1; 
        CountedPointer<CountedPointerTestData> ptr = &auto_TCP1;
        CPPUNIT_ASSERT( auto_TCP1.CountedPointersAttached() == 2);
    }
    void testWeakPointerDefaultCtorNull() {
        WeakPointer<WeakPointerTestData> a1;
        CPPUNIT_ASSERT( a1.isnull() );
    }
    void testWeakPointerInitialise() {
        WeakPointerTestData * a=new WeakPointerTestData( 123456 );
        delete a;
    }
    
    void testWeakPointerDereference() {
        WeakPointerTestData *a=new WeakPointerTestData( 123456 );
        WeakPointer<WeakPointerTestData> a1 = a->getWeakPointer();

        CPPUNIT_ASSERT( a1->value_ == 123456 );
        delete a;
    }
    
    void testWeakPointerCleared() {
        WeakPointerTestData * a=new WeakPointerTestData( 123456 );
        WeakPointer<WeakPointerTestData> a1 =a->getWeakPointer();
        delete a;
        CPPUNIT_ASSERT( a1.isnull() );
    }
    
    void testWeakPointerCopyCtorNonNull() {
        WeakPointerTestData * a=new WeakPointerTestData( 123456 );
        WeakPointer<WeakPointerTestData> a1 =a->getWeakPointer();

        WeakPointer<WeakPointerTestData> b1 = a1;
        CPPUNIT_ASSERT( b1.isset() );
        CPPUNIT_ASSERT( b1->value_ == 123456 );
        delete a;
    }
    
    void testWeakPointerCopyCtorNullFromNull() {
        WeakPointer<WeakPointerTestData> a1;
        WeakPointer<WeakPointerTestData> b1 = a1;
        CPPUNIT_ASSERT( b1.isnull() );
    }
    
    void testWeakPointerGet() {
        WeakPointerTestData *a=new WeakPointerTestData( 123456 );
        WeakPointer<WeakPointerTestData> a1(a->getWeakPointer());
        CPPUNIT_ASSERT( a1.isset());
        CPPUNIT_ASSERT( a1.get() == a);
        delete a;
    }
    
    void testWeakOperatorStar() {
        WeakPointerTestData * a=new WeakPointerTestData( 123456 );
        WeakPointer<WeakPointerTestData> a1((*a).getWeakPointer());
        CPPUNIT_ASSERT( &(*a1) == a);
        delete a;
    }
    
    void testWeakPointerAssignment() {
        WeakPointerTestData * a=new WeakPointerTestData( 123456 );
        WeakPointerTestData * b=new WeakPointerTestData( 654321 );
        WeakPointer<WeakPointerTestData> a1 =a->getWeakPointer();

        WeakPointer<WeakPointerTestData> b1;
        CPPUNIT_ASSERT( b1.isnull() );
        b1 = b->getWeakPointer();
        CPPUNIT_ASSERT( b1.isset() );
        CPPUNIT_ASSERT( b1->value_ == 654321 );
        a1=b1;
        CPPUNIT_ASSERT( a1.isset() );
        CPPUNIT_ASSERT( a1->value_ == 654321 );
        // Check both weak pointers are to the same object
        b1->value_ = 975310;
        CPPUNIT_ASSERT( a1->value_ == 975310 );
        delete b;
        CPPUNIT_ASSERT( a1.isnull());
        CPPUNIT_ASSERT( b1.isnull());
    }
    
    void testWeakPointerSet() {
        WeakPointerTestData * a=new WeakPointerTestData( 123456 );
        WeakPointerTestData * b=new WeakPointerTestData( 654321 );
        WeakPointer<WeakPointerTestData> a1 =a->getWeakPointer();

        WeakPointer<WeakPointerTestData> b1;
        CPPUNIT_ASSERT( b1.isnull() );
        b1 = b->getWeakPointer();
        CPPUNIT_ASSERT( b1.isset() );
        CPPUNIT_ASSERT( b1->value_ == 654321 );
        a1.set(b1);
        CPPUNIT_ASSERT( a1.isset() );
        CPPUNIT_ASSERT( a1->value_ == 654321 );
    }
    void testWeakPointerTargetCopyCtor(){
        WeakPointerTestData * a=new WeakPointerTestData( 123456 );
        WeakPointerTestData * b=new WeakPointerTestData( *a );
        WeakPointer<WeakPointerTestData> a1 =a->getWeakPointer();
        WeakPointer<WeakPointerTestData> b1 =b->getWeakPointer();
        a1->value_ = 765432;
        CPPUNIT_ASSERT( b1->value_ == 123456 );
        delete a;
        delete b;
    }
    
    /*
     
     */
    
    CPPUNIT_TEST_SUITE(CountedPointerTestClass);

    CPPUNIT_TEST(testDefaultCtorIsNull);
    CPPUNIT_TEST(testCtorIsNonNull);
    CPPUNIT_TEST(testCopyCtor);
    CPPUNIT_TEST(testAssignmentToNull);
    CPPUNIT_TEST(testCountedPtrSafeInitialisesCounter);
    CPPUNIT_TEST(testWeakPointerDefaultCtorNull);
    CPPUNIT_TEST(testWeakPointerCopyCtorNonNull);
    CPPUNIT_TEST(testWeakPointerCopyCtorNullFromNull);
    CPPUNIT_TEST(testWeakPointerGet);
    CPPUNIT_TEST(testWeakOperatorStar);
    CPPUNIT_TEST(testWeakPointerInitialise);
    CPPUNIT_TEST(testWeakPointerDereference);
    CPPUNIT_TEST(testWeakPointerCleared);
    CPPUNIT_TEST(testWeakPointerAssignment);
    CPPUNIT_TEST(testWeakPointerTargetCopyCtor);

    CPPUNIT_TEST_SUITE_END();


};

CPPUNIT_TEST_SUITE_REGISTRATION(CountedPointerTestClass);

