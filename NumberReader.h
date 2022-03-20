/*
MIT License

Copyright (c) 2022 Marat Sungatullin (MrSung, mrsung82, graveman, graveman82)

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

#ifndef FAST_NUMBER_READER__NUMBERREADER_H
#define FAST_NUMBER_READER__NUMBERREADER_H

// Uncomment first if FastNumberReader is inside your project source directory
// and set the correct path to it in your project settings.

//#define FNR_INCLUDED_IN_LARGER_PROJECT
#ifdef FNR_INCLUDED_IN_LARGER_PROJECT
#   include "FastNumberReader/Config.h"
#else
#   include "Config.h"
#endif

namespace fnr
{

class DoubleReaderImpl;

//-----------------------------------------------------------------------------

template <typename T>
class NumberReader;

//-----------------------------------------------------------------------------
//
template <>
class NumberReader<double>
{
public:
    NumberReader();
    ~NumberReader();
    int put(CharType ch);
    double value() const;
    bool valid() const;

private:

    DoubleReaderImpl* impl_;

};

//-----------------------------------------------------------------------------
//
template <>
class NumberReader<float>
{
public:
    NumberReader();
    ~NumberReader();
    int put(CharType ch);
    float value() const;
    bool valid() const;

private:

    DoubleReaderImpl* impl_;

};

//-----------------------------------------------------------------------------
//
template <>
class NumberReader<long double>
{
public:
    NumberReader();
    ~NumberReader();
    int put(CharType ch);
    long double value() const;
    bool valid() const;

private:

    DoubleReaderImpl* impl_;

};

//*****************************************************************************
class IntegerReaderImpl;

//-----------------------------------------------------------------------------
//
template <>
class NumberReader<long>
{
public:
    NumberReader();
    ~NumberReader();
    int put(CharType ch);
    long value() const;
    bool valid() const;

private:

    IntegerReaderImpl* impl_;

};

//-----------------------------------------------------------------------------
//
template <>
class NumberReader<int>
{
public:
    NumberReader();
    ~NumberReader();
    int put(CharType ch);
    int value() const;
    bool valid() const;

private:

    IntegerReaderImpl* impl_;

};

//-----------------------------------------------------------------------------
//
template <>
class NumberReader<short>
{
public:
    NumberReader();
    ~NumberReader();
    int put(CharType ch);
    short value() const;
    bool valid() const;

private:

    IntegerReaderImpl* impl_;

};


} // end of fnr


#endif // FAST_NUMBER_READER__NUMBERREADER_H
