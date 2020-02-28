#pragma once
#include <list>
#include <mutex>
#include <iterator>

namespace fpp {

    template <class Type>
    class AsyncList {

    public:

        AsyncList() = default;
        virtual ~AsyncList() { clear(); }

        AsyncList(const AsyncList& other) {
            this->_list = other._list;
        }

        AsyncList(const AsyncList&& other) {
            this->_list = std::move(other._list);
        }

        AsyncList& operator=(const AsyncList& other) {
            std::lock_guard<std::mutex> lock(_list_mutex);
            if (this != &other) {
                this->_list = other._list;
            }
            return *this;
        }

        AsyncList& operator=(const AsyncList&& other) {
            std::lock_guard<std::mutex> lock(_list_mutex);
            if (this != &other) {
                 this->_list = std::move(other._list);
            }
            return *this;
        }

        Type& operator[](const size_t index) { //TODO
            std::lock_guard<std::mutex> lock(_list_mutex);
            auto result = _list.begin();
            std::advance(result, index);
            return *result;
        }

        size_t size() const {
            std::lock_guard<std::mutex> lock(_list_mutex);
            return _list.size();
        }

        void push_back(const Type& value) {
            std::lock_guard lock(_list_mutex);
            _list.push_back(value);
        }

        void push_back(const Type&& value) {
            std::lock_guard lock(_list_mutex);
            _list.push_back(value);
        }

        void remove_if(std::function<bool(const Type&)> condition) {
            std::lock_guard lock(_list_mutex);
            _list.remove_if(condition);
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(_list_mutex);
            return _list.empty();
        }

        void access(const std::function<void(std::list<Type>&)>&& foo) {
            std::lock_guard<std::mutex> lock(_list_mutex);
            foo(_list);
        }

        auto for_each(std::function<void(Type&)> foo) {
            std::lock_guard<std::mutex> lock(_list_mutex);
            for (auto& item : _list) {
                foo(item);
            }
        }

        auto for_each(std::function<void(const Type&)> foo) const {
            std::lock_guard<std::mutex> lock(_list_mutex);
            for (const auto& item : _list) {
                foo(item);
            }
        }

        auto for_each(std::function<void(Type& current, Type& next)> foo) { //TODO кривой код, непонятный интерфейс
            std::lock_guard<std::mutex> lock(_list_mutex);
            for (auto item = _list.begin(); item != _list.end(); item++) {
                auto next = std::next(item);
                if (next == _list.end()) { return; }
                foo(*item, *next);
            }
        }

        void clear() {
            std::lock_guard<std::mutex> lock(_list_mutex);
            std::list<Type> empty;
            std::swap(_list, empty);
        }

    private:

        std::list<Type>     _list;
        mutable std::mutex  _list_mutex;

    };

}
