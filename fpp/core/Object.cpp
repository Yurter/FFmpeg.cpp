#include "Object.hpp"
#include <cassert>

namespace fpp {

    Object::Object()
        : _name { "Object" }
        , _inited { false } {
    }

    void Object::setName(const std::string& name) {
        assert(name.size() < 16);
        _name = name;
    }

    std::string Object::name() const {
//        /* fpp::Object */
//        const std::string name_with_namespace = typeid(*this).name();
//        /* Object */
//        const std::string name_without_namespace = name_with_namespace.substr(name_with_namespace.find_last_of(':') + 1);
//        return name_without_namespace;
        /* ^ Криво работает в момент удаления объекта: имя меняется по мере срабатывания деструкторов */
        /* ^ мб устанавливать единожды... */
        return _name;
    }

    bool Object::is(const std::string& name) const {
        return _name == name;
    }

    std::string Object::toString() const {
        return "[" + _name + ":" + std::to_string(int64_t(this)) + "]";
//        return "{" + std::string(typeid(*this).name()) + ":" + std::to_string(int64_t(this)) + "}";
    }

} // namespace fpp
