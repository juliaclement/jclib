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
 * File:   countedPointer.hpp
 * Author: Julia Clement <Julia at Clement dot nz>
 *
 * Created on 1 October 2017, 9:46 PM
 */

#ifndef COUNTINGPOINTER_HPP
#define COUNTINGPOINTER_HPP

namespace jclib {
/**
 * This is an implementation of an intrusive counted pointed.
 * The typename T must expose two public methods CountedPointerAtttach and
 * CountedPointerDetach. The expected behaviour is that these will maintain
 * a count of attached pointers and self delete when the counter reaches zero 
**/
template< typename T> class CountedPointer {
private:
    T * ptr_;
    inline void postAssign() {
        if( ptr_ != nullptr )
            ptr_->CountedPointerAttach();
    }
public:
    /**
     * Test if the pointer is set to nullptr
     * @return true if the internal pointer is nullptr
    **/
    bool isnull() const { return ptr_ == nullptr; }
    /**
     * Test if the pointer is set to an object
     * @return true if the internal pointer is not nullptr
    **/
    bool isset() const { return ptr_ != nullptr; }
    
    /**
     * @return the address of the object pointed at
     */
    T * get() const { return const_cast< T *>(ptr_); }
    /**
     * retrieve a reference to the address of the object pointed at
     */
    T & operator *() const { return const_cast< T &>(*ptr_); }
    T * operator ->() const { return const_cast< T *>(ptr_); }
    /**
     * Set the internal value. Attaching / Detaching as requried
     * occasionaally useful as a replacement for the assignment operator.
     * @param ptr the new value to set
     */
    inline void set( T * const ptr) {
        // Note: We Attach before Detach so A=A (with attach count==1)
        // doesn't cause the attach count to temporarily zero and have
        // the target delete itself;
        T * tmp_ptr=ptr_; 
        ptr_ = const_cast< T*>(ptr);
        postAssign();
        if( tmp_ptr != nullptr ) {
            tmp_ptr->CountedPointerDetach(); 
        } 
    }
    /**
     * Set the internal value. Attaching / Detaching as requried
     * occasionaally useful as a replacement for the assignment operator.
     * @param ptr the new value to set
     */
    inline void set( const CountedPointer<T> & old ) {
        set( old.ptr_);
    }
    /**
     * Set the internal value. Attaching / Detaching as requried
     * @param ptr the new value to set
     * @return ourself
     */
    class CountedPointer<T> & operator =( T * const ptr ) {
        set( ptr );
        return *this;
    }
    /**
     * Set the internal value. Attaching / Detaching as requried
     * @param ptr the new value to set
     * @return ourself
     */
    class CountedPointer<T> & operator =( const CountedPointer<T> & ptr ) { 
        set( ptr.ptr_ );
        return *this;
    }
    CountedPointer( ) : ptr_( nullptr) { postAssign(); }
    CountedPointer( T * ptr ) : ptr_( ptr) { postAssign(); }
    CountedPointer( const class CountedPointer<T>& old ) : ptr_( old.ptr_) { postAssign(); }
    ~CountedPointer() { if( ptr_ != nullptr ) ptr_->CountedPointerDetach(); ptr_=nullptr; }
};

/**
 * A simple class that satisfies requirements for targets of CountedPointer
 * 
 */

class CountedPointerTarget {
    private:
        int counter_;
    public:
        inline void CountedPointerAttach() { ++counter_; }
        inline void CountedPointerDetach() { if( !--counter_) delete this;}
        inline int  CountedPointersAttached() const { return counter_; }
        CountedPointerTarget() : counter_(0) {}
        CountedPointerTarget(const CountedPointerTarget &) : counter_(0) {}
        // virtual destructor to ensure derrived classes are destroyed properly
        virtual ~CountedPointerTarget() {}
};

/**
 * A template child class that permits automatic & static variables to have 
 * counted pointers.
 * 
 * The user must ensure that all CountedPointers to instances of this class
 * are deleted before the target.
 * 
 * NB: It is not generally safe to have CountedPointers point at automatic or 
 *     static variables as when the pointer count reaches zero an attempt will
 *     be made to delete the variable.
 */
template<typename T> class CountedPointerSafe : public T {
    public:
    template< typename ... Types > CountedPointerSafe( const Types&...args )
    : T(args...)
    { T::CountedPointerAttach(); }
};

};

#endif /* COUNTINGPOINTER_HPP */

