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

// Uncomment first if FastNumberReader is inside your project source directory
// and set the correct path to it in your project settings.

//#include "FastNumberReader/NumberReader.h"
#include "NumberReader.h"

#include <new>
#include <vector>
#ifdef FNR_DEBUG
#   include <assert.h>
#endif

namespace fnr
{

enum eCharClass
{
    kCC_None = -1,
    kCC_Sign,
    kCC_Digit,
    kCC_Point,
    kCC_Hex,
    kCC_Exp,
    kCC_SuffixF,
    kCC_SuffixLD,
    kCC_Count
};

static const CharType* alphabet[kCC_Count] =
{
    "+-",
    "0123456789",
    ".",
    "xX",
    "eE"
    "fF"
    "lL"
};

static int IsCharIn(CharType ch, const char* str)
{
    if (!str)
        return -1;

    for (int i = 0; str[i]; ++i)
    {
        if (ch == str[i]) return i;
    }

    return -1;
}

static int ToDigit(CharType ch)
{
    const char* str = alphabet[kCC_Digit];
    for (int i = 0; str[i]; ++i)
    {
        if (ch == str[i]) return i;
    }

    return 0;
}

//-----------------------------------------------------------------------------

static eCharClass GetCharClass(const CharType ch)
{
#if (FNR_GETCHARCLASS==0)

    if (ch >= '0' && ch <= '9')
    {
        return kCC_Digit;
    }
    else if ('.' == ch)
    {
        return kCC_Point;
    }
    else if ('-' == ch || '+' == ch)
    {
        return kCC_Sign;
    }
    else if (ch >= 'e' && ch <= 'x')
    {
        switch (ch)
        {
            case 'e': return kCC_Exp;
            case 'x': return kCC_Hex;
            case 'f': return kCC_SuffixF;
            case 'l': return kCC_SuffixLD;
        }
    }
    else if (ch >= 'E' && ch <= 'X')
    {
        switch (ch)
        {
            case 'E': return kCC_Exp;
            case 'X': return kCC_Hex;
            case 'F': return kCC_SuffixF;
            case 'L': return kCC_SuffixLD;
        }
    }
#elif (FNR_GETCHARCLASS==1)
    int i = -1;
    for (int k = kCC_Sign; k < kCC_Count; ++k)
    {
        i = IsCharIn(ch, alphabet[k]);
        if (-1 != i)
            return (eCharClass)k;
    }
#else
#   error "use 0 or 1 for FNR_GETCHARCLASS"
#endif
    return kCC_None;
}

//-----------------------------------------------------------------------------
bool isSpace(CharType ch)
{
    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r' || ch == '\v')
        return true;
    return false;
}

//-----------------------------------------------------------------------------
double pow(double x, int e)
{
    double y = 1.0;
    if (e > 0)
    {
        for (int i = 0; i < e; ++i)
        {
            y *= x;
        }
    }
    else if (e < 0)
    {
        y = 1.0 / pow(x, -e);
    }
    return y;
}

//-----------------------------------------------------------------------------
class DoubleReaderImpl
{
public:

    enum eType
    {
        kDouble,
        kFloat,
        kLongDouble
    };

    enum eState
    {
        kInitState,
        kWaitIDP_State,
        kWaitIDPE_State,
        kWaitFD_State,
        kWaitFDES_State,
        kWaitESD_State,
        kWaitEDS_State,
        kWaitTS_State,
        kStateCount
    };

private:

    struct State;

    static State* GetState(eState state);

    struct Data
    {
        friend class State;

        Data() : state_(0), type_(kDouble)
        {
            reset();
        }

        void reset()
        {
            state_ = 0;
            valid_ = false;

            sign_ = 1.0;
            value_ = 0.0;
            fracScale_ = 0.0;
            expSign_ = 1;
            expValue_ = 0;

            intDigits_ = 0;
            fracDigits_ = 0;
            expDigits_ = 0;
            trailingSpaces_ = 0;

        }

        void setType(eType type)
        {
            type_ = type;
        }

        int put(CharType ch)
        {
            if (!state_)
                GoTo(kInitState);
            return state_->put(ch);
        }

        void GoTo(eState state)
        {
            state_ = GetState(state);
        }

        eType type() const { return type_; }
        double value() const { return sign_ * value_ * pow(10.0, expSign_ * expValue_); }
        bool valid() const { return valid_; }
        double sign() const { return sign_; }

        int intDigits() const { return intDigits_; }
        int fracDigits() const { return fracDigits_; }
        int expDigits() const { return expDigits_; }
        int trailingSpaces() const { return trailingSpaces_; }

    private:


        void SetSign(bool neg) { neg ? sign_ = -1.0 : sign_ = 1.0; }
        void SetExpSign(bool neg) { neg ? expSign_ = -1 : expSign_ = 1; }


        State* state_;
        bool valid_;
        eType type_;

        double sign_;
        double value_;
        double fracScale_;
        int expSign_;
        int expValue_;

        int intDigits_;
        int fracDigits_;
        int expDigits_;
        int trailingSpaces_;
    };

    struct State
    {
        virtual int put(CharType ch)
        {
            assert(data_);
            return 0;
        }

    protected:
        State(Data* data) : data_(data) { }

        void AddIntDigit(CharType ch)
        {
            data_->value_ *= 10.0;
            data_->value_ += (double)ToDigit(ch);
            data_->intDigits_++;
        }

        void AddFracDigit(CharType ch)
        {
            data_->value_ += ((double)ToDigit(ch)) * data_->fracScale_;
            data_->fracScale_ *= 0.1;
            data_->fracDigits_++;
        }

        void AddExpDigit(CharType ch)
        {
            data_->expValue_ *= 10;
            data_->expValue_ += (int)ToDigit(ch);
            data_->expDigits_++;
        }

        void SetSign(CharType ch)
        {
            switch(ch)
            {
                case '-': data_->SetSign(true);  break;
                case '+': data_->SetSign(false);  break;
                default: break;
            }
        }

        void SetExpSign(CharType ch)
        {
            switch(ch)
            {
                case '-': data_->SetExpSign(true);  break;
                case '+': data_->SetExpSign(false);  break;
                default: break;
            }
        }

        void SetValid() { data_->valid_ = true; }
        void SetInvalid() { data_->valid_ = false; }

        void ResetFracScale() { data_->fracScale_ = 0.1; }

        Data* data_;
    };

    DoubleReaderImpl();

public:
    ~DoubleReaderImpl();
    static DoubleReaderImpl* Instance();

    //-----------------------------------------------------------------------------

    struct InitState : public State
    {
        InitState(Data* data) : State(data) {}

        int put(CharType ch)
        {
            State::put(ch);

            if (isSpace(ch))
            {
                return 1;
            }

            eCharClass charClass = GetCharClass(ch);

            if (kCC_None == charClass)
            {
                data_->reset();
                return 0;
            }

            if (kCC_Sign == charClass)
            {
                SetSign(ch);
                data_->GoTo(kWaitIDP_State);
                return 1;
            }

            if (kCC_Digit == charClass)
            {
                AddIntDigit(ch);
                data_->GoTo(kWaitIDPE_State);
                return 1;
            }

            if (kCC_Point == charClass)
            {
                ResetFracScale();
                data_->GoTo(kWaitFD_State);
                return 1;
            }

            data_->reset();
            return 0;

        } // put()
    }; // InitState class

    //-----------------------------------------------------------------------------

    struct WaitIDP_State : public State
    {
        WaitIDP_State(Data* data) : State(data) {}

        int put(CharType ch)
        {
            State::put(ch);

            eCharClass charClass = GetCharClass(ch);

            if (kCC_None == charClass)
            {
                data_->reset();
                return 0;
            }

            if (kCC_Digit == charClass)
            {
                AddIntDigit(ch);
                data_->GoTo(kWaitIDPE_State);
                return 1;
            }

            if (kCC_Point == charClass)
            {
                if (data_->intDigits() > 0)
                {
                    SetValid();
                    data_->GoTo(kWaitFDES_State);
                }

                ResetFracScale();
                data_->GoTo(kWaitFD_State);
                return 1;
            }

            data_->reset();
            return 0;

        } // put()
    }; // WaitIDP_State class

    //-----------------------------------------------------------------------------

    struct WaitIDPE_State : public State
    {
        WaitIDPE_State(Data* data) : State(data) {}

        int put(CharType ch)
        {
            State::put(ch);

            eCharClass charClass = GetCharClass(ch);

            if (kCC_None == charClass)
            {
                data_->reset();
                return 0;
            }

            if (kCC_Digit == charClass)
            {
                AddIntDigit(ch);
                return 1;
            }

            if (kCC_Point == charClass)
            {
                if (data_->intDigits() > 0)
                {
                    SetValid();
                    data_->GoTo(kWaitFDES_State);
                }

                ResetFracScale();
                data_->GoTo(kWaitFD_State);
                return 1;
            }

            if (kCC_Exp == charClass)
            {
                SetInvalid();
                data_->GoTo(kWaitESD_State);
                return 1;
            }

            data_->reset();
            return 0;

        } // put()
    }; // WaitIDPE_State class

    //-----------------------------------------------------------------------------

    struct WaitFD_State : public State
    {
        WaitFD_State(Data* data) : State(data) {}

        int put(CharType ch)
        {
            State::put(ch);

            eCharClass charClass = GetCharClass(ch);

            if (kCC_None == charClass)
            {
                data_->reset();
                return 0;
            }

            if (kCC_Digit == charClass)
            {
                AddFracDigit(ch);
                SetValid();
                data_->GoTo(kWaitFDES_State);
                return 1;
            }

            data_->reset();
            return 0;

        } // put()
    }; // WaitFD_State class

    //-----------------------------------------------------------------------------

    struct WaitFDES_State : public State
    {
        WaitFDES_State(Data* data) : State(data) {}

        int put(CharType ch)
        {
            State::put(ch);

            if (isSpace(ch) && data_->valid())
            {
                return 1;
            }

            if (data_->trailingSpaces() > 0)
            {
                data_->reset();
                return 0;
            }

            eCharClass charClass = GetCharClass(ch);

            if (kCC_None == charClass)
            {
                data_->reset();
                return 0;
            }

            if (kCC_Digit == charClass)
            {
                AddFracDigit(ch);
                return 1;
            }

            if (kCC_Exp == charClass)
            {
                SetInvalid();
                data_->GoTo(kWaitESD_State);
                return 1;
            }

            if ((kCC_SuffixF == charClass && data_->type() == kFloat) ||
                (kCC_SuffixLD == charClass && data_->type() == kLongDouble))
            {
                data_->GoTo(kWaitTS_State);
                return 1;
            }

            data_->reset();
            return 0;

        } // put()
    }; // WaitFDES_State class

    //-----------------------------------------------------------------------------

    struct WaitESD_State : public State
    {
        WaitESD_State(Data* data) : State(data) {}

        int put(CharType ch)
        {
            State::put(ch);

            eCharClass charClass = GetCharClass(ch);

            if (kCC_None == charClass)
            {
                data_->reset();
                return 0;
            }

            if (kCC_Sign == charClass)
            {
                SetExpSign(ch);

                data_->GoTo(kWaitEDS_State);
                return 1;
            }

            if (kCC_Digit == charClass)
            {
                AddExpDigit(ch);
                SetValid();
                data_->GoTo(kWaitEDS_State);
                return 1;
            }

            data_->reset();
            return 0;

        } // put()
    }; // WaitESD_State class

    //-----------------------------------------------------------------------------

    struct WaitEDS_State : public State
    {
        WaitEDS_State(Data* data) : State(data) {}

        int put(CharType ch)
        {
            State::put(ch);

            if (isSpace(ch) && data_->valid())
            {
                return 1;
            }

            if (data_->trailingSpaces() > 0)
            {
                data_->reset();
                return 0;
            }

            eCharClass charClass = GetCharClass(ch);

            if (kCC_None == charClass)
            {
                data_->reset();
                return 0;
            }

            if (kCC_Digit == charClass)
            {
                AddExpDigit(ch);
                SetValid();
                return 1;
            }

            if ((kCC_SuffixF == charClass && data_->type() == kFloat) ||
                (kCC_SuffixLD == charClass && data_->type() == kLongDouble))
            {
                data_->GoTo(kWaitTS_State);
                return 1;
            }

            data_->reset();
            return 0;

        } // put()
    }; // WaitEDS_State class

    //-----------------------------------------------------------------------------

    struct WaitTS_State : public State
    {
        WaitTS_State(Data* data) : State(data) {}

        int put(CharType ch)
        {
            State::put(ch);

            if (isSpace(ch) && data_->valid())
            {
                return 1;
            }

            if (data_->trailingSpaces() > 0)
            {
                data_->reset();
                return 0;
            }

            data_->reset();
            return 0;

        } // put()
    }; // WaitTS_State class

    template<typename T>
    Data* data(T = 0);

private:
    InitState        *initState_;
    WaitIDP_State    *waitIDP_State_;
    WaitIDPE_State   *waitIDPE_State_;
    WaitFD_State     *waitFD_State_;
    WaitFDES_State   *waitFDES_State_;
    WaitESD_State    *waitESD_State_;
    WaitEDS_State    *waitEDS_State_;
    WaitTS_State     *waitTS_State_;

    Data data_;

    ByteType initStateMemory_       [sizeof(InitState)];
    ByteType waitIDP_StateMemory_   [sizeof(WaitIDP_State)];
    ByteType waitIDPE_StateMemory_  [sizeof(WaitIDPE_State)];
    ByteType waitFD_tateMemory_     [sizeof(WaitFD_State)];
    ByteType waitFDES_StateMemory_  [sizeof(WaitFDES_State)];
    ByteType waitESD_StateMemory_   [sizeof(WaitESD_State)];
    ByteType waitEDS_StateMemory_   [sizeof(WaitEDS_State)];
    ByteType waitTS_StateMemory_    [sizeof(WaitTS_State)];

    State* states_[kStateCount];

    static DoubleReaderImpl* sInstance_;

};

//-----------------------------------------------------------------------------
template<>
DoubleReaderImpl::Data* DoubleReaderImpl::data(double) { data_.setType(kDouble); return &data_; }

template<>
DoubleReaderImpl::Data* DoubleReaderImpl::data(float) { data_.setType(kFloat); return &data_; }

template<>
DoubleReaderImpl::Data* DoubleReaderImpl::data(long double) { data_.setType(kLongDouble); return &data_; }

//-----------------------------------------------------------------------------
DoubleReaderImpl* DoubleReaderImpl::sInstance_ = 0;

//-----------------------------------------------------------------------------
DoubleReaderImpl* DoubleReaderImpl::Instance()
{
    static ByteType instanceMemory[sizeof(DoubleReaderImpl)];
    if (!sInstance_)
    {
        sInstance_ = new (&instanceMemory[0]) DoubleReaderImpl();
    }
    return sInstance_;
}
//-----------------------------------------------------------------------------
DoubleReaderImpl::DoubleReaderImpl()
{
    assert(!sInstance_);
    sInstance_ = this;

    initState_          = new (&initStateMemory_[0])        InitState(data<double>());
    waitIDP_State_      = new (&waitIDP_StateMemory_[0])    WaitIDP_State(data<double>());
    waitIDPE_State_     = new (&waitIDPE_StateMemory_[0])   WaitIDPE_State(data<double>());
    waitFD_State_       = new (&waitFD_tateMemory_[0])      WaitFD_State(data<double>());
    waitFDES_State_     = new (&waitFDES_StateMemory_[0])    WaitFDES_State(data<double>());
    waitESD_State_      = new (&waitESD_StateMemory_[0])    WaitESD_State(data<double>());
    waitEDS_State_      = new (&waitEDS_StateMemory_[0])     WaitEDS_State(data<double>());
    waitTS_State_       = new (&waitTS_StateMemory_[0])     WaitTS_State(data<double>());

    states_[kInitState]         = initState_;
    states_[kWaitIDP_State]     = waitIDP_State_;
    states_[kWaitIDPE_State]    = waitIDPE_State_;
    states_[kWaitFD_State]      = waitFD_State_;
    states_[kWaitFDES_State]    = waitFDES_State_;
    states_[kWaitESD_State]     = waitESD_State_;
    states_[kWaitEDS_State]     = waitEDS_State_;
    states_[kWaitTS_State]      = waitTS_State_;
}

//-----------------------------------------------------------------------------

DoubleReaderImpl::~DoubleReaderImpl()
{
    sInstance_ = 0;
}
//-----------------------------------------------------------------------------
DoubleReaderImpl::State* DoubleReaderImpl::GetState(DoubleReaderImpl::eState state)
{
    return DoubleReaderImpl::Instance()->states_[state];
}

//*****************************************************************************
// NumberReader<double>
//*****************************************************************************

//-----------------------------------------------------------------------------

NumberReader<double>::NumberReader() : impl_(DoubleReaderImpl::Instance())
{

}

//-----------------------------------------------------------------------------
NumberReader<double>::~NumberReader()
{
    impl_->data<double>()->reset();
}
//-----------------------------------------------------------------------------

int NumberReader<double>::put(CharType ch)
{
    return impl_->data<double>()->put(ch);
}

//-----------------------------------------------------------------------------

double NumberReader<double>::value() const
{
    return impl_->data<double>()->value();
}

//-----------------------------------------------------------------------------

bool NumberReader<double>::valid() const
{
    return impl_->data<double>()->valid();
}

//*****************************************************************************
// NumberReader<float>
//*****************************************************************************

//-----------------------------------------------------------------------------

NumberReader<float>::NumberReader() : impl_(DoubleReaderImpl::Instance())
{

}

//-----------------------------------------------------------------------------
NumberReader<float>::~NumberReader()
{
    impl_->data<float>()->reset();
}
//-----------------------------------------------------------------------------

int NumberReader<float>::put(CharType ch)
{
    return impl_->data<float>()->put(ch);
}

//-----------------------------------------------------------------------------

float NumberReader<float>::value() const
{
    return impl_->data<float>()->value();
}

//-----------------------------------------------------------------------------

bool NumberReader<float>::valid() const
{
    return impl_->data<float>()->valid();
}

//*****************************************************************************
// NumberReader<long double>
//*****************************************************************************

//-----------------------------------------------------------------------------

NumberReader<long double>::NumberReader() : impl_(DoubleReaderImpl::Instance())
{

}

//-----------------------------------------------------------------------------
NumberReader<long double>::~NumberReader()
{
    impl_->data<long double>()->reset();
}
//-----------------------------------------------------------------------------

int NumberReader<long double>::put(CharType ch)
{
    return impl_->data<long double>()->put(ch);
}

//-----------------------------------------------------------------------------

long double NumberReader<long double>::value() const
{
    return impl_->data<long double>()->value();
}

//-----------------------------------------------------------------------------

bool NumberReader<long double>::valid() const
{
    return impl_->data<long double>()->valid();
}
} // end of fnr
