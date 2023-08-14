#pragma once

#if defined(INK_BUILD_SHARED_LIBRARY)
#    define InkExport __declspec(dllexport)
#elif defined(INK_SHARED_LIBRARY)
#    define InkExport __declspec(dllimport)
#else
#    define InkExport
#endif
