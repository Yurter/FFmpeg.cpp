#pragma once
#include <fpp/core/async/AsyncObject.hpp>
#include <queue>
#include <atomic>

#define KILOBYTES * 1024
#define MEGABYTES * 1024 * 1024

namespace fpp {

    template <class T>
    class AsyncQueue : public AsyncObject<std::queue<T>> {

    public:

        using SizeGetter = std::function<uint64_t(const T&)>;

        AsyncQueue(uint64_t queue_capacity = 50, SizeGetter&& get_item_size = [](const T&) -> uint64_t { return 1; }) :
            _queue_size(0)
            , _queue_capacity(queue_capacity)
            , _stop_wait(false)
            , _get_item_size(get_item_size) {}

        virtual ~AsyncQueue() override { stop_wait(); }

        [[nodiscard]]
        bool push(const T& item) {
            std::lock_guard<std::mutex> lock(this->_mutex);
            if (not_enough_storage(item)) { return false; }
            push_and_notify(item);
            return true;
        }

        [[nodiscard]]
        bool pop(T& item) {
            std::lock_guard<std::mutex> lock(this->_mutex);
            if (this->_data.empty()) { return false; }
            pop_and_notify(item);
            return true;
        }

        int64_t force_push(const T& item) {
            std::lock_guard<std::mutex> lock(this->_mutex);
            int64_t popped_items_count = 0;
            while (not_enough_storage(item)) {
                this->_data.pop();
                popped_items_count++;
            }
            this->_data.push(item);
            _condition_variable_pushed.notify_one();
            return popped_items_count;
        }

        int64_t pop_while(std::function<bool(const T&)>&& pred) {
            std::lock_guard<std::mutex> lock(this->_mutex);
            int64_t popped_items_count = 0;
            while (!this->_data.empty() && pred(this->_data.front())) {
                this->_data.pop();
                popped_items_count++;
            }
            return popped_items_count;
        }

        [[nodiscard]]
        bool wait_and_push(const T& data) {
            std::unique_lock<std::mutex> lock(this->_mutex);
            if (not_enough_storage(data)) {
                _condition_variable_popped.wait(lock, [&]() {
                    return !not_enough_storage(data) || _stop_wait;
                });
            }

            if (_stop_wait) {
                _stop_wait = false;
                return false;
            }

            push_and_notify(data);
            return true;
        }

        [[nodiscard]]
        bool wait_and_pop(T& data) {
            std::unique_lock<std::mutex> lock(this->_mutex);
            if (!this->_data.empty()) {
                pop_and_notify(data);
                return true;
            }

            _condition_variable_pushed.wait(lock, [&]() {
                return !this->_data.empty() || _stop_wait;
            });

            if (_stop_wait) {
                _stop_wait = false;
                return false;
            }

            pop_and_notify(data);
            return true;
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(this->_mutex);
            return this->_data.empty();
        }

        int64_t length() const {
            std::lock_guard<std::mutex> lock(this->_mutex);
            return this->_data.size();
        }

        int64_t size() const {
            std::lock_guard<std::mutex> lock(this->_mutex);
            return _queue_size;
        }

        void clear() {
            std::lock_guard<std::mutex> lock(this->_mutex);
            std::queue<T> empty;
            std::swap(this->_data, empty);
        }

        void set_capacity(uint64_t queue_capacity) {
            _queue_capacity = queue_capacity;
        }

        void stop_wait() {
            _stop_wait = true;
            _condition_variable_pushed.notify_one();
            _condition_variable_popped.notify_one();
        }

    private:

        void push_and_notify(const T& item) {
            this->_data.push(item);
            _condition_variable_pushed.notify_one();
            _queue_size += _get_item_size(item);
        }

        void pop_and_notify(T& item) {
            item = T(this->_data.front());
            this->_data.pop();
            _condition_variable_popped.notify_one();
            _queue_size -= _get_item_size(item);
        }

        bool not_enough_storage(const T& item) const {
            return (_queue_size + _get_item_size(item)) > _queue_capacity;
        }

    private:

        uint64_t            _queue_size;
        uint64_t            _queue_capacity;
        std::atomic_bool    _stop_wait;
        std::function<uint64_t(const T&)>   _get_item_size;
        std::condition_variable _condition_variable_pushed;
        std::condition_variable _condition_variable_popped;

    };

} // namespace fpp

