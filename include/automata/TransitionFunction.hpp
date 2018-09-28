#pragma once

#include <functional>
#include <memory>
#include <type_traits>
#include <unordered_map>

#include "memory/Storage.hpp"
#include "memory/StorageView.hpp"
#include "memory/Utils.hpp"

namespace hx {

enum class TransitionType : std::uint8_t { MAP, FUNC, TABLE };

class TransitionFunction {
public:
    virtual std::size_t get(std::size_t state, std::size_t alphabet) const = 0;
    virtual TransitionType getType() const = 0;
    virtual ~TransitionFunction() = default;

    std::size_t operator()(std::size_t state, std::size_t alphabet) const {
        return this->get(state, alphabet);
    }
};

class TransitionFunctionTable : public TransitionFunction {
public:
    TransitionFunctionTable(std::size_t numberOfStates, std::size_t numberOfActions)
        : _numberOfStates(numberOfStates)
        , _numberOfActions(numberOfActions)
        , _tableStorage(sizeof(std::size_t) * _numberOfStates * _numberOfActions)
        , _tableView(&_tableStorage, _numberOfStates, _numberOfActions){};

    virtual ~TransitionFunctionTable() = default;

    std::size_t get(std::size_t state, std::size_t action) const override {
        return _tableView(state, action);
    }

    void set(std::size_t inputState, std::size_t inputAction, std::size_t outputState) {
        _tableView(inputState, inputAction) = outputState;
    }

    TransitionType getType() const override { return TransitionType::TABLE; }

private:
    const std::size_t _numberOfStates;
    const std::size_t _numberOfActions;

    hx::memory::Storage _tableStorage;
    hx::memory::StorageView<std::size_t, 2> _tableView;
};

class TransitionFunctionTableIndirect : public TransitionFunction {
public:
    TransitionFunctionTableIndirect(std::unique_ptr<TransitionFunction> base,
                                    hx::memory::SmallLookupBimap bimapStates,
                                    hx::memory::SmallLookupBimap bimapAction)
        : _base(std::move(base)), _bimapStates(bimapStates), _bimapActions(bimapAction){};

    std::size_t get(std::size_t state, std::size_t action) const override {
        return _bimapStates.from(
            _base->get(_bimapStates.to(state), _bimapActions.to(action)));
    }

    TransitionType getType() const override { return _base->getType(); }

private:
    std::unique_ptr<TransitionFunction> _base;
    hx::memory::SmallLookupBimap _bimapStates;
    hx::memory::SmallLookupBimap _bimapActions;
};

class TransitionFunctionMap : public TransitionFunction {
public:
    struct pair_hash {
        std::size_t operator()(const std::pair<std::size_t, std::size_t> &p) const {
            return std::hash<std::size_t>()(p.first ^ p.second);
        }
    };

    using ContainerMap = std::unordered_map<std::pair<std::size_t, std::size_t>,
                                            std::size_t,
                                            TransitionFunctionMap::pair_hash>;

    TransitionFunctionMap(ContainerMap map) : transition(map){};

    std::size_t get(std::size_t state, std::size_t alphabet) const override {
        return transition.at(std::make_pair(state, alphabet));
    }
    TransitionType getType() const override { return TransitionType::MAP; }

private:
    ContainerMap transition;
};

class TransitionFunctionFunc : public TransitionFunction {
public:
    using FunctionType = std::function<std::size_t(std::size_t, std::size_t)>;
    TransitionFunctionFunc(FunctionType function) : f(function){};

    std::size_t get(std::size_t state, std::size_t alphabet) const override {
        return f(state, alphabet);
    }
    TransitionType getType() const override { return TransitionType::FUNC; }

private:
    FunctionType f;
};
}// namespace hx