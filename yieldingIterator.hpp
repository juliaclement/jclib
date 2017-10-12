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
 * File:   yieldingIterator.hpp
 * Author: Julia Clement <Julia at Clement dot nz>
 *
 * Created on 6 October 2017, 12:50 PM
 */

#ifndef YIELDINGITERATOR_HPP
#define YIELDINGITERATOR_HPP

#include <iterator>
#include "countedPointer.hpp"

namespace jclib {
#   define YielderBegin { switch(state) { do {default: throw (int)-314159; case 0:; } while(0)
    /**
     * Copy retval to value then return.
     * Processing resumes with the next statement on next call
     */
#   define YieldReturn(retvalue) do { value = (retvalue); state = __LINE__; return; case __LINE__:; } while(0)
    /**
     * Yield Return with the value already set in parameter value
     * Processing resumes with the next statement on next call
     */
#   define YieldReturnAnswer() do { state = __LINE__; return; case __LINE__:; } while(0)
#   define YeildingFinished do { state = -1; return; } while(0)
#   define YielderEnd } YeildingFinished; }
    template<typename T> class yieldingIteratorHelper : public CountedPointerTarget {
        public: 
            /**
             * Called to advance the iterator to its next state
             * @param value that will be returned by * iterator. I/O
             * @param state used by helper to determine where it is. Setting
             *        state to -1 signals end() is true
             */
            virtual void next( T & value, int & state ) = 0;
            /**
             * Called when iterator is created to set initial state
             * Optional: Default action is to call next() with state = 0;
             * @param value that will be returned by * iterator. I/O
             * @param state used by helper to determine where it is. 
             * There are two special values:<ul><li>-1: End of range encountered</li>
             * <li>0 beginning of sequence required. Either the iterator is
             * new or has been reset</li></ul>
             */
            virtual void begin( T & value, int & state ) {
                state = 0;
                next( value, state );
            }
            /**
             * Called when state is set to -1 (end) either by the user
             * or on return from next()
             * @param value that will be returned by * iterator. I/O
             */
            virtual void ending( T & value ) {
            }
            /**
             * Called by the yieldingIterator copy constructor. If 
             * this object has state stored outside the value and state
             * parameters to next(), it should return a new object, otherwise
             * it's fine to return ourselves
             * @return either ourself or a clone of ourself
             */
            virtual yieldingIteratorHelper<T> * maybeClone() { return this; }
    };
    
    /**
     * This is the user accessible class that uses a helper to provide 
     * a set of calculated values.
     */
    template<typename T> class yieldingIterator
                         : public std::iterator<std::forward_iterator_tag,
                                           T,
                                           ptrdiff_t,
                                           T*,
                                           T&> {
        public:
            enum { yiEof = -1, yiBegin = 0 };
        protected:
            // don't change the order of these members, 
            // helper_ must be constructed before state_
            CountedPointer< yieldingIteratorHelper<T> > helper_;
            int state_;
            T value_;
        public:
            yieldingIterator<T> operator++(int postfix) {
                yieldingIterator<T> answer = *this;
                helper_->next(value_, state_);
                return answer;
            }
            yieldingIterator<T> & operator++() {
                helper_->next(value_, state_);
                if( state_ == yiEof)
                    helper_->ending(value_);
                return *this;
            }
            T & operator*() const { return const_cast<T&>(value_); }
            T * operator->() const { return &const_cast<T&>(value_); }
            
            /**
             * Reset the iterator to the beginning of its sequence
             * requires the helper to act appropriately to the begin() call
             */
            void set_begin() {  
                if( helper_.isset()) {
                    state_ = yiBegin;
                    helper_->begin(value_, state_);
                } else {
                    state_ = yiEof;
                }
            }
            
            /**
             * Checks if the iterator is at the end of its range<br/>
             * Quick test for when container.end() isn't accessible.<br/>
             * Once end is set, helper_->next() won't be called until the
             * iterator is reset by a call to set_begin() or an assignment.
             * @return boolean
             */
            inline bool is_end() const { return state_ == yiEof; }

            /**
             * Sets the internal state to end of sequence<br/>
             * Once end is set, helper_->next() won't be called until the
             * iterator is reset by a call to set_begin() or an assignment.
             */
            void set_end() {
                state_ = yiEof;
                if( helper_.isset())
                    helper_->ending( value_ );
            }
            
            inline bool operator==( const yieldingIterator & rhs ) const {
                return state_ == rhs.state_ && 
                      (state_ < 1 || value_ == rhs.value_);
            }
            
            inline bool operator!=( const yieldingIterator & rhs ) const {
                return ! ( * this == rhs );
            }
            
            /** Variadic constructor relies on the helper being able to 
             * accept const & arguments in all positions. If this ever turns
             * out to be not so we may need to subclass or even split this class
             * 
             * @param helper 
             * @param args for the constructor of type T
             */
            template< typename ... Types >
                yieldingIterator( class yieldingIteratorHelper<T> * helper, 
                                  const Types&...args )
                : helper_( helper )
                , state_( yiBegin)
                , value_(args...)
                {
                    if( helper_.isset() ) {
                        helper_->begin( value_, state_);
                    } else {
                        state_ = yiEof;
                        // No helper so can't call ending())
                    }
                }
                
            /**
             * Constructor to satisfy the default constructable requirement 
             * for iterators.
             */
            yieldingIterator( )
            : helper_(  nullptr )
            , state_( yiEof )
            , value_( )
            {
            }
            
            /**
             * Copy constructor. NB: If the source iterator has ended, the
             * ending() method will be called. This means it is possible for
             * an iterator that never had a begin() to end().
             * @param old cource of copy.
             */
            yieldingIterator( const yieldingIterator<T> & old )
            : helper_( old.helper_.isnull() ? nullptr : old.helper_->maybeClone() )
            , state_( old.helper_.isnull() ? yiEof : old.state_ )
            , value_( old.value_)
            {
                if( helper_.isset() && state_ == yiEof )
                    helper_->ending(value_);
            }
            
            /**
             * Assignment operator. NB: If the source iterator has ended, the
             * ending() method will be called. This means it is possible for
             * an iterator that never had a begin() to end().
             * @param old cource of copy.
             */
            yieldingIterator<T> & operator=( const yieldingIterator<T> & old ) {
                helper_ = old.helper_.isnull() ? nullptr : old.helper_->maybeClone();
                state_ = old.helper_.isnull() ? yiEof : old.state_;
                value_ = old.value_;
                if( helper_.isset() && state_ == yiEof )
                    helper_->ending(value_);
                return *this;
            }
    };
    
    /**
     * Emulates a container object providing begin & end iterators
     * suitable for feeding to for( var : container )
     */
    template<typename T> class yieldingContainer {
        public:
            CountedPointer< yieldingIteratorHelper< T > > helper_;
            typedef class yieldingIterator<T> iterator_T;
        public:
            /**
             * instantiate and return an iterator set to the start of the range
             * @return the iterator
             */
            iterator_T begin() {
                iterator_T answer( helper_.isnull() ? nullptr : helper_->maybeClone() );
                answer.set_begin();
                return answer;
            }
            /**
             * return an iterator set to the end of the range. Only one iterator
             * per class is actually created
             * @return the iterator
             */
            iterator_T const & end() {
                // Being able to use a static end_ relies on the 
                // == operator considering only state_ when it is yiEof.
                static iterator_T end_(nullptr);
                return end_;
            }
            /**
             * Constructor<br/>
             * I have deliberately not created a default constructor.<br/>
             * Given that instances can be constructed with a nullptr (and a
             * test exercises that) there is no functional reason for not
             * having one, I simply can't see a purpose for it. JC Oct 2017
             * @param helper. If a nullptr, an assignment must be made before
             * calling begin() or bad things will happen
             */
            yieldingContainer ( yieldingIteratorHelper< T > * helper )
            : helper_( helper )
            {
            }
            
            
            /**
             * Copy constructor.
             * @param source object
             */
            yieldingContainer ( const yieldingContainer< T > & old )
             // I'm not sure if a maybeClone is required I believe not as it
             // should happen for every iterator created through begin() 
            : helper_( old.helper_ )
            {}
            
            virtual ~yieldingContainer() {
            }
            
            /**
             * Assignment.
             * @param source object
             */
            yieldingContainer<T> & operator = ( const yieldingContainer< T > & old ) {
             // see note on maybeClone in the copy constructor
                helper_ = old.helper_;
                return *this;
            }
    };
}


#endif /* YIELDINGITERATOR_HPP */

