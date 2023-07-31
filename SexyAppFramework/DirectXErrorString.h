#ifndef __SEXY_DIRECTXERRORSTRING_H__
#define __SEXY_DIRECTXERRORSTRING_H__
#include <ddraw.h>
#include <string>

namespace Sexy {
std::string GetDirectXErrorString(HRESULT theResult);
} // namespace Sexy

#endif