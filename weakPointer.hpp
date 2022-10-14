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
 * File:   weakPointer.hpp
 * Author: Julia Clement <Julia at Clement dot nz>
 *
 * Created on 5 October 2017, 7:07 PM
 */

#ifndef WEAKPOINTER_HPP
#define WEAKPOINTER_HPP

/**
 * Implementation of a weak pointer. 
 * jclib::WeakPointer is a smart pointer that holds a non-owning ("weak") 
 * reference to an object that is managed by jclib::CountedPointer. 
 * like std::weak_ptr, jclib::WeakPointer models temporary ownership: 
 * when an object needs to be available only if it exists, and it may be 
 * deleted at any time by other code.
 * Like std::weak_ptr, jclib::WeakPointer is intended to break circular 
 * references in pointers..

 * This is not the same as std::weak_ptr, details on that class are at
 *  http://en.cppreference.com/w/cpp/memory/weak_ptr
 * Major differences: 
 * * std::weak_ptr is related to std::shared pointer, jclib::WeakPointer is
 *   related to jclib::CountedPointer
 * * std::weak_pointer may hold references to any object std::shared_pointer
 *   may hold references to. jclib::WeakPointer may only hold references
 *   to objects that publicly inherit from jclib::WeakPointerTargetBase. Usually
 *   something that publicly inherits from WeakPointerTarget<T>
 * * jclib::WeakPointer may hold a nullptr or a value returned from
 *   WeakPointerTarget<T>::getWeakPointer()
 * * std::weak_ptr must be converted to std::shared_ptr in order to access the
 *   referenced object. jclib::WeakPointer may be directly dereferenced to a 
 *   native pointer. That said, unless the pointer is very short lived, 
 *   converting to a CountedPointer is usually safer.
 * Todo: Consider if WeakPointerTargetBase should be an interface
 */

#include "countedPointer.hpp"

namespace jclib {
class WeakPointerTargetBase;

/**
 * All jclib::WeakPointers are CountedPointers of this class.
 * It is created, initialised and ultimately cleared by the target
 */
class WeakPointerBase {
private:
    int countedPointers_;
public:
    WeakPointerTargetBase * wtb_target_;
    
    inline void CountedPointerAttach() { ++countedPointers_; }
    inline void CountedPointerDetach() { if( !--countedPointers_ ) delete this;}
    WeakPointerBase( WeakPointerTargetBase * target) : countedPointers_( 0 ), wtb_target_(target) {}
    // copy constructor. Not required, but included as failsafe 
    WeakPointerBase( WeakPointerBase const &) : countedPointers_( 0 ) {}
};

/**
 */
template< typename T> class WeakPointer {
    public:
    typedef CountedPointer< class WeakPointerBase > pointer_t;
    private:
        pointer_t basePointer_;
    public:
    /**
     * Test if the pointer is set to nullptr
     * @return true if either the internal pointer or the WeakPointerBase is nullptr
    **/
    bool isnull() const {
        return basePointer_.isnull() || 
               basePointer_->wtb_target_ == nullptr;
    }
    /**
     * Test if the pointer is set to an object
     * * @return true if neither the internal pointer or the WeakPointerBase is nullptr
    **/
    bool isset() const {
        return  basePointer_.isset() && 
                basePointer_->wtb_target_ != nullptr;
    }
    
    /**
     * Obtain a native C++ pointer to the object
     * @return the address of the object pointed at
     */
    T * get() const {
        return 
            isnull()
            ? nullptr 
            : static_cast<T*> ( const_cast<class WeakPointerTargetBase *>(basePointer_->wtb_target_) ); } //const_cast< T *>((T*)(basePointer_.get())); }
    /**
     * retrieve a reference to the address of the object pointed at
     */
    T & operator *() const { 
        return *get();//* const_cast< T *>((T*)(basePointer_->target_));
    }
    /**
     * retrieve a reference to the address of the object pointed at
     */
    T * operator ->() const { return get(); } //return const_cast< T *>((T*)(basePointer_->target_)); }
    /**
     * Set to point to a different target. Attaching / Detaching as requried
     * occasionaally useful as a replacement for the assignment operator.
     * @param ptr the new value to set
     */
    inline void set( const WeakPointer<T> & old) {
       basePointer_.set(old.basePointer_); 
    }
    /**
     * Set the internal value. Attaching / Detaching as requried
     * @param ptr the new value to set
     * @return ourself
     */
    class WeakPointer<T> & operator =( const WeakPointer<T> & ptr ) { 
        basePointer_ = ptr.basePointer_;
        return *this;
    }
    WeakPointer( )
    : basePointer_( nullptr ) { }
    WeakPointer( const CountedPointer< class WeakPointerBase > & base )
    : basePointer_( base ) { }
    /*
    WeakPointer( class WeakPointerBase * base )
    : basePointer_( base ) {
    }
     */
    WeakPointer( const class WeakPointer<T>& old )
    : basePointer_( old.basePointer_) {
    }
    virtual ~WeakPointer( ) {
        basePointer_ = nullptr; }
};

/**
 * 
 */
class WeakPointerTargetBase {
public:
    typedef CountedPointer< class WeakPointerBase > pointer_t;
private:
    pointer_t basePointer_;
protected:
    class WeakPointerBase * getBasePointer() const {
        return (const_cast< WeakPointerTargetBase *>(this)->basePointer_).get();
    }
    WeakPointerTargetBase( )
    : basePointer_ ( new WeakPointerBase( this ) ) 
    {
    }
    /*
    WeakPointerTargetBase( void * container )
    : basePointer_( new WeakPointerBase( container ) )
    {
    }
     */
    /**
     * Copy constructor. This should only be used when the owning 
     * class single inherits from WeakPointerTargetBase
     * @param ignored as we create our own pointer_t we don't use 
     *        any data from the old object 
     */
    WeakPointerTargetBase( WeakPointerTargetBase const & ignored )
    : basePointer_( new WeakPointerBase( this ) )
    {        
    }
    virtual ~WeakPointerTargetBase( ) {
        // reset the pointer 
        basePointer_->wtb_target_ = nullptr;
    }
};

/**
 * All targets of weak pointers should inherit from this template class
 * using something like:
 * class TargetClass1 : public WeakPointerTarget<TargetClass0> ...
 */
template< typename T > class WeakPointerTarget : public WeakPointerTargetBase {
    public:
        WeakPointer<T> getWeakPointer() const {
            class WeakPointerBase * answer = getBasePointer();
            return WeakPointer<T>( answer );
        }
};
}
#endif /* WEAKPOINTER_HPP */

