#pragma once
#include <Windows.h>
#include <usp10.h>
#include <AtlBase.h>
#include <AtlCom.h>
#include <mlang.h>

#include <iostream>
#include <vector>
#include <string>
#include "font.h"

namespace Font {
struct FTTFontInfo;
std::vector<GlyphBitmapInfo> GetGlyphBitmapSystem(FTTFontInfo* font, uint32_t char_code);
}  // namespace Font