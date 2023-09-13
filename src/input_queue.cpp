#include "input_queue.hpp"

namespace lc32sim {

    InputQueue StdInputQueue { std::cin };

    // Make sure to initialize everything that needs to be. All references must
    // be initialized, and we have to populate the thread somehow.
    InputQueue::InputQueue(std::istream &input) noexcept : input_stream_{input} {
        this->producer_thread_ = std::thread(&InputQueue::produce, this);
    }

    // The producer thread. Constantly grab from the input stream and add to the
    // queue. Don't exceed the max size though.
    void InputQueue::produce() noexcept {
        while (true) {
            // Get a character from the input stream. This blocks until one is
            // available, but its okay since it's in a separate thread.
            char new_char;
            this->input_stream_.get(new_char);

            // If something went wrong with the input stream, die
            if (!this->input_stream_.good())
                return;

            // Lock 'em up. This will be released at the end of this iteration.
            std::unique_lock<std::mutex> queue_lock { this->queue_mutex_ };
            // Check that the queue is small enough for us to add new elements.
            // If it isn't, sleep until it is.
            this->queue_on_poll_.wait(queue_lock, [this]() noexcept -> bool {
                return this->queue_.size() < InputQueue::QUEUE_MAX_SIZE;
            });

            // Add to the queue. Notify the main thread if it is waiting on us
            // to add to the queue.
            this->queue_.push(new_char);
            this->queue_on_push_.notify_one();
        }
    }

    std::optional<char> InputQueue::try_poll() noexcept {
        // Lock 'em up. This will be released when this function exits.
        std::lock_guard<std::mutex> queue_guard { this->queue_mutex_ };

        // If empty, return None
        if (this->queue_.empty())
            return std::nullopt;

        // Otherwise, get and remove the first element
        char ret = this->queue_.front();
        this->queue_.pop();
        // Remember to signal the other thread, that might be waiting on us to
        // remove stuff so it can put it in the back.
        this->queue_on_poll_.notify_one();
        // Finally return
        return ret;
    }

    char InputQueue::poll() noexcept {
        // Lock 'em up. This will be released when this function exits.
        std::unique_lock<std::mutex> queue_lock { this->queue_mutex_ };

        // Wait until the queue is not empty
        this->queue_on_push_.wait(queue_lock, [this]() noexcept -> bool {
            return !this->queue_.empty();
        });

        // Get and remove the first element
        char ret = this->queue_.front();
        this->queue_.pop();
        // Remember to signal the other thread, that might be waiting on us to
        // remove stuff so it can put it in the back.
        this->queue_on_poll_.notify_one();
        // Finally return
        return ret;
    }

}
