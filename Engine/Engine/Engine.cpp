// Engine.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "Engine.h"

#include <iostream>

// This is the constructor of a class that has been exported.
CEngine::CEngine()
{
    return;
}

void CEngine::hello()
{
    std::cout << "hello world!" << std::endl;
}
