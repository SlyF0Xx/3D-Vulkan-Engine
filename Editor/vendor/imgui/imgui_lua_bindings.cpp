#include "imgui_lua_bindings.h"

// THIS IS FOR LUA 5.3 although you can make a few changes for other versions



// define ENABLE_IM_LUA_END_STACK
// to keep track of end and begins and clean up the imgui stack
// if lua errors


// define this global before you call RunString or LoadImGuiBindings
// lua_State* lState;

#ifdef ENABLE_IM_LUA_END_STACK
// Stack for imgui begin and end
std::deque<int> endStack;
static void AddToStack(int type) {
  endStack.push_back(type);
}

static void PopEndStack(int type) {
  if (!endStack.empty()) {
    endStack.pop_back(); // hopefully the type matches
  }
}

static void ImEndStack(int type);

#endif

// Example lua run string function
// returns NULL on success and error string on error
const char * RunString(lua_State* lState, const char* szLua) {
  if (!lState) {
    fprintf(stderr, "You didn't assign the global lState, either assign that or refactor LoadImguiBindings and RunString\n");
  }

  int iStatus = luaL_loadstring(lState, szLua);
  if(iStatus) {
    return lua_tostring(lState, -1);
    //fprintf(stderr, "Lua syntax error: %s\n", lua_tostring(lState, -1));
    //return;
  }
#ifdef ENABLE_IM_LUA_END_STACK
  endStack.clear();
#endif
  iStatus = lua_pcall( lState, 0, 0, 0 );

#ifdef ENABLE_IM_LUA_END_STACK
  bool wasEmpty = endStack.empty();
  while(!endStack.empty()) {
    ImEndStack(endStack.back());
    endStack.pop_back();
  }

#endif
  if( iStatus )
  {
      return lua_tostring(lState, -1);
      //fprintf(stderr, "Error: %s\n", lua_tostring( lState, -1 ));
      //return;
  }
#ifdef ENABLE_IM_LUA_END_STACK
  else if (!wasEmpty) {
    return "Script didn't clean up imgui stack properly";
  }
#endif
  return NULL;
}


void LoadImguiBindings(lua_State* lState) {
  if (!lState) {
    fprintf(stderr, "You didn't assign the global lState, either assign that or refactor LoadImguiBindings and RunString\n");
  }
  lua_newtable(lState);
  luaL_setfuncs(lState, imguilib, 0);
  PushImguiEnums(lState, "constant");
  lua_setglobal(lState, "imgui");
}
