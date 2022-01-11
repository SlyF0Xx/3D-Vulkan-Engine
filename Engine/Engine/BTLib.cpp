#include "BTLib.h"

void BehaviourActionFunctions::OnInit(BehaviourAction& action, diffusion::ScriptComponentState& script)
{
    CallLuaFunction(script, action.ActionOnInit.c_str());
}

void BehaviourActionFunctions::OnFinish(BehaviourAction& action, diffusion::ScriptComponentState& script)
{
    CallLuaFunction(script, action.ActionOnFinish.c_str());
}

void BehaviourActionFunctions::OnAbort(BehaviourAction& action, diffusion::ScriptComponentState& script)
{
    CallLuaFunction(script, action.ActionOnAbort.c_str());
}

void BehaviourActionFunctions::CallLuaFunction(diffusion::ScriptComponentState& script, const char* name)
{
    auto ref = luabridge::getGlobal(script.m_state, name);
    auto ret = ref();
    if (!ret.wasOk()) {
        std::string err = ret.errorMessage();
        std::cerr << err;
    }
}

BehaviourState BehaviourActionFunctions::OnTick(diffusion::ScriptComponentState& script, BehaviourAction& action,
    float delta)
{
    auto ref = luabridge::getGlobal(script.m_state, action.ActionOnTick.c_str());
    auto ret = ref(delta);
    if (!ret.wasOk()) {
        std::string err = ret.errorMessage();
        std::cerr << err;
    }
    if (!(ret.size() && ret[0].isValid() && ret[0].isString()))
    {
        std::cerr << "Wrong return value, string {Success | Failed | Running} required";
    }
    auto state = ret[0].tostring();
    if (state == "Success")
    {
        return BehaviourState::Succeeded; 
    } else if (state == "Failed")
    {
        return BehaviourState::Failed;
    } else if (state == "Running")
    {
        return BehaviourState::Running;
    } else
    {
        std::cerr << "Wrong return string, {Success | Failed | Running} required";
    }
    return BehaviourState::Failed;
}

bool BehaviourActionFunctions::ConditionFunction(diffusion::ScriptComponentState& script, BehaviourCondition condition)
{
    auto ref = luabridge::getGlobal(script.m_state, condition.ConditionName.c_str());
    auto ret = ref();
    if (!ret.wasOk()) {
        std::string err = ret.errorMessage();
        std::cerr << err;
    }
    if (!(ret.size() && ret[0].isValid() && ret[0].isBool()))
    {
        std::cerr << "Wrong return value, boolean {true | false} required" << std::endl;
        std::cerr << ret.size() << std::endl;
        std::cerr << ret[0] << std::endl;
        std::cerr << ret[0].isValid() << std::endl;
        std::cerr << ret[0].isBool() << std::endl;
    }
    bool state = ret[0];
    return state;
}
