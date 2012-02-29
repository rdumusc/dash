
/* Copyright (c) 2011, EFPL/Blue Brain Project
 *                     Stefan Eilemann <stefan.eilemann@epfl.ch> 
 *
 * This file is part of DASH <https://github.com/BlueBrain/dash>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "test.h"

#include <dash/Context.h>
#include <dash/Vector.h>

#include <co/base/clock.h>
#include <co/base/init.h>
#include <co/base/monitor.h>
#include <co/base/omp.h>
#include <co/base/thread.h>

#include <limits>

#define LOOPSIZE  200000

co::base::Clock _clock;
typedef dash::Vector< size_t > Vector_t;
float rTime_;
float wTime_;
float eTime_;
float cTime_;
float pTime_;

co::base::Monitor< size_t > stage_;
#define STAGESIZE 10000;

class Reader : public co::base::Thread
{
public:
    Reader() {}
    virtual ~Reader() {}

    virtual void run()
        {
            size_t stage = 0;
            while( true )
            {
                stage += STAGESIZE;
                stage_.waitGE( stage );
                if( stage_ == std::numeric_limits< size_t >::max( ))
                    return;

                if( vector_ )
                {
                    for( size_t i = 0; i < LOOPSIZE; ++i )
                    {
                        if( i < vector_->size( ))
                        {
                            const size_t value = (*vector_)[ i ];
                            TESTINFO( i == value || value == 0,
                                      i << ", " << value );
                        }
                    }
                    rTime_ = _clock.getTimef();
                }
                ++stage_;
            }
        }
    Vector_t* vector_;
};

class Writer : public co::base::Thread
{
public:
    Writer() {}
    virtual ~Writer() {}

    virtual void run()
        {
            size_t stage = 0;
            while( true )
            {
                stage += STAGESIZE;
                stage_.waitGE( stage );
                if( stage_ == std::numeric_limits< size_t >::max( ))
                    return;

                if( vector_ )
                {
                    for( size_t i = 0; i < LOOPSIZE; ++i )
                    {
                        if( i < vector_->size( ))
                            (*vector_)[ i ] = i;
                    }
                    wTime_ = _clock.getTimef();
                }
                ++stage_;
            }
        }
    Vector_t* vector_;
};

class Pusher : public co::base::Thread
{
public:
    Pusher() {}
    virtual ~Pusher() {}

    virtual void run()
        {
            size_t stage = 0;
            while( true )
            {
                stage += STAGESIZE;
                stage_.waitGE( stage );
                if( stage_ == std::numeric_limits< size_t >::max( ))
                    return;

                if( vector_ )
                {
                    while( vector_->size() < LOOPSIZE )
                        vector_->push_back( 0 );
                    pTime_ = _clock.getTimef();
                }
                ++stage_;
            }
        }
    Vector_t* vector_;
};

class Copier : public co::base::Thread
{
public:
    Copier() {}
    virtual ~Copier() {}

    virtual void run()
        {
            Vector_t copy;
            copy = *vector_;
            *vector_ = copy;
            TEST( copy.size() >= vector_->size( ));
            cTime_ = _clock.getTimef();
        }
    Vector_t* vector_;
};

class Eraser : public co::base::Thread
{
public:
    Eraser() {}
    virtual ~Eraser() {}

    virtual void run()
        {
            const size_t pos = vector_->size() / 2;
            Vector_t::iterator i = vector_->begin() + pos;
            Vector_t::iterator j = vector_->erase( i );
            TEST( j != vector_->end( ));
            TEST( i == j );
            eTime_ = _clock.getTimef();
        }
    Vector_t* vector_;
};

class Flusher : public co::base::Thread
{
public:
    Flusher() {}
    virtual ~Flusher() {}

    virtual void run()
        {
            size_t i = 0xFFFFFFFFu;
            while( vector_->pop_back( i ))
            {
                TEST( i == 0 || i >= vector_->size( ));
            }
        }
    Vector_t* vector_;
};

template< class V, class T > void _runSerialTest()
{
    V vector;

    _clock.reset();
    while( vector.size() < LOOPSIZE*10 )
        vector.push_back( T( 0 ));
    pTime_ = _clock.getTimef();

    _clock.reset();
    for( size_t i = 0; i < LOOPSIZE*10; ++i )
    {
        if( i < vector.size( ))
            vector[ i ] = i;
    }
    wTime_ = _clock.getTimef();

    _clock.reset();
    typename V::const_iterator it = vector.begin();
    for( size_t i = 0; i < LOOPSIZE*10; ++i )
    {
        const size_t value = *it;
        TESTINFO( i == value || value == 0, i << ", " << value );
        ++it;
        if( it == vector.end( ))
            it = vector.begin();
    }
    rTime_ = _clock.getTimef();

    _clock.reset();
    for( size_t i = 0; i < 10; ++i )
    {
        V copy;
        copy = vector;
        vector = copy;
        TEST( copy.size() >= vector.size( ));
    }
    cTime_ = _clock.getTimef();

    _clock.reset();
    const size_t pos = vector.size() / 2;
    typename V::iterator i = vector.begin() + pos;
    typename V::iterator j = vector.erase( i );
    TEST( j != vector.end( ));
    TEST( *j == 0 || *j >= pos );
    eTime_ = _clock.getTimef();

    _clock.reset();
    while( !vector.empty( ))
        vector.pop_back();
    const float fTime = _clock.getTimef();

    std::cerr << std::setw(11) << float(LOOPSIZE*10)/rTime_ << ", "
              << std::setw(11) << float(LOOPSIZE*10)/wTime_ << ", "
              << std::setw(11) << float(LOOPSIZE*10)/pTime_ << ", " 
              << std::setw(9) << float(10)/cTime_ << ", "
              << std::setw(9) << float(10)/eTime_ << ", "
              << std::setw(9) << float(LOOPSIZE*10)/fTime << ", "
              << std::setw(3) << 0 << ", " << std::setw(3) << 0
              << std::endl;

    vector.push_back( 42 );
    i = vector.begin();
    i = vector.erase( i );
    TEST( i == vector.begin( ));
}

int dash::test::main( int argc, char **argv )
{
    //co::base::sleep( 5000 );
    dash::Context::getMain( argc, argv );

#ifdef CO_USE_OPENMP
    const size_t nThreads = co::base::OMP::getNThreads() * 3;
#else
    const size_t nThreads = 16;
#endif

    std::cout << "       read,       write,        push,      copy,     erase,  "
              << "flush/ms,  rd, other #threads" << std::endl;
    _runSerialTest< std::vector< size_t >, size_t >();
    _runSerialTest< dash::Vector< size_t >, size_t >();

    std::vector< Reader > readers(nThreads);
    std::vector< Writer > writers(nThreads);
    std::vector< Pusher > pushers(nThreads);

    stage_ = 1;
    size_t stage = 0;

    for( size_t l = 0; l < nThreads; ++l )
    {
        readers[l].start();
        writers[l].start();
        pushers[l].start();
    }
    co::base::sleep( 10 );

    for( size_t i = 1; i <= nThreads; i = i<<1 )
        for( size_t j = 1; j <= nThreads; j = j<<1 )
        {
            // concurrent read, write, push
            Vector_t vector;
            for( size_t k = 0; k < nThreads; ++k )
            {
                readers[k].vector_ = k < i ? &vector : 0;
                writers[k].vector_ = k < j ? &vector : 0;
                pushers[k].vector_ = k < j ? &vector : 0;
            }

            const size_t nextStage = ++stage * STAGESIZE;

            _clock.reset();
            stage_ = nextStage;
            stage_.waitEQ( nextStage + (3 * nThreads) );
            TEST( vector.size() >= LOOPSIZE );

            // multi-threaded copy
            std::vector< Copier > copiers(j);

            _clock.reset();
            for( size_t k = 0; k < j; ++k )
            {
                copiers[k].vector_ = &vector;
                copiers[k].start();
            }
            for( size_t k = 0; k < j; ++k )
                copiers[k].join();

            for( size_t k = 0; k < vector.size(); ++k )
                TEST( vector[k] == k || vector[k] == 0 );
            
            // multi-threaded erase
            std::vector< Eraser > erasers(j);

            _clock.reset();
            for( size_t k = 0; k < j; ++k )
            {
                erasers[k].vector_ = &vector;
                erasers[k].start();
            }
            for( size_t k = 0; k < j; ++k )
                erasers[k].join();

            for( size_t k = 0; k < vector.size(); ++k )
            {
                if( vector[k] == 0 )
                    break;
                if( k > vector.size() / 2 )
                {
                    TEST( vector[k] > vector[k-1] );
                }
                else
                {
                    TEST( vector[k] == k );
                }
            }

            // multi-threaded pop_back
            const size_t fOps = vector.size();
            std::vector< Flusher > flushers(j);
            _clock.reset();
            for( size_t k = 0; k < j; ++k )
            {
                flushers[k].vector_ = &vector;
                flushers[k].start();
            }
            for( size_t k = 0; k < j; ++k )
                flushers[k].join();
            const float fTime = _clock.getTimef();
            TEST( vector.empty( ));

            std::cerr << std::setw(11) << float(i*LOOPSIZE)/rTime_ << ", "
                      << std::setw(11) << float(j*LOOPSIZE)/wTime_ << ", "
                      << std::setw(11) << float(LOOPSIZE)/pTime_ << ", " 
                      << std::setw(9) << float(j)/cTime_ << ", "
                      << std::setw(9) << float(j)/eTime_ << ", "
                      << std::setw(9) << float(fOps)/fTime << ", "
                      << std::setw(3) << i << ", " << std::setw(3) << j
                      << std::endl;
        }

    stage_ = std::numeric_limits< size_t >::max();
    for( size_t k = 0; k < nThreads; ++k )
    {
        readers[k].join();
        writers[k].join();
        pushers[k].join();
    }

    return EXIT_SUCCESS;
}
