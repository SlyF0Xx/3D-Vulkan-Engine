#pragma once

#include <vector>
#include <map>
#include <string>
#include <cmath>


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

class BlackBoard
{
private:
    std::map<std::string, int> intValues;
    std::map<std::string, float> floatValues;
    std::map<std::string, bool> boolValues;

public:
    int GetIntByName(std::string name)
    {
        return intValues[name];
    }
    
    float GetFloatByName(std::string name)
    {
        return floatValues[name];
    }
    bool GetBooleanByName(std::string name)
    {
        return boolValues[name];
    }

    void SetIntByName(std::string name, int Value)
    {
        intValues[name] = Value;
    }

    void SetFloatByName(std::string name, float Value)
    {
        floatValues[name] = Value;
    }

    void SetBooleanByName(std::string name, bool Value)
    {
        boolValues[name] = Value;
    }
};

class Behaviour
{
private:
    /* data */
public:
    virtual BehaviourState tick(float delta) = 0;
    virtual void onInit() = 0;
    virtual void onFinish() = 0;
    virtual void Abort() = 0;
    virtual ~Behaviour();
};

class Composite : public Behaviour
{
private:
    /* data */
protected:
    std::vector<Behaviour *> composites;
};

class Sequence : public Composite
{
private:
    /* data */
    int CurrentTaskIndex = 0;

public:
    BehaviourState tick(float delta) override;
    void onInit() override;
    void Abort() override;
};

class Selector : public Composite
{
private:
    /* data */
    int CurrentTaskIndex = 0;

public:
    BehaviourState tick(float delta) override;
    void onInit() override;
    void Abort() override;
};

class Parallel : public Composite
{
private:
    ParallelPolicy policy = ParallelPolicy::SuccessAny;

public:
    BehaviourState tick(float delta) override;
    void onInit() override;
    void Abort() override;
};

class Decorator : public Behaviour
{
protected:
    Behaviour *Decorated;

public:
    Decorator(Behaviour &behaviour) : Decorated(&behaviour){};
    void onInit() override;
    void onFinish() override;
    void Abort() override;
};

class CoolDown : public Decorator
{
private:
    float Capacity = 0.0f;
    float CurrentTime = 0.0f;
    bool isCooldownInProgress = false;

public:
    CoolDown(Behaviour& behaviour, float Capacity);
    BehaviourState tick(float delta) override;
    void onFinish() override;
    void CooldownTick(float delta);
};

class RepeatN : public Decorator
{
private:
    int N = 1;
    int iteration = 0;

public:
    RepeatN(Behaviour& behaviour, int N = 1);
    void onInit() override;
    BehaviourState tick(float delta) override;
};

class RetryN : public Decorator
{
private:
    int N = 1;
    int iteration = 0;

public:
    RetryN(Behaviour& behaviour, int N = 1);
    void onInit() override;
    BehaviourState tick(float delta) override;
};

class Invert : public Decorator
{
private:
public:
    Invert(Behaviour &behaviour) : Decorator(behaviour){};
    BehaviourState tick(float delta) override;
};

class ForceSucceeded : public Decorator
{
private:
public:
    ForceSucceeded(Behaviour &behaviour) : Decorator(behaviour){};
    BehaviourState tick(float delta) override;
};

class Condition : public Behaviour
{
protected:
    BlackBoard *blackBoard;
    bool (*Check)(BlackBoard &blackBoard);

public:
    Condition(BlackBoard& blackBoard, bool (*cond)(BlackBoard& blackBoard));
    BehaviourState tick(float delta) override;;
};

class Action : public Behaviour
{
protected:
    BlackBoard *blackBoard;
    BehaviourState (*ActionFunction)(BlackBoard &blackBoard, float delta);

public:
    Action(BlackBoard& blackBoard, BehaviourState (*action)(BlackBoard& blackBoard, float delta));
    BehaviourState tick(float delta) override;
};
