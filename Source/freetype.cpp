#include <algorithm>
#include <cctype>
#include <sstream>
#include <cmath>
#include <fstream>
#include <iterator>

#include "freetype.h"

namespace Font {

// trim from start (in place)
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}

struct FreetypeLibraryWrapper {
    FT_Library ftlib_;

    FreetypeLibraryWrapper() {
        FT_Error error = FT_Init_FreeType(&ftlib_);
        if (error) {
            throw(std::exception("Initialize Freetype library failed!"));
        }
    }

    ~FreetypeLibraryWrapper() {
        if (ftlib_) {
            FT_Done_FreeType(ftlib_);
        }
    }
};

static FreetypeLibraryWrapper FreetypeLibrary;

FreetypeFontFaceInfo::FreetypeFontFaceInfo(FT_Long _face_idx, FT_Long _instance_idx, FT_Long _id, FT_Face _face)
    : face_idx(_face_idx),
      instance_idx(_instance_idx),
      id(_id),
      face(_face)

{}

FreetypeFontFaceInfo::~FreetypeFontFaceInfo() { FT_Done_Face(face); }

std::shared_ptr<GlyphBitmapInfo> FreetypeFontFaceInfo::GetGlyphBitmapInfo(uint32_t char_code, uint32_t size) {
    FT_Set_Pixel_Sizes(face, size, 0);
    FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);
    if (glyph_index == 0) {
        return nullptr;
    }
    FT_Long error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
    if (error) {
        return nullptr;
    }
    std::shared_ptr<GlyphBitmapInfo> glyph_result = std::make_shared<GlyphBitmapInfo>();
    glyph_result->error_code = GlyphErrorCode::Success;
    glyph_result->width = face->glyph->bitmap.width;
    glyph_result->height = face->glyph->bitmap.rows;
    glyph_result->bearing_x = face->glyph->bitmap_left;
    glyph_result->bearing_y = face->glyph->bitmap_top;
    glyph_result->advance = static_cast<int32_t>(face->glyph->advance.x >> 6);
    glyph_result->emoji = false;
    glyph_result->data = malloc(face->glyph->bitmap.width * face->glyph->bitmap.rows * 4 * sizeof(char));
    unsigned char* pixels = (unsigned char*)(glyph_result->data);
    for (unsigned int row = 0; row < face->glyph->bitmap.rows; ++row) {
        for (unsigned int col = 0; col < face->glyph->bitmap.width; ++col) {
            auto value = face->glyph->bitmap.buffer[row * face->glyph->bitmap.pitch + col];
            auto pixel_offest = row * face->glyph->bitmap.width * 4 + col * 4;
            pixels[pixel_offest] = value;
            pixels[pixel_offest + 1] = value;
            pixels[pixel_offest + 2] = value;
            pixels[pixel_offest + 3] = value;
        }
    }
    return glyph_result;
}

FreetypeFontFace::FreetypeFontFace(void* ttf_data, uint32_t size) {
    ttf_data_ = ttf_data;
    FT_Long num_faces = 0;
    FT_Long num_instances = 0;

    FT_Long face_idx = 0;
    FT_Long instance_idx = 0;

    FT_Open_Args args;
    args.flags = FT_OPEN_MEMORY;
    args.memory_base = (FT_Byte*)ttf_data;
    args.memory_size = size;

    do {
        FT_Face face;
        FT_Long id = (instance_idx << 16) + face_idx;

        FT_Error error = FT_Open_Face(FreetypeLibrary.ftlib_, &args, id, &face);
        if (error) {
            continue;
        }
        faces.emplace_back(std::make_shared<FreetypeFontFaceInfo>(face_idx, instance_idx, id, face));

        num_faces = face->num_faces;
        num_instances = face->style_flags >> 16;
        if (face->family_name) {
            font_family = face->family_name;
        }

        if (instance_idx < num_instances)
            instance_idx++;
        else {
            face_idx++;
            instance_idx = 0;
        }

    } while (face_idx < num_faces);
}

FreetypeFontFace ::~FreetypeFontFace() {
    if (ttf_data_) {
        free(ttf_data_);
    }
}

void* FreetypeFont::Create(const std::string& name, uint32_t size) {
    std::string name_ = trim_copy(name);
    if (name_ == "") {
        name_ = "Arial";
    }
    std::string delimiter = " ";

    std::vector<std::string> nameTokens;
    {
        size_t pos = 0;
        std::string token;
        while ((pos = name_.find(delimiter)) != std::string::npos) {
            token = name_.substr(0, pos);
            nameTokens.push_back(token);
            name_.erase(0, pos + delimiter.length());
        }
        nameTokens.push_back(name_);
    }

    bool bold = false;
    bool italic = false;
    bool parseStyle = true;
    while (nameTokens.size() > 0 && parseStyle) {
        parseStyle = false;
        if (nameTokens.back() == "Italic") {
            nameTokens.pop_back();
            italic = true;
            parseStyle = true;
        } else if (nameTokens.back() == "Bold") {
            nameTokens.pop_back();
            bold = true;
            parseStyle = true;
        }
    }

    std::ostringstream imploded;
    std::copy(nameTokens.begin(), nameTokens.end(), std::ostream_iterator<std::string>(imploded, " "));
    name_ = trim_copy(imploded.str());

    FTTFontInfo* ret = new FTTFontInfo;
    ret->name = name_;
    ret->size = size;
    ret->bold = bold;
    ret->italic = italic;
    return ret;
}

std::string FreetypeFont::Load(void* ttf_data, uint32_t size) {
    void* ttf_data_ = malloc(size * sizeof(char));
    memcpy(ttf_data_, ttf_data, size);
    std::shared_ptr<FreetypeFontFace> face = std::make_shared<FreetypeFontFace>(ttf_data_, size);
    if (face->font_family != "") {
        auto iter = font_info_.find(face->font_family);
        if (iter == font_info_.end()) {
            std::vector<std::shared_ptr<FreetypeFontFace>> faces;
            iter = font_info_.emplace(face->font_family, faces).first;
        }
        iter->second.emplace_back(face);
        return face->font_family;
    }
    return "";
}

LinkedList<GlyphBitmapInfo> FreetypeFont::GetGlyphBitmap(void* font, uint32_t char_code) {
    FTTFontInfo* info = (FTTFontInfo*)font;
    LinkedList<GlyphBitmapInfo> result;
    if (font_info_.find(info->name) != font_info_.end()) {
        for (auto& famliy : font_info_.find(info->name)->second) {
            for (auto face : famliy->faces) {
                if (info->bold && !(face->face->style_flags & FT_STYLE_FLAG_BOLD)) {
                    continue;
                }
                if (info->italic && !(face->face->style_flags & FT_STYLE_FLAG_ITALIC)) {
                    continue;
                }
                auto r = face->GetGlyphBitmapInfo(char_code, info->size);
                if (r) {
                    result.add(*r);
                    return result;
                }
            }
        }
        if (info->bold || info->italic) {
            for (auto& famliy : font_info_.find(info->name)->second) {
                for (auto face : famliy->faces) {
                    auto r = face->GetGlyphBitmapInfo(char_code, info->size);
                    if (r) {
                        result.add(*r);
                        return result;
                    }
                }
            }
        }
    }
    return result;
}

void FreetypeFont::Destroy(void* font) { delete (FTTFontInfo*)(font); }

static FreetypeFont FreetypeFontInstance;

FreetypeFont& GetFreetypeFontInstance() { return FreetypeFontInstance; }

}  // namespace Font