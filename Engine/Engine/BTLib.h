#pragma once

#include <vector>
#include <map>
#include <string>
#include <variant>
#include <cmath>
#include <nlohmann/json.hpp>
#include <entt/entity/registry.hpp>

#include "BaseComponents/ScriptComponent.h"

struct Sequence;
struct Selector;
struct Parallel;
struct InvertDecorator;
struct RepeatNDecorator;
struct RetryNDecorator;
struct ForceSucceededDecorator;
struct CooldownDecorator;
struct BehaviourAction;
struct BehaviourCondition;

enum class BehaviourState
{
    Running,
    Failed,
    Succeeded
};

enum class ParallelPolicy
{
    SuccessAny,
    SuccessAll,
    FailAny,
    FailAll,
    RequireAny,
    RequireAll
};

using Behaviour = std::variant<Sequence, Selector, Parallel, InvertDecorator, RepeatNDecorator, RetryNDecorator,
                               ForceSucceededDecorator, CooldownDecorator, BehaviourAction, BehaviourCondition>;

struct BTCooldownComponent
{
    bool* onCooldownRef;
    float remains;
};

struct Sequence
{
    Sequence() = default;

    Sequence(std::vector<Behaviour>& childs) : childs(childs)
    {
    };
    size_t taskIndex{};
    std::vector<Behaviour> childs = {};
};

struct Selector
{
    Selector() = default;

    Selector(std::vector<Behaviour>& childs) : childs(childs)
    {
    };
    size_t taskIndex{};
    std::vector<Behaviour> childs = {};
};

struct Parallel
{
    Parallel() = default;

    Parallel(
        std::vector<Behaviour>& childs,
        ParallelPolicy policy = ParallelPolicy::RequireAny) : childs(childs), policy(policy)
    {
    };

    ParallelPolicy policy;
    std::vector<Behaviour> childs = {};
};

struct InvertDecorator
{
    InvertDecorator() = default;

    InvertDecorator(std::vector<Behaviour>& child) : child(child)
    {
    };
    std::vector<Behaviour> child;
};

struct RepeatNDecorator
{
    RepeatNDecorator() = default;

    RepeatNDecorator(std::vector<Behaviour>& childs, size_t N = 0) : child(child), N(N)
    {
    };
    size_t N = 1;
    size_t I = 0;
    std::vector<Behaviour> child;
};

struct RetryNDecorator
{
    RetryNDecorator() = default;

    RetryNDecorator(std::vector<Behaviour>& childs, size_t N) : child(child), N(N)
    {
    };
    size_t N = 1;
    size_t I = 0;
    std::vector<Behaviour> child;
};

struct ForceSucceededDecorator
{
    ForceSucceededDecorator() = default;

    ForceSucceededDecorator(std::vector<Behaviour>& childs) : child(child)
    {
    };
    std::vector<Behaviour> child;
};

struct CooldownDecorator
{
    CooldownDecorator() = default;

    CooldownDecorator(std::vector<Behaviour>& childs, float capacity = 0.0f) : child(child), capacity(capacity)
    {
    };
    float capacity = 0.0f;
    bool isOnCooldown = false;
    std::vector<Behaviour> child;
};

struct BehaviourAction
{
    BehaviourAction() = default;

    BehaviourAction(std::string& ActionName)
    {
    };
    std::string ActionOnInit = "";
    std::string ActionOnFinish = "";
    std::string ActionOnTick = "";
    std::string ActionOnAbort = "";
};

struct BehaviourCondition
{
    BehaviourCondition() = default;

    BehaviourCondition(std::string& ConditionName) : ConditionName(ConditionName)
    {
    };
    std::string ConditionName = "";
};

namespace BehaviourActionFunctions
{
    void OnInit(BehaviourAction& action, diffusion::ScriptComponentState& script);
    void OnFinish(BehaviourAction& action, diffusion::ScriptComponentState& script);
    void OnAbort(BehaviourAction& action, diffusion::ScriptComponentState& script);
    void CallLuaFunction(diffusion::ScriptComponentState& script, const char* name);
    BehaviourState OnTick(diffusion::ScriptComponentState& script, BehaviourAction& action, float delta);
    bool ConditionFunction(diffusion::ScriptComponentState& script, BehaviourCondition condition);
}

struct ToJsonFunctions
{
    nlohmann::json& operator()(const Sequence& behaviour)
    {
        auto childs = nlohmann::json::array();
        for (auto& child : behaviour.childs)
        {
            childs.push_back(std::visit(ToJsonFunctions{}, child));
        }
        nlohmann::json j = {{"type", 0}, {"childs", childs}};
        return j;
    }

    nlohmann::json& operator()(const Selector& behaviour)
    {
        auto childs = nlohmann::json::array();
        for (auto& child : behaviour.childs)
        {
            childs.push_back(std::visit(ToJsonFunctions{}, child));
        }
        nlohmann::json j = {{"type", 1}, {"childs", childs}};
        return j;
    }

    nlohmann::json& operator()(const Parallel& behaviour)
    {
        auto childs = nlohmann::json::array();
        for (auto& child : behaviour.childs)
        {
            childs.push_back(std::visit(ToJsonFunctions{}, child));
        }
        nlohmann::json j = {{"type", 2}, {"policy", (int)behaviour.policy}, {"childs", childs}};
        return j;
    }

    nlohmann::json& operator()(const InvertDecorator& behaviour)
    {
        auto child = std::visit(ToJsonFunctions{}, behaviour.child[0]);
        nlohmann::json j = {{"type", 3}, {"child", child}};
        return j;
    }

    nlohmann::json& operator()(const RepeatNDecorator& behaviour)
    {
        auto child = std::visit(ToJsonFunctions{}, behaviour.child[0]);
        nlohmann::json j = {{"type", 4}, {"N", behaviour.N}, {"child", child}};
        return j;
    }

    nlohmann::json& operator()(const RetryNDecorator& behaviour)
    {
        auto child = std::visit(ToJsonFunctions{}, behaviour.child[0]);
        nlohmann::json j = {{"type", 5}, {"N", behaviour.N}, {"child", child}};
        return j;
    }

    nlohmann::json& operator()(const ForceSucceededDecorator& behaviour)
    {
        auto child = std::visit(ToJsonFunctions{}, behaviour.child[0]);
        nlohmann::json j = {{"type", 6}, {"child", child}};
        return j;
    }

    nlohmann::json& operator()(const CooldownDecorator& behaviour)
    {
        auto child = std::visit(ToJsonFunctions{}, behaviour.child[0]);
        nlohmann::json j = {{"type", 7}, {"capacity", behaviour.capacity}, {"child", child}};
        return j;
    }

    nlohmann::json& operator()(const BehaviourAction& behaviour)
    {
        nlohmann::json events = {
            behaviour.ActionOnInit,
            behaviour.ActionOnTick,
            behaviour.ActionOnAbort,
            behaviour.ActionOnFinish
        };

        nlohmann::json j = {{"type", 8}, {"events", events}};
        return j;
    }

    nlohmann::json& operator()(const BehaviourCondition& behaviour)
    {
        nlohmann::json j = {{"type", 9}, {"condition", behaviour.ConditionName}};
        return j;
    }
};

struct NonTickFunction
{
    void operator()(Sequence& Behaviour)
    {
        auto& behaviour = Behaviour.childs[Behaviour.taskIndex];
        std::visit(NonTickFunction{script, func}, behaviour);
    }

    void operator()(Selector& Behaviour)
    {
        auto& behaviour = Behaviour.childs[Behaviour.taskIndex];
        std::visit(NonTickFunction{script, func}, behaviour);
    }

    void operator()(Parallel& Behaviour)
    {
        for (auto& behaviour : Behaviour.childs)
        {
            std::visit(NonTickFunction{script, func}, behaviour);
        }
    }

    void operator()(InvertDecorator& Behaviour)
    {
        auto& behaviour = Behaviour.child[0];
        std::visit(NonTickFunction{script, func}, behaviour);
    }

    void operator()(RepeatNDecorator& Behaviour)
    {
        auto& behaviour = Behaviour.child[0];
        std::visit(NonTickFunction{script, func}, behaviour);
    }

    void operator()(RetryNDecorator& Behaviour)
    {
        auto& behaviour = Behaviour.child[0];
        std::visit(NonTickFunction{script, func}, behaviour);
    }

    void operator()(ForceSucceededDecorator& Behaviour)
    {
        auto& behaviour = Behaviour.child[0];
        std::visit(NonTickFunction{script, func}, behaviour);
    }

    void operator()(CooldownDecorator& Behaviour)
    {
        if (Behaviour.isOnCooldown)
        {
            return;
        }
        if (func == BehaviourActionFunctions::OnFinish || func == BehaviourActionFunctions::OnAbort)
        {
            Behaviour.isOnCooldown = true;
            auto entity = registry->create();
            registry->emplace<BTCooldownComponent>(entity, BTCooldownComponent{
                                                       &Behaviour.isOnCooldown, Behaviour.capacity
                                                   });
        }
        auto& behaviour = Behaviour.child[0];
        std::visit(NonTickFunction{script, func}, behaviour);
    }

    void operator()(BehaviourCondition& Behaviour)
    {
    }

    void operator()(BehaviourAction& behaviour)
    {
        func(behaviour, *script);
    };

    diffusion::ScriptComponentState* script;
    void (*func)(BehaviourAction& action, diffusion::ScriptComponentState& script);
    static ::entt::registry* registry;
};

//BehaviourState SequenceAndSelector()

struct BehaviourTickFunctions
{
    BehaviourState operator()(Sequence& Sequence)
    {
        size_t& CurrentTaskIndex = Sequence.taskIndex;
        auto& childs = Sequence.childs;
        const size_t taksQuantity = childs.size();
        auto& behaviour = childs[CurrentTaskIndex];
        auto State = std::visit(BehaviourTickFunctions{script, deltaSeconds}, behaviour);

        if (State != BehaviourState::Running)
        {
            std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnFinish}, behaviour);
        }

        switch (State)
        {
        case BehaviourState::Succeeded:
            CurrentTaskIndex++;
            if (CurrentTaskIndex < taksQuantity)
            {
                auto& behaviour = childs[CurrentTaskIndex];
                std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnInit}, behaviour);
                return BehaviourState::Running;
            }
            else
            {
                CurrentTaskIndex = 0;
                return BehaviourState::Succeeded;
            }
        case BehaviourState::Failed:
            CurrentTaskIndex = 0;
            return BehaviourState::Failed;
        case BehaviourState::Running:
            return BehaviourState::Running;
        }
        return BehaviourState::Running;
    }

    BehaviourState operator()(Selector& Selector)
    {
        size_t& CurrentTaskIndex = Selector.taskIndex;
        auto& childs = Selector.childs;
        const size_t taksQuantity = childs.size();
        auto& behaviour = childs[CurrentTaskIndex];
        const auto State = std::visit(BehaviourTickFunctions{script, deltaSeconds}, behaviour);

        if (State != BehaviourState::Running)
        {
            std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnFinish}, behaviour);
        }
        switch (State)
        {
        case BehaviourState::Succeeded:
            CurrentTaskIndex = 0;
            return BehaviourState::Succeeded;
        case BehaviourState::Failed:
            CurrentTaskIndex++;
            if (CurrentTaskIndex < taksQuantity)
            {
                auto& behaviour = childs[CurrentTaskIndex];
                std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnInit}, behaviour);
                return BehaviourState::Running;
            }
            else
            {
                CurrentTaskIndex = 0;
                return BehaviourState::Failed;
            }
        case BehaviourState::Running:
            return BehaviourState::Running;
        }
        return BehaviourState::Running;
    };

    BehaviourState operator()(Parallel& Parallel)
    {
        bool RequiredAllState = true;
        auto& childs = Parallel.childs;
        for (auto& child : childs)
        {
            const auto State = std::visit(BehaviourTickFunctions{script, deltaSeconds}, child);
            switch (Parallel.policy)
            {
            case ParallelPolicy::SuccessAny:
                if (State == BehaviourState::Succeeded)
                {
                    std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnAbort}, child);
                    return BehaviourState::Succeeded;
                }
                break;
            case ParallelPolicy::SuccessAll:
                if (State == BehaviourState::Failed)
                {
                    std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnAbort}, child);
                    return BehaviourState::Failed;
                }
                break;
            case ParallelPolicy::FailAny:
                if (State == BehaviourState::Failed)
                {
                    std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnAbort}, child);
                    return BehaviourState::Succeeded;
                }
                break;
            case ParallelPolicy::FailAll:
                if (State == BehaviourState::Succeeded)
                {
                    std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnAbort}, child);
                    return BehaviourState::Failed;
                }
                break;
            case ParallelPolicy::RequireAny:
                if (State != BehaviourState::Running)
                {
                    std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnAbort}, child);
                    return BehaviourState::Succeeded;
                }
                break;
            case ParallelPolicy::RequireAll:
                if (State != BehaviourState::Running)
                {
                    RequiredAllState = false;
                }
                break;
            }
        }
        if (Parallel.policy == ParallelPolicy::RequireAll && RequiredAllState)
        {
            return BehaviourState::Succeeded;
        }
        return BehaviourState::Running;
    }

    BehaviourState operator()(InvertDecorator& InvertDecorator)
    {
        auto& child = InvertDecorator.child[0];
        const auto State = std::visit(BehaviourTickFunctions{script, deltaSeconds}, child);
        if (State != BehaviourState::Running)
        {
            std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnFinish}, child);
            return State == BehaviourState::Succeeded ? BehaviourState::Failed : BehaviourState::Succeeded;
        }
        return BehaviourState::Running;
    };

    BehaviourState operator()(RepeatNDecorator& RepeadNDecorator)
    {
        auto& child = RepeadNDecorator.child[0];
        auto& iteration = RepeadNDecorator.I;
        auto& N = RepeadNDecorator.N;
        const auto State = std::visit(BehaviourTickFunctions{script, deltaSeconds}, child);
        if (State != BehaviourState::Running)
        {
            std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnFinish}, child);
        }

        switch (State)
        {
        case BehaviourState::Succeeded:
            iteration++;
            if (iteration < N)
            {
                std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnInit}, child);
                return BehaviourState::Running;
            }
            else
            {
                iteration = 0;
                return BehaviourState::Succeeded;
            }
        case BehaviourState::Failed:
            iteration = 0;
            return BehaviourState::Failed;
        case BehaviourState::Running:
            return BehaviourState::Running;
        }
        return BehaviourState::Running;
    }

    BehaviourState operator()(RetryNDecorator& RetryNDecorator)
    {
        auto& child = RetryNDecorator.child[0];
        auto& iteration = RetryNDecorator.I;
        auto& N = RetryNDecorator.N;
        const auto State = std::visit(BehaviourTickFunctions{script, deltaSeconds}, child);
        if (State != BehaviourState::Running)
        {
            std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnFinish}, child);
        }

        switch (State)
        {
        case BehaviourState::Succeeded:
            iteration = 0;
            return BehaviourState::Succeeded;
        case BehaviourState::Failed:
            iteration++;
            if (iteration < N)
            {
                std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnInit}, child);
                return BehaviourState::Running;
            }
            else
            {
                iteration = 0;
                return BehaviourState::Failed;
            }
        case BehaviourState::Running:
            return BehaviourState::Running;
        }
        return BehaviourState::Running;
    }

    BehaviourState operator()(ForceSucceededDecorator& ForceSucceededDecorator)
    {
        auto& child = ForceSucceededDecorator.child[0];
        const auto State = std::visit(BehaviourTickFunctions{script, deltaSeconds}, child);
        if (State != BehaviourState::Running)
        {
            std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnFinish}, child);
            return BehaviourState::Succeeded;
        }
        return BehaviourState::Running;
    }

    BehaviourState operator()(CooldownDecorator& CooldownDecorator)
    {
        if (CooldownDecorator.isOnCooldown)
        {
            return BehaviourState::Failed;
        }
        auto& child = CooldownDecorator.child[0];
        const auto State = std::visit(BehaviourTickFunctions{script, deltaSeconds}, child);
        if (State != BehaviourState::Running)
        {
            std::visit(NonTickFunction{script, &BehaviourActionFunctions::OnFinish}, child);
            //TODO call cooldowns tick if capacity > 0
        }
        return State;
    };

    BehaviourState operator()(BehaviourCondition& BehaviourCondition)
    {
        return BehaviourActionFunctions::ConditionFunction(*script, BehaviourCondition)
                   ? BehaviourState::Succeeded
                   : BehaviourState::Failed;
    }

    BehaviourState operator()(BehaviourAction& BehaviourAction)
    {
        return BehaviourActionFunctions::OnTick(*script, BehaviourAction, deltaSeconds);
    }

    diffusion::ScriptComponentState* script;
    float deltaSeconds = 0.0f;
};

inline void to_json(nlohmann::json& j, const Behaviour& behaviour)
{
    j = std::visit(ToJsonFunctions{}, behaviour);
}

inline void from_json(const nlohmann::json& j, Behaviour& behaviour)
{
    auto const index = j.at("type").get<int>();

    switch (index)
    {
    case 0:
        {
            Sequence sequence{};
            for (auto& child : j.at("childs"))
            {
                Behaviour childBehaviour;
                from_json(child, childBehaviour);
                sequence.childs.push_back(childBehaviour);
            }
            behaviour = sequence;
            break;
        }
    case 1:
        {
            Selector selector;
            for (auto& child : j.at("childs"))
            {
                Behaviour childBehaviour;
                from_json(child, childBehaviour);
                selector.childs.push_back(childBehaviour);
            }
            behaviour = selector;
            break;
        }
    case 2:
        {
            Parallel parallel;
            parallel.policy = (ParallelPolicy)j.at("policy").get<int>();
            for (auto& child : j.at("childs"))
            {
                Behaviour childBehaviour;
                from_json(child, childBehaviour);
                parallel.childs.push_back(childBehaviour);
            }
            behaviour = parallel;
            break;
        }
    case 3:
        {
            InvertDecorator decorator;
            Behaviour childBehaviour;
            from_json(j.at("child"), childBehaviour);
            decorator.child.push_back(childBehaviour);
            behaviour = decorator;
            break;
        }
    case 4:
        {
            RepeatNDecorator repeat;
            repeat.N = j.at("N").get<int>();

            Behaviour childBehaviour;
            from_json(j.at("child"), childBehaviour);
            repeat.child.push_back(childBehaviour);
            behaviour = repeat;
            break;
        }
    case 5:
        {
            RetryNDecorator retry;
            retry.N = j.at("N").get<int>();

            Behaviour childBehaviour;
            from_json(j.at("child"), childBehaviour);
            retry.child.push_back(childBehaviour);
            behaviour = retry;
            break;
        }
    case 6:
        {
            ForceSucceededDecorator forceSucceeded;

            Behaviour childBehaviour;
            from_json(j.at("child"), childBehaviour);
            forceSucceeded.child.push_back(childBehaviour);
            behaviour = forceSucceeded;
            break;
        }
    case 7:
        {
            CooldownDecorator cooldown;

            cooldown.capacity = j.at("capacity").get<float>();

            Behaviour childBehaviour;
            from_json(j.at("child"), childBehaviour);
            cooldown.child.push_back(childBehaviour);
            behaviour = cooldown;
            break;
        }
    case 8:
        {
            auto& events = j.at("events");
            BehaviourAction action;
            action.ActionOnInit = events[0];
            action.ActionOnFinish = events[1];
            action.ActionOnTick = events[2];
            action.ActionOnAbort = events[3];
            behaviour = action;
            break;
        }
    case 9:
        {
            BehaviourCondition condition;
            condition.ConditionName = j.at("condition");
            behaviour = condition;
            break;
        }
    default:
        throw std::runtime_error{""};
    }
}


struct BTComponent
{
    Behaviour root;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BTComponent, root);
};
