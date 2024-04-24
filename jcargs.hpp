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
 * File:   args.hpp
 * Author: Julia Clement <Julia at Clement dot nz>
 * 
 * Part of the JCLib project https://github.com/juliaclement/jclib
 *
 * Created 22-24 October 2022
 */
#ifndef JCARGS_HPP
#define JCARGS_HPP
#include "countedPointer.hpp"
#include "jString.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <map>

/**
 * Flexible, header only parser for command line arguments.
 * 
 * Designed to support the awkccc project, but not limited to it.
*/

namespace jclib {
    class arg_base : public CountedPointerTarget {
        public:
            class arguments * owner_ = nullptr;
            jString short_name_;
            jString long_name_;
            jString desc_;
            bool is_set_ = false;
            bool is_defaulted_ = false;
            bool multiples_ok_=false;
            bool takes_parameter_;
            bool required_;
            virtual bool is_set() const { return is_set_; }
            virtual bool set() {return false;}
            virtual bool set(jString &value) {return false;}
            arg_base(jString short_name, jString long_name, jString desc, bool takes_parameter, bool required=false)
            : short_name_(short_name)
            , long_name_(long_name)
            , desc_(desc)
            , takes_parameter_(takes_parameter)
            , required_(required)
            {}
    };

    class simple_bool : public arg_base {
        public: 
            bool & answer_;
            virtual bool set() {answer_ = true; is_set_ = true; return true;}
            simple_bool( bool & answer, jString short_name, jString long_name, jString desc, bool takes_parameter, bool required=false )
            : arg_base( short_name, long_name, desc, takes_parameter, required )
            , answer_( answer)
            {}
            virtual bool set(jString &value) {return false;} // FIXME: Doesn't take value;
    };

    template<class T> class arg_instance : public arg_base {
        public: 
            T & answer_;
            virtual bool set(jString &value);
            arg_instance( T & answer, jString short_name, jString long_name, jString desc, bool takes_parameter, bool required=false )
            : arg_base( short_name, long_name, desc, takes_parameter, required )
            , answer_( answer)
            {}
    };

    template<> bool arg_instance<short int>::set(jString &value) {
        errno = 0;
        answer_ = strtol(value.data(),nullptr,10);
        return errno == 0;
    }

    template<> bool arg_instance<int>::set(jString &value) {
        errno = 0;
        answer_ = strtol(value.data(),nullptr,10);
        return errno == 0;
    }

    template<> bool arg_instance<long>::set(jString &value) {
        errno = 0;
        answer_ = strtol(value.data(),nullptr,10);
        return errno == 0;
    }

    template<> bool arg_instance<long long>::set(jString &value) {
        errno = 0;
        answer_ = strtoll(value.data(),nullptr,10);
        return errno == 0;
    }

    template<> bool arg_instance<unsigned short int>::set(jString &value) {
        errno = 0;
        answer_ = strtoul(value.data(),nullptr,10);
        return errno == 0;
    }

    template<> bool arg_instance<unsigned int>::set(jString &value) {
        errno = 0;
        answer_ = strtoul(value.data(),nullptr,10);
        return errno == 0;
    }

    template<> bool arg_instance<unsigned long>::set(jString &value) {
        errno = 0;
        answer_ = strtoul(value.data(),nullptr,10);
        return errno == 0;
    }

    template<> bool arg_instance<unsigned long long>::set(jString &value) {
        errno = 0;
        answer_ = strtoull(value.data(),nullptr,10);
        return errno == 0;
    }

    template<> bool arg_instance<jString>::set(jString &value) {
        answer_ = value;
        return true;
    }

    template<> bool arg_instance<std::string>::set(jString &value) {
        answer_.assign(value.data(), value.len());
        return true;
    }

    template<> bool arg_instance< std::vector<jString> >::set(jString &value) {
        errno = 0;
        answer_.push_back(value);
        multiples_ok_ = true;
        return errno == 0;
    }

    template<class T> arg_instance<T>* arg(T&answer, jString short_name, jString long_name, jString desc, bool takes_parameter, bool required=false ){
            return new arg_instance<T>( answer, short_name, long_name, desc, takes_parameter, required );}

    simple_bool* arg(bool&answer, jString short_name, jString long_name, jString desc, bool takes_parameter, bool required=false ){
            return new simple_bool( answer, short_name, long_name, desc, takes_parameter, required );}

    class arguments{
        public:
            std::ostream & error_log_;
            std::vector< jString > positional_args_;
            // jString end_keywords_ = "--";
            std::vector<CountedPointer< arg_base > > all_options_;
            typedef std::map<jString, CountedPointer< arg_base > > map_T;
            map_T short_names_;
            map_T long_names_;
            jString help_prefix_ = jclib::jString::get_empty();
            jString help_suffix_ = jclib::jString::get_empty();
            jString version_string_ = jclib::jString::get_empty();
            int errors_ = 0;
            bool show_help_ = false;
            bool show_version_ = false;
            void insert(CountedPointer<arg_base> input) {
                all_options_.push_back( input );
                if( input->short_name_.len()==1) {
                    if( short_names_.find(input->short_name_) != short_names_.end() ) {
                        error_log_ << "Internal error: \"" << input->short_name_ << "\" duplicated\n";
                        errors_++;
                    }
                    else{
                        short_names_[input->short_name_] = input;
                        input->owner_ = this;
                    }
                } else if( input->short_name_.len() != 0 ) {
                    error_log_ << "Internal error: \"" << input->short_name_ << "\" longer than 1 character\n";
                    errors_++;
                }
                if( input->long_name_.len()>0) {
                    if( long_names_.find(input->long_name_) != long_names_.end() ) {
                        error_log_ << "Internal error: \"" << input->long_name_ << "\" duplicated\n";
                        errors_++;
                    }
                    else{
                        long_names_[input->long_name_] = input;
                        input->owner_ = this;
                    }
                }
            }

            void load(std::initializer_list<arg_base *> input) {
                for( auto i : input) {
                    insert( i );
                }
            }

            // Process a configuration file, one option per line
            // Lines starting with # are comments
            // Blank lines are skipped
            void process_config( std::istream & file, jString filename ) {
                constexpr int max_line_len = 1024;
                char buffer[ max_line_len + 1];
                char * bufptr;
                int linenr = 0;
                jString optionName, value;
                map_T::iterator found;
                while( file.getline(buffer, max_line_len)) {
                    buffer[ max_line_len ] = '\0'; // Prevent overrun, normally there's an earlier null
                    linenr++;
                    char * buf = buffer;
                    // skip leading space on the line
                    while( *buf == ' ' || *buf == '\t')
                        buf++;
                    // skip empty & comment lines
                    if( *buf == '\0' || *buf == '#' )
                        continue;
                    if( *buf == '-' ) {
                        error_log_ << "Error config file lines must start with option or '#' not '-'. Line "
                                  << linenr << " of " << filename << "\n";
                        errors_++;
                        continue;
                    }
                    bufptr = buf + 1;
                    if( *bufptr == ' ' || *bufptr == '\t' || *bufptr == '=' ) {
                        // Unix style 1 character argument name
                        optionName = jString(buf, bufptr );
                        found = short_names_.find(optionName);
                        if( found == short_names_.end()) {
                            error_log_<< "Unknown argument " << optionName << linenr << " of " << filename << "\n";
                            errors_++;
                            continue;
                        }
                    } else {
                        // GNU style long argument name
                        while ( isalnum(*bufptr)) {
                            bufptr++;
                        }
                        optionName = jString(buf, bufptr );
                        found = long_names_.find(optionName);
                        if( found == long_names_.end()) {
                            error_log_<< "Unknown argument " << optionName << linenr << " of " << filename << "\n";
                            errors_++;
                            continue;
                        }
                    }
                    arg_base & arg = * found->second;
                    // config does not override options already set on the command line
                    if( arg.is_set_ )
                        continue;
                    while( *bufptr == '=' || *bufptr == ' ' || *bufptr == '\t' )
                        ++bufptr;
                    char * bufend = bufptr;
                    // Find end of string
                    while( *bufend )
                        ++bufend;
                    while( bufend != bufptr && (*bufend == ' ' || *bufend == '\t' || *bufend == '\0' ))
                        --bufend;
                    jString val( bufptr, bufend+1 );
                    if( arg.takes_parameter_) {
                        arg.is_defaulted_ = arg.set(val);
                    } else {
                        if( val.len() == 0 )
                            arg.is_defaulted_ = arg.set();
                        else {
                            error_log_ << optionName << " does not accept options " << linenr << " of " << filename << "\n";
                            errors_++;
                            }
                    }
                }
            }

            bool process_args( int argc, const char ** argv, int start_at = 1) {
                int argnum = start_at;
                for( ; argnum < argc && argv[ argnum ][0] == '-'; ++argnum ) {
                    const char * argstr=argv[argnum ];
                    if( argstr[1] == '-') { // long arg
                        if( argstr[2] == '\0') { // end of kw args
                            ++argnum;
                            break;
                        }
                        std::vector<jString> argarray = jString(argstr+2).split("=",1);
                        jString kw=argarray[0];
                        jString val=argarray[1];
                        if( ! val.isvalid())
                            val = jString::get_empty();
                        auto found = long_names_.find(kw);
                        if( found == long_names_.end()) {
                            error_log_<< "Unknown argument " << argstr << "\n";
                            errors_++;
                            continue;
                        }else{
                            arg_base & arg = * found->second;
                            if( arg.is_set_ && ! arg.multiples_ok_ ) {
                                error_log_<< "Multiple use of " << argstr << "\n";
                                errors_++;
                                continue;
                            }
                            if( arg.takes_parameter_) {
                                int val_len = val.len();
                                int nextargnum=argnum + 1;
                                if( (val_len==0) && (nextargnum < argc) && (argv[nextargnum][0] != '-')) {
                                    val = argv[++argnum];
                                }
                                arg.is_set_ = arg.set(val);
                            } else {
                                if( val.len() == 0 )
                                    arg.is_set_ = arg.set();
                                else {
                                    error_log_ << arg.long_name_ << " does not accept options\n";
                                    errors_++;
                                 }
                            }
                        }
                    } else {
                        // args can be run together, so we have to decode the arg to know 
                        // e.g. -abcdef might be -a(=bcdef) or -a -b -c(=def) or ...
                        const char * end = argstr + strlen(argstr);
                        // +1 to skip the -
                        for( const char *chars = argstr+1; chars < end; ++ chars ) {
                            auto kw=jString(chars,1);
                            auto found = short_names_.find(kw);
                            if( found == short_names_.end()) {
                                error_log_<< "Unknown argument " << argstr << "\n";
                                errors_++;
                            }else{
                                arg_base & arg = * found->second;
                                if( arg.is_set_ && ! arg.multiples_ok_ ) {
                                    error_log_<< "Multiple use of " << argstr << "\n";
                                    errors_++;
                                    continue;
                                }
                                if( arg.takes_parameter_) {
                                    jString val(chars+1);
                                    if( chars[1] == '\0' && (argnum + 1) < argc && argv[argnum+1][0] != '-') {
                                        val = argv[++argnum];
                                    }
                                    arg.is_set_ = arg.set(val);
                                    break;
                                } else 
                                    arg.is_set_ = arg.set();
                            }
                        }
                    }
                }
                // Now process the remaining (positional) arguments
                for( ; argnum < argc; ++ argnum) {
                    this->positional_args_.push_back(argv[argnum]);
                }
                // Finally, scan arguments to make sure required ones are set
                
                if( ! (show_version_ || show_help_ ) )
                {
                    for( auto k : all_options_) {
                        if(  k->required_ &&  ! ( k->is_set_ || k->is_defaulted_) ) {
                            errors_++;
                            error_log_ << "Required option \"";
                            if( k->short_name_.len() > 0 && k->long_name_.len() > 0 )
                                error_log_ << "--" << k->long_name_ << "\" (\"-" << k->short_name_ << "\")";
                            else if (k->short_name_.len() > 0)
                                error_log_ << "-" << k->short_name_ << "\"";
                            else
                                error_log_ << "--" << k->long_name_ << "\"";
                            if( k->desc_.len() > 0 )
                                error_log_ << " (" << k->desc_ << ")";
                            error_log_ << " missing\n";
                            errors_++;
                        }
                    }
                }
                // If version has been requested & we are set-up to provide it.
                if( show_version_ ) {
                    error_log_ << version_string_ << "\n";;
                }
                // If help has been requested or a parameter error is detected & we are set-up to provide it.
                if( show_help_ || ( errors_ != 0 && help_prefix_.len()>0 ) ) {
                    error_log_ << help_prefix_ << "\n";
                    // find longest long option name
                    size_t longest = 1;
                    for( auto k : all_options_) {
                        longest=std::max<size_t>( longest, k->long_name_.len() );
                    }
                    // now print the lines
                    longest ++;
                    for( auto k : all_options_) {
                        if( k->short_name_.len() > 0 )
                            error_log_ << "  -" << k->short_name_ << " ";
                        else
                            error_log_<< "     ";
                        error_log_ << (k->long_name_.len()==0 ? "  " : "--")
                                   << k->long_name_.pad(longest)
                                   << k->desc_ << "\n";
                    }
                    error_log_ << help_suffix_ << "\n";
                }
                return errors_ == 0 && !(show_version_ || show_help_);
            }

            arguments(std::ostream & error_log=std::cerr)
                : error_log_( error_log)
            {}
    };

    class config : public arg_base {
        public:
            arguments & processor_;
            virtual bool set() { return false;}
            config( arguments & processor, jString short_name, jString long_name, jString desc )
            : arg_base( short_name, long_name, desc, true, false )
            , processor_( processor )
            {
                multiples_ok_ = true;
            }
            virtual bool set(jString &value) {
                std::ifstream stream;
                stream.open( value, std::ifstream::in);
                if( stream.is_open()) {
                    processor_.process_config( stream, value);
                    stream.close();
                } else {
                    processor_.error_log_ << "Can not open config file " << value << "\n";
                    processor_.errors_++;
                }
                return false;
            }
    };
}
#endif