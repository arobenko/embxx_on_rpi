//
// Copyright 2014 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <functional>
#include <chrono>

#include "embxx/util/Assert.h"
#include "embxx/error/ErrorStatus.h"

template <typename TLed, typename TInBuf, typename TTimerMgr>
class Morse
{
public:
    typedef TLed Led;
    typedef TInBuf InBuf;
    typedef typename InBuf::CharType CharType;
    typedef TTimerMgr TimerMgr;
    typedef typename TimerMgr::Timer Timer;

    Morse(Led& led, InBuf& buf, TimerMgr& timerMgr)
      : led_(led),
        buf_(buf),
        timer_(timerMgr.allocTimer())
    {
        GASSERT(timer_.isValid());
    }

    ~Morse() = default;

    void start()
    {
        buf_.start();
        nextLetter();
    }

private:
    typedef unsigned Duration;
    static const Duration Dot = 200;
    static const Duration Dash = Dot * 3;
    static const Duration End = 0;
    static const Duration Spacing = Dot;
    static const Duration InterSpacing = Spacing * 2;


    void nextLetter()
    {
        buf_.asyncWaitDataAvailable(
            1U,
            [this](const embxx::error::ErrorStatus& es)
            {
                if (es) {
                    GASSERT(buf_.empty());
                    nextLetter();
                    return;
                }

                GASSERT(!buf_.empty());
                auto ch = buf_[0];
                buf_.consume(1U);

                auto* seq = getLettersSeq(ch);
                if (seq == nullptr) {
                    nextLetter();
                    return;
                }

                nextSyllable(seq);
            });
    }

    void nextSyllable(const Duration* seq)
    {
        GASSERT(seq != nullptr);
        GASSERT(*seq != End);

        auto duration = *seq;
        ++seq;

        led_.on();
        timer_.asyncWait(
            std::chrono::milliseconds(duration),
            [this, seq](const embxx::error::ErrorStatus& es)
            {
                static_cast<void>(es);
                GASSERT(!es);

                led_.off();

                if (*seq != End) {
                    timer_.asyncWait(
                        std::chrono::milliseconds(Duration(Spacing)),
                        [this, seq](const embxx::error::ErrorStatus& es)
                        {
                            static_cast<void>(es);
                            GASSERT(!es);
                            nextSyllable(seq);
                        });
                    return;
                }

                timer_.asyncWait(
                    std::chrono::milliseconds(Duration(InterSpacing)),
                    [this](const embxx::error::ErrorStatus& es)
                    {
                        static_cast<void>(es);
                        GASSERT(!es);
                        nextLetter();
                    });
            });
    }

    const Duration* getLettersSeq(CharType ch) const
    {
        static const Duration Seq_A[] = {Dot, Dash, End};
        static const Duration Seq_B[] = {Dash, Dot, Dot, Dot, End};
        static const Duration Seq_C[] = {Dash, Dot, Dash, Dot, End};
        static const Duration Seq_D[] = {Dash, Dot, Dot, End};
        static const Duration Seq_E[] = {Dot, End};
        static const Duration Seq_F[] = {Dot, Dot, Dash, Dot, End};
        static const Duration Seq_G[] = {Dash, Dash, Dot, End};
        static const Duration Seq_H[] = {Dot, Dot, Dot, Dot, End};
        static const Duration Seq_I[] = {Dot, Dot, End};
        static const Duration Seq_J[] = {Dot, Dash, Dash, Dash, End};
        static const Duration Seq_K[] = {Dash, Dot, Dash, End};
        static const Duration Seq_L[] = {Dot, Dash, Dot, Dot, End};
        static const Duration Seq_M[] = {Dash, Dash, End};
        static const Duration Seq_N[] = {Dash, Dot, End};
        static const Duration Seq_O[] = {Dash, Dash, Dash, End};
        static const Duration Seq_P[] = {Dot, Dash, Dash, Dot, End};
        static const Duration Seq_Q[] = {Dash, Dash, Dot, Dash, End};
        static const Duration Seq_R[] = {Dot, Dash, Dot, End};
        static const Duration Seq_S[] = {Dot, Dot, Dot, End};
        static const Duration Seq_T[] = {Dash, End};
        static const Duration Seq_U[] = {Dot, Dot, Dash, End};
        static const Duration Seq_V[] = {Dot, Dot, Dot, Dash, End};
        static const Duration Seq_W[] = {Dot, Dash, Dash, End};
        static const Duration Seq_X[] = {Dash, Dot, Dot, Dash, End};
        static const Duration Seq_Y[] = {Dash, Dot, Dash, Dash, End};
        static const Duration Seq_Z[] = {Dash, Dash, Dot, Dot, End};

        static const Duration Seq_0[] = {Dash, Dash, Dash, Dash, Dash, End};
        static const Duration Seq_1[] = {Dot, Dash, Dash, Dash, Dash, End};
        static const Duration Seq_2[] = {Dot, Dot, Dash, Dash, Dash, End};
        static const Duration Seq_3[] = {Dot, Dot, Dot, Dash, Dash, End};
        static const Duration Seq_4[] = {Dot, Dot, Dot, Dot, Dash, End};
        static const Duration Seq_5[] = {Dot, Dot, Dot, Dot, Dot, End};
        static const Duration Seq_6[] = {Dash, Dot, Dot, Dot, Dot, End};
        static const Duration Seq_7[] = {Dash, Dash, Dot, Dot, Dot, End};
        static const Duration Seq_8[] = {Dash, Dash, Dash, Dot, Dot, End};
        static const Duration Seq_9[] = {Dash, Dash, Dash, Dash, Dot, End};

        static const Duration* Letters[] = {
            Seq_A,
            Seq_B,
            Seq_C,
            Seq_D,
            Seq_E,
            Seq_F,
            Seq_G,
            Seq_H,
            Seq_I,
            Seq_J,
            Seq_K,
            Seq_L,
            Seq_M,
            Seq_N,
            Seq_O,
            Seq_P,
            Seq_Q,
            Seq_R,
            Seq_S,
            Seq_T,
            Seq_U,
            Seq_V,
            Seq_W,
            Seq_X,
            Seq_Y,
            Seq_Z
        };

        static_assert(std::extent<decltype(Letters)>::value == (('Z' - 'A') + 1),
            "Incorrect alphabet.");

        static const Duration* Numbers[] = {
            Seq_0,
            Seq_1,
            Seq_2,
            Seq_3,
            Seq_4,
            Seq_5,
            Seq_6,
            Seq_7,
            Seq_8,
            Seq_9
        };

        static_assert(std::extent<decltype(Numbers)>::value == (('9' - '0') + 1),
            "Incorrect alphabet.");

        if ((static_cast<CharType>('A') <= ch) &&
            (ch <= static_cast<CharType>('Z'))) {
            return Letters[ch - 'A'];
        }

        if ((static_cast<CharType>('a') <= ch) &&
            (ch <= static_cast<CharType>('z'))) {
            return Letters[ch - 'a'];
        }

        if ((static_cast<CharType>('0') <= ch) &&
            (ch <= static_cast<CharType>('9'))) {
            return Numbers[ch - '0'];
        }

        return nullptr;
    }

    Led& led_;
    InBuf buf_;
    Timer timer_;
};

