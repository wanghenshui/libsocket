/**
 * Created by Jian Chen
 * @since  2017.01.15
 * @author Jian Chen <admin@chensoft.com>
 * @link   http://chensoft.com
 */
#pragma once

#include <socket/base/basic_event.hpp>
#include <functional>
#include <chrono>

namespace chen
{
    class timer : public basic_event
    {
    public:
        timer();
        timer(std::function<void ()> cb);
        ~timer();

    public:
        /**
         * Invoke callback only once after a period of time
         */
        void timeout(const std::chrono::nanoseconds &value);

        /**
         * Invoke callback only once in a future calendar date
         */
        void future(const std::chrono::high_resolution_clock::time_point &value);

        /**
         * Invoke callback repeatedly after a period of time
         */
        void interval(const std::chrono::nanoseconds &value);

    public:
        /**
         * Native handle value
         */
        virtual handle_t native() const
        {
            return invalid_handle;  // no use in timer
        }

        /**
         * Timer properties
         */
        bool repeat() const
        {
            return this->_repeat;
        }

        std::chrono::nanoseconds cycle() const
        {
            return this->_cycle;
        }

        std::chrono::high_resolution_clock::time_point alarm() const
        {
            return this->_alarm;
        }

        /**
         * Calculate init value
         */
        void adjust(const std::chrono::high_resolution_clock::time_point &now);

        /**
         * Update timer value
         * @return true if timer expired after update, otherwise false
         */
        bool update(const std::chrono::high_resolution_clock::time_point &now);

    public:
        /**
         * Bind & Emit callback
         */
        void bind(std::function<void ()> cb);
        void emit();

    protected:
        /**
         * At least one event has occurred
         */
        virtual void onEvent(reactor &loop, int type);

    private:
        bool _repeat = false;
        std::function<void ()> _notify;

        std::chrono::nanoseconds _cycle;  // value when call timeout or interval
        std::chrono::high_resolution_clock::time_point _alarm;  // the next trigger time point
    };
}