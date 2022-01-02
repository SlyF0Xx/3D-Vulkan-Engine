#include "BTLib.h"

// Sequence
BehaviourState Sequence::tick(float delta)
{
    const auto State = composites[CurrentTaskIndex]->tick(delta);
    if (State != BehaviourState::Running)
    {
        composites[CurrentTaskIndex]->onFinish();
    }
    if (State == BehaviourState::Succeeded)
    {
        CurrentTaskIndex++;
        if (CurrentTaskIndex < composites.size())
        {
            composites[CurrentTaskIndex]->onInit();
            return BehaviourState::Running;
        }
        else
        {
            CurrentTaskIndex = 0;
            return BehaviourState::Succeeded;
        }
    }
    else
    {
        return State;
    }
}

void Sequence::onInit()
{
    CurrentTaskIndex = 0;
    composites[CurrentTaskIndex]->onInit();
}

void Sequence::Abort()
{
    composites[CurrentTaskIndex]->Abort();
}

// Selector

BehaviourState Selector::tick(float delta)
{
    const auto State = composites[CurrentTaskIndex]->tick(delta);
    if (State != BehaviourState::Running)
    {
        composites[CurrentTaskIndex]->onFinish();
    }
    if (State == BehaviourState::Failed)
    {
        CurrentTaskIndex++;
        if (CurrentTaskIndex < composites.size())
        {
            composites[CurrentTaskIndex]->onInit();
            return BehaviourState::Running;
        }
        else
        {
            CurrentTaskIndex = 0;
            return BehaviourState::Failed;
        }
    }
    else
    {
        return State;
    }
}

void Selector::onInit()
{
    CurrentTaskIndex = 0;
    composites[CurrentTaskIndex]->onInit();
}

void Selector::Abort()
{
    composites[CurrentTaskIndex]->Abort();
}

// Parallel

BehaviourState Parallel::tick(float delta)
{
    bool RequiredAllState = true;
    for (auto& composite : composites)
    {
        const auto State = composite->tick(delta);
        switch (policy)
        {
        case ParallelPolicy::SuccessAny:
            if (State == BehaviourState::Succeeded)
            {
                Abort();
                return BehaviourState::Succeeded;
            }
            break;
        case ParallelPolicy::SuccessAll:
            if (State == BehaviourState::Failed)
            {
                Abort();
                return BehaviourState::Failed;
            }
            break;
        case ParallelPolicy::FailAny:
            if (State == BehaviourState::Failed)
            {
                Abort();
                return BehaviourState::Succeeded;
            }
            break;
        case ParallelPolicy::FailAll:
            if (State == BehaviourState::Succeeded)
            {
                Abort();
                return BehaviourState::Failed;
            }
            break;
        case ParallelPolicy::RequireAny:
            if (State != BehaviourState::Running)
            {
                Abort();
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
    if (policy == ParallelPolicy::RequireAll && RequiredAllState)
    {
        Abort();
        return BehaviourState::Succeeded;
    }
    return BehaviourState::Running;
}

void Parallel::onInit()
{
    for (auto& composite : composites)
    {
        composite->onInit();
    }
}

void Parallel::Abort()
{
    for (auto& composite : composites)
    {
        composite->Abort();
    }
}

// Decorator

void Decorator::onInit()
{
    Decorated->onInit();
}

void Decorator::onFinish()
{
    Decorated->onFinish();
}

void Decorator::Abort()
{
    Decorated->Abort();
}

// CoolDown

CoolDown::CoolDown(Behaviour& behaviour, float Capacity): Decorator(behaviour)
{
    this->Capacity = std::max(0.0f, Capacity);
}

BehaviourState CoolDown::tick(float delta)
{
    if (isCooldownInProgress)
    {
        return BehaviourState::Failed;
    }
    const auto State = Decorated->tick(delta);
    if (State != BehaviourState::Running)
    {
        onFinish();
    }
    return State;
}

void CoolDown::onFinish()
{
    CurrentTime = 0.0f;
    isCooldownInProgress = Capacity > 0.f ? false : true;
    Decorator::onFinish();
}

void CoolDown::CooldownTick(float delta)
{
    CurrentTime += delta;
    if (CurrentTime >= Capacity)
    {
        isCooldownInProgress = true;
    }
}

// RepeatN

RepeatN::RepeatN(Behaviour& behaviour, int N): Decorator(behaviour)
{
    this->N = std::max(1, N);
}

void RepeatN::onInit()
{
    iteration = 0;
    Decorator::onInit();
}

BehaviourState RepeatN::tick(float delta)
{
    const auto State = Decorated->tick(delta);
    if (State != BehaviourState::Running)
    {
        Decorated->onFinish();
    }
    if (State == BehaviourState::Succeeded)
    {
        iteration++;
        if (iteration < N)
        {
            Decorated->onInit();
        }
        else
        {
            iteration = 0;
            return BehaviourState::Succeeded;
        }
    }
    else
    {
        return State;
    }
    return BehaviourState::Running;
}

// RetryN

RetryN::RetryN(Behaviour& behaviour, int N): Decorator(behaviour)
{
    this->N = std::max(1, N);
}

void RetryN::onInit()
{
    iteration = 0;
    Decorator::onInit();
}

BehaviourState RetryN::tick(float delta)
{
    const auto State = Decorated->tick(delta);
    if (State != BehaviourState::Running)
    {
        Decorated->onFinish();
    }
    if (State == BehaviourState::Failed)
    {
        iteration++;
        if (iteration < N)
        {
            Decorated->onInit();
        }
        else
        {
            iteration = 0;
            return BehaviourState::Failed;
        }
    }
    else
    {
        return State;
    }
    return BehaviourState::Running;
}

BehaviourState Invert::tick(float delta)
{
    const auto State = Decorated->tick(delta);
    if (State != BehaviourState::Running)
    {
        Decorated->onFinish();
        return State == BehaviourState::Succeeded ? BehaviourState::Failed : BehaviourState::Succeeded;
    }
    return BehaviourState::Running;
}

// ForceSucceeded

BehaviourState ForceSucceeded::tick(float delta)
{
    const auto State = Decorated->tick(delta);
    if (State != BehaviourState::Running)
    {
        Decorated->onFinish();
        return BehaviourState::Succeeded;
    }
    return BehaviourState::Running;
}

// ForceSucceeded

Condition::Condition(BlackBoard& blackBoard, bool (* cond)(BlackBoard& blackBoard))
{
    this->blackBoard = &blackBoard;
    Check = cond;
}

BehaviourState Condition::tick(float delta)
{
    return Check(*blackBoard) ? BehaviourState::Succeeded : BehaviourState::Failed;
}

// Action

Action::Action(BlackBoard& blackBoard, BehaviourState (* action)(BlackBoard& blackBoard, float delta))
{
    this->blackBoard = &blackBoard;
    ActionFunction = action;
}

BehaviourState Action::tick(float delta)
{
    return ActionFunction(*blackBoard, delta);
}

