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
 * File:   Save.hpp
 * Author: Julia Clement <Julia at Clement dot nz>
 * 
 * Purpose: Save<T> is a template class that stores a copy of a
 *          T variable's data and restores it when the Save<T>
 *          is destroyed.
 *          T must be copy constructable & assignable.
 * 
 * Part of the JCLib project https://github.com/juliaclement/jclib
 *
 * Created on 7 November 2022
 */
#ifndef Save_HPP
#define Save_HPP

namespace jclib {
    namespace Private {
        template<typename T> class Saver {
            private:
                T & saved_object_;
                T saved_data_;
                /*** Occasionally we need to disable the restore */
                bool enabled_ = true;
                // forbid copying
                Saver( const Saver & old  ) = delete;
                Saver & operator =( const Saver & old ) = delete;
            public:
                Saver(T& savee )
                : saved_object_( savee )
                , saved_data_( savee )
                {}
                ~Saver( ) {
                    if( enabled_ )
                        saved_object_ = saved_data_;
                }
                /*** restore data ahead of destruction. 
                 *   allows multiple restores of the 
                 *   original data, effectively creating a checkpoint.
                */
                Saver &  restore( ) {
                    saved_object_ = saved_data_;
                    return * this;
                }
                /*** Turn off the automatic restore
                 *   Typically used after a manual call to restore()
                */
                Saver & disable() {
                    enabled_ = false;
                    return * this;
                }
                /*** Turn on the automatic restore 
                 *   Only required following a disable()
                */
                Saver & enable() {
                    enabled_ = true;
                    return * this;
                }

        };
    };
    /*** Create an object that will capture the current value of
     *   savee, then restore this value when the created
     *   object is destroyed.
    */
    template<typename T> Private::Saver<T> Save( T& savee) {
        return Private::Saver<T>(savee);
    }
};
#endif