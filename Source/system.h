#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "font.h"

namespace Font {
struct FontInfo;
std::vector<GlyphBitmapInfo> GetGlyphBitmapSystem(FontInfo* font, uint32_t char_code, bool enbaleRemap, bool enableFallback);
}  // namespace Font