#pragma once

#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#define PROFILE_SCOPE() ZoneScoped
#define PROFILE_FUNCTION() ZoneScopedN(__FUNCTION__)
#define PROFILE_NAMED(name) ZoneScopedN(name)
#define FRAME_MARK() FrameMark
#else
#define PROFILE_SCOPE()
#define PROFILE_FUNCTION()
#define PROFILE_NAMED(name)
#define FRAME_MARK()
#endif