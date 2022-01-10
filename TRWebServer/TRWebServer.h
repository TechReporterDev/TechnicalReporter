#pragma once
#include "TRCore\Core.h"
#include "TRCore\DirectClient.h"

TR::Core::Core* getServerCore();
void setServerCore(TR::Core::Core* core);