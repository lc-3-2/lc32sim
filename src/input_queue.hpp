/*!
    \file input_queue.hpp
    \brief Queue for reading input asynchronously

    The C++ standard library doesn't have any for polling input buffers. You
    have to block on them until data becomes available. This is problematic for
    us, since we want the KBSR to signal whether data is available. The standard
    solution is to run a separate thread to consume the input and drop it into a
    buffer. This file implements that approach.
*/

#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>

namespace lc32sim {

    /*!
        \brief Class that to transform an input stream into a queue

        This class spawns a separate thread to pull from the provided input
        buffer. The results are dumped into a shared queue, which this class
        then provides an interface to read from.
    */
    class InputQueue {
        public:
            /*!
                \brief Wraps the input stream

                Note that the input stream should not be read by other sources.
                This is because another thread is already reading from it. If
                you try to read from it elsewhere, you might get a later
                character and this queue might drop characters.

                \param [in] in The stream to wrap
            */
            InputQueue(std::istream &input) noexcept;

            /*!
                \return The next character to be read, or an empty value if no
                such character exists yet
            */
            std::optional<char> try_poll() noexcept;

            /*!
                \return The next character to be read, blocking until available
            */
            char poll() noexcept;

        private:
            std::istream &input_stream_;

            std::thread producer_thread_;
            void produce() noexcept;

            std::queue<char> queue_;
            std::mutex queue_mutex_;
            std::condition_variable queue_on_poll_;
            std::condition_variable queue_on_push_;

            static const size_t QUEUE_MAX_SIZE = 0x100;
    };

    /*!
        \brief InputQueue for std::cin
    */
    extern InputQueue StdInputQueue;

} // namespace lc32sim
