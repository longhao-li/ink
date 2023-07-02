#pragma once

#ifdef INK_BUILD_DLL
#    define InkApi __declspec(dllexport)
#else
#    define InkApi __declspec(dllimport)
#endif
