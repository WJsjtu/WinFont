#include "font.h"
#include "freetype.h"
#include "system.h"

namespace Font {

String::String() {
    m_data = new char[1];
    m_data[0] = '\0';
    m_size = 0;
}
String::String(const char* str, size_t size) {
    if (size == 0) {
        size = strlen(str);
    }
    m_size = size;
    m_data = new char[m_size + 1];
    memcpy(m_data, str, m_size);
    m_data[m_size] = '\0';
}
String::String(const String& str) {
    m_size = str.m_size;
    m_data = new char[m_size + 1];
    memcpy(m_data, str.m_data, str.m_size);
    m_data[m_size] = '\0';
}
String::~String() { delete[] m_data; }
String& String::operator=(const String& str) {
    if (this == &str) return *this;
    delete[] m_data;
    m_size = (str.m_size);
    m_data = new char[m_size + 1];
    memcpy(m_data, str.m_data, str.m_size);
    m_data[m_size] = '\0';
    return *this;
}
bool String::operator==(const char* str) { return m_size == strlen(str) ? strncmp(m_data, str, m_size) == 0 : false; }
bool String::operator==(const String& str) { return m_size == str.m_size ? strncmp(m_data, str.m_data, m_size) == 0 : false; }
const char* String::data() const { return m_data; }
const size_t String::size() const { return m_size; }

void* CreateTTFFont(const String& name, uint32_t size) { return GetFreetypeFontInstance().Create(name.data(), size); }
void DestroyTTFFont(void* font) { GetFreetypeFontInstance().Destroy(font); }
String LoadTTFFont(void* ttf_data, uint32_t size) {
    std::string name = GetFreetypeFontInstance().Load(ttf_data, size);
    return String(name.c_str());
}

GlyphBitmapInfo GetDefaultGlyph() {
    GlyphBitmapInfo result{};
    result.error_code = GlyphErrorCode::Success;
    result.width = 4;
    result.height = 4;
    result.bearing_x = result.bearing_y = 0;
    result.emoji = false;
    result.data = malloc(4 * 4 * 4);
    memset(result.data, 125, 4 * 4 * 4);
    return result;
};

LinkedList<GlyphBitmapInfo> GetGlyphBitmapInfo(void* font, uint32_t char_code) {
    auto freetype = GetFreetypeFontInstance().GetGlyphBitmap(font, char_code);
    if (freetype.size()) {
        return freetype;
    }
    auto sys = GetGlyphBitmapSystem((FTTFontInfo*)font, char_code);
    LinkedList<GlyphBitmapInfo> result;
    for (auto& sysr : sys) {
        result.add(sysr);
    }
    return result;
}
}  // namespace Font