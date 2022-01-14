#pragma once

#include "font.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace Font {

class FreetypeFontFaceInfo {
public:
    FreetypeFontFaceInfo() = delete;
    FreetypeFontFaceInfo(const FreetypeFontFaceInfo&) = delete;
    FreetypeFontFaceInfo(FT_Long face_idx, FT_Long instance_idx, FT_Long id, FT_Face face);
    ~FreetypeFontFaceInfo();
    std::shared_ptr<GlyphBitmapInfo> GetGlyphBitmapInfo(uint32_t char_code, uint32_t size);
    FT_Long face_idx;
    FT_Long instance_idx;
    FT_Long id;
    FT_Face face;
};

class FreetypeFontFace {
public:
    FreetypeFontFace() = delete;
    FreetypeFontFace(const FreetypeFontFace&) = delete;

    FreetypeFontFace(void* ttf_data, uint32_t size);

    ~FreetypeFontFace();

    std::string font_family = "";
    std::vector<std::shared_ptr<FreetypeFontFaceInfo>> faces;

private:
    void* ttf_data_;
};

class FreetypeFont {
public:
    FontInfo* Create(const std::string& name, uint32_t size);
    std::string Load(void* ttf_data, uint32_t size);
    LinkedList<GlyphBitmapInfo> GetGlyphBitmap(void* font, uint32_t char_code);
    void Destroy(FontInfo* font);

private:
    std::unordered_map<std::string, std::vector<std::shared_ptr<FreetypeFontFace>>> font_info_;
};
FreetypeFont& GetFreetypeFontInstance();
}  // namespace Font