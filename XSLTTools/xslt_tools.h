#pragma once

/* make sure init function is exported on win32 */
#if defined(_WIN32)
#ifdef XSLT_TOOLS_DYNAMIC_LIB
#        define PLUGINPUBFUN __declspec(dllexport)
#      else
#        define PLUGINPUBFUN __declspec(dllimport)
#endif
#endif

PLUGINPUBFUN void xslt_tools_init(void);