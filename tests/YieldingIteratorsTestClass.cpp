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
 * File:   YieldingIteratorsTestClass.cpp
 * Author: Julia Clement <Julia at Clement dot nz>
 *
 * Created on 8/10/2017, 9:33:39 PM
 */

#include <string>
#include <cstring>
#include "../yieldingIterator.hpp"
#include <cppunit/extensions/HelperMacros.h>
using namespace std;
using namespace jclib;

class intSequence : public yieldingIteratorHelper<int> {
    int start_, limit_;
public:
    virtual void next( int & value, int & state ) {
        if( state == 0 ) {
            state = 1;
            value = start_;
        } else if( value < limit_ ) {
            ++value;
        } else {
            state = -1; // eof
        }
        return;
    }
    virtual void ending( int & value ) {
        value = -1;
    }
    intSequence( int start, int limit ) 
    : start_( start )
    , limit_( limit )
    {
    }
};

class piDigits : public yieldingIteratorHelper<int> {
public:
    virtual void next( int & value, int & state ) {
        YielderBegin;
            YieldReturn(3);
            YieldReturn(1);
            YieldReturn(4);
            YieldReturn(1);
            YieldReturn(5);
            YieldReturn(9);
        YielderEnd;
    }
};

class piChars : public yieldingIteratorHelper<char> {
private:
    const char * position_; 
public:
    // As there is class data we implement maybeClone.
    virtual piChars * maybeClone() { return new piChars(*this); }
    virtual void next( char & value, int & state ) {
        YielderBegin;
            for( position_ = "3.1415926535897932384";
                 * position_;
                 ++position_ )
                YieldReturn( *position_);
            YieldReturn('6');
            YeildingFinished;
            for( position_ = "IF THIS SHOWS, YEILDINGFINISHED IS BROKEN";
                 * position_;
                 ++position_ )
                YieldReturn( *position_);
        YielderEnd;
    }
};

/** Struct to test -> operator on iterators */    
struct maoTest {
    double d;
    int i;
};
class maoTestIteratorYielder : public yieldingIteratorHelper<struct maoTest> {
    double d_;
    int i_;
public:
    virtual void next( struct maoTest & value, int & state ) {
        YielderBegin;
            d_ = 1.0;
            for( i_ = 1; i_ < 4; ++ i_ ) {
                d_+=d_;
                struct maoTest answer;
                answer.d = d_;
                answer.i = i_;
                value = answer;
                YieldReturnAnswer();
            }
        YielderEnd;
        return;
    }
    virtual void ending( struct maoTest & value ) {
        value.i = -1;
    }
    virtual maoTestIteratorYielder * maybeClone() {
        return new maoTestIteratorYielder(*this);
    }
    maoTestIteratorYielder( ) 
    : i_( 0 )
    , d_( 0.0)
    {
    }
    maoTestIteratorYielder( const maoTestIteratorYielder & old) 
    : i_( old.i_ )
    , d_( old.d_ )
    {
    }
};

int vals[] = {1, 2, 4, 8}; 

class YieldingIteratorsTestClass : public CPPUNIT_NS::TestFixture {

public:
    YieldingIteratorsTestClass() {
    }
    virtual ~YieldingIteratorsTestClass() {
    }
    void setUp() {
    }
    void tearDown() {
    }

private:
    void testCharContainer() {
        // Test copy constructor of containers
        yieldingContainer<char> piCharsContainer2( new piChars  );
        std::string answer;
        for( auto i : piCharsContainer2 ) {
            answer.append( 1, i);
        }

        CPPUNIT_ASSERT(answer == "3.14159265358979323846");
    }
    void testContainerCopyCtor( ){
        // Test copy constructor of containers
        yieldingContainer<char> piCharsContainer( new piChars  );
        yieldingContainer<char> piCharsContainer2( piCharsContainer  );
        std::string answer;
        for( auto i : piCharsContainer2 ) {
            answer.append( 1, i);
        }

        CPPUNIT_ASSERT_EQUAL(answer, std::string("3.14159265358979323846"));
    }
    
    void testContainerAssignment( ){
        // Test assignment of containers
        yieldingContainer<char> piCharsContainer( nullptr );
        { // I want piCharsContainer2 destroyed before I invoke begin()
            yieldingContainer<char> piCharsContainer2( new piChars  );
            piCharsContainer = piCharsContainer2;
        }
        std::string answer;
        for( auto i : piCharsContainer ) {
            answer.append( 1, i);
        }

        CPPUNIT_ASSERT_EQUAL(answer, std::string("3.14159265358979323846"));
    }
    
    void testInitialValue( ){
        yieldingIterator<int> intSequenceIterator( new intSequence( 10, 15), 3);
        CPPUNIT_ASSERT_EQUAL(*intSequenceIterator, 10);
    }
    
    void testBeginValue( ){
        yieldingContainer<int> intSequenceContainer( new intSequence( 10, 15) );
        yieldingIterator<int> intSequenceIterator = intSequenceContainer.begin();
        CPPUNIT_ASSERT_EQUAL(*intSequenceIterator, 10);
    }
    
    void testPreIncrement( ){
        yieldingIterator<int> intSequenceIterator( new intSequence( 10, 15), 3);
        ++intSequenceIterator;
        int j = *++intSequenceIterator;
        CPPUNIT_ASSERT_EQUAL( j, 12 );
    }
    
    void testPostIncrement( ){
        yieldingIterator<int> intSequenceIterator( new intSequence( 10, 15), 3);
        intSequenceIterator++;
        int j = *(intSequenceIterator++);
        CPPUNIT_ASSERT_EQUAL( j, 11 );
    }
    
    void testSetBegin( ){
        yieldingIterator<int> intSequenceIterator( new intSequence( 10, 12), 3);
        ++intSequenceIterator; // 11
        ++intSequenceIterator; // 12
        CPPUNIT_ASSERT( ! intSequenceIterator.is_end());
        ++intSequenceIterator; // 13 would be after end
        CPPUNIT_ASSERT( intSequenceIterator.is_end());
        intSequenceIterator.set_begin();
        CPPUNIT_ASSERT( *intSequenceIterator == 10 );
    }
    
    void testSetBeginNoSequence( ){
        yieldingIterator<int> intSequenceIterator( nullptr );
        CPPUNIT_ASSERT( intSequenceIterator.is_end());
        intSequenceIterator.set_begin();
        CPPUNIT_ASSERT( intSequenceIterator.is_end());
    }
    
    void testSetEnd( ){
        yieldingIterator<int> intSequenceIterator( new intSequence( 10, 15), 3);
        CPPUNIT_ASSERT( ! intSequenceIterator.is_end());
        intSequenceIterator.set_end();
        CPPUNIT_ASSERT( intSequenceIterator.is_end());
        CPPUNIT_ASSERT( *intSequenceIterator == -1 );
        // CPPUNIT_ASSERT_EQUAL( j, 12 );
    }
    
    void testWholeSequence( ){
        yieldingContainer<int> seqContainer( new intSequence( 22, 24) );
        int expected = 22;
        for( auto i : seqContainer ) {
            CPPUNIT_ASSERT_EQUAL( i, expected++ );
        }
        CPPUNIT_ASSERT_EQUAL( expected, 25 );
    }
    
    void testMemberAccessOperator( ){
        yieldingIterator<maoTest> maoSequenceIterator( new maoTestIteratorYielder );
        CPPUNIT_ASSERT_EQUAL( maoSequenceIterator->i, 1 );
    }
    
    /**
     * Chect that assignment creates a new independent copy 
     */
    void testIteratorAssignment( ){
        yieldingIterator<maoTest> maoSequenceIterator( new maoTestIteratorYielder );
        ++maoSequenceIterator;
        CPPUNIT_ASSERT_EQUAL( maoSequenceIterator->i, 2 );
        yieldingIterator<maoTest> maoSequenceIterator2;
        maoSequenceIterator2 = maoSequenceIterator;
        ++maoSequenceIterator; // Does not change maoSequenceIterator2
        CPPUNIT_ASSERT_EQUAL( maoSequenceIterator->i, 3 );
        CPPUNIT_ASSERT_EQUAL( maoSequenceIterator2->i, 2 );
    }
    
    /**
     * Chect that copy constructor creates a new independent copy 
     */
    void testIteratorCopyCtor( ){
        yieldingIterator<maoTest> maoSequenceIterator( new maoTestIteratorYielder );
        ++maoSequenceIterator;
        CPPUNIT_ASSERT_EQUAL( maoSequenceIterator->i, 2 );
        yieldingIterator<maoTest> maoSequenceIterator2( maoSequenceIterator );
        ++maoSequenceIterator; // Does not change maoSequenceIterator2
        CPPUNIT_ASSERT_EQUAL( maoSequenceIterator->i, 3 );
        CPPUNIT_ASSERT_EQUAL( maoSequenceIterator2->i, 2 );
    }
    /*
    void test( ){
        yieldingIterator<int> intSequenceIterator( new intSequence( 10, 15), 3);
        yieldingContainer<int> intSequenceContainer( new intSequence( 10, 15) );
        // CPPUNIT_ASSERT_EQUAL( j, 12 );
    } */
    
    CPPUNIT_TEST_SUITE(YieldingIteratorsTestClass);

    CPPUNIT_TEST(testCharContainer);
    CPPUNIT_TEST(testContainerCopyCtor);
    CPPUNIT_TEST(testContainerAssignment);
    CPPUNIT_TEST(testInitialValue);
    CPPUNIT_TEST(testBeginValue);
    CPPUNIT_TEST(testPreIncrement);
    CPPUNIT_TEST(testPostIncrement);
    CPPUNIT_TEST(testSetBegin);
    CPPUNIT_TEST(testSetBeginNoSequence);
    CPPUNIT_TEST(testSetEnd);
    CPPUNIT_TEST(testWholeSequence);
    CPPUNIT_TEST(testMemberAccessOperator);
    CPPUNIT_TEST(testIteratorCopyCtor);
    CPPUNIT_TEST(testIteratorAssignment);
    //CPPUNIT_TEST_PARAMETERIZED(testMultiMethod,vals );

    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(YieldingIteratorsTestClass);
