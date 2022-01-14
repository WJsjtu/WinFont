#include <algorithm>
#include <sstream>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iterator>

#include <Windows.h>
#include <usp10.h>
#include <AtlBase.h>
#include <AtlCom.h>
#include <mlang.h>

#include "system.h"
#include "freetype.h"

namespace Font {

std::string UnicodeToUTF8(int unicode) {
    std::string s;
    if (unicode >= 0 && unicode <= 0x7f)  // 7F(16) = 127(10)
    {
        s = static_cast<char>(unicode);
        return s;
    } else if (unicode <= 0x7ff)  // 7FF(16) = 2047(10)
    {
        unsigned char c1 = 192, c2 = 128;

        for (int k = 0; k < 11; ++k) {
            if (k < 6)
                c2 |= (unicode % 64) & (1 << k);
            else
                c1 |= (unicode >> 6) & (1 << (k - 6));
        }

        s = c1;
        s += c2;

        return s;
    } else if (unicode <= 0xffff)  // FFFF(16) = 65535(10)
    {
        unsigned char c1 = 224, c2 = 128, c3 = 128;

        for (int k = 0; k < 16; ++k) {
            if (k < 6)
                c3 |= (unicode % 64) & (1 << k);
            else if (k < 12)
                c2 |= (unicode >> 6) & (1 << (k - 6));
            else
                c1 |= (unicode >> 12) & (1 << (k - 12));
        }

        s = c1;
        s += c2;
        s += c3;

        return s;
    } else if (unicode <= 0x1fffff)  // 1FFFFF(16) = 2097151(10)
    {
        unsigned char c1 = 240, c2 = 128, c3 = 128, c4 = 128;

        for (int k = 0; k < 21; ++k) {
            if (k < 6)
                c4 |= (unicode % 64) & (1 << k);
            else if (k < 12)
                c3 |= (unicode >> 6) & (1 << (k - 6));
            else if (k < 18)
                c2 |= (unicode >> 12) & (1 << (k - 12));
            else
                c1 |= (unicode >> 18) & (1 << (k - 18));
        }
        s = c1;
        s += c2;
        s += c3;
        s += c4;
        return s;
    } else if (unicode <= 0x3ffffff)  // 3FFFFFF(16) = 67108863(10)
    {
        // actually, there are no 5-bytes unicodes
    } else if (unicode <= 0x7fffffff)  // 7FFFFFFF(16) = 2147483647(10)
    {
        // actually, there are no 6-bytes unicodes
    } else {  // incorrect unicode (< 0 or > 2147483647)
    }

    return "";
}

std::wstring UnicodeToWideString(uint32_t unicode) {
    std::string utf8Str = UnicodeToUTF8(unicode);
    if (utf8Str.length() == 0) {
        return L"";
    }
    UINT nLen = MultiByteToWideChar(CP_UTF8, NULL, utf8Str.data(), -1, NULL, NULL);
    WCHAR* wszBuffer = new WCHAR[nLen + 1];
    nLen = MultiByteToWideChar(CP_UTF8, NULL, utf8Str.data(), -1, wszBuffer, nLen);
    wszBuffer[nLen] = 0;
    std::wstring result = wszBuffer;
    delete[] wszBuffer;
    return result;
}

int CALLBACK MetaFileProc(_In_ HDC hdc, _In_reads_(nHandles) HANDLETABLE FAR* lpht, _In_ CONST ENHMETARECORD* record, _In_ int nHandles, _In_opt_ LPARAM logfont) {
    if (record->iType == EMR_EXTCREATEFONTINDIRECTW) {
        auto ptr = (const EMREXTCREATEFONTINDIRECTW*)record;
        *(LOGFONTW*)logfont = ptr->elfw.elfLogFont;
    }
    return 1;
}

std::string WideStringToMultiString(const std::wstring& wstr) {
    UINT nLen = WideCharToMultiByte(CP_ACP, NULL, wstr.data(), -1, NULL, NULL, NULL, NULL);
    CHAR* szBuffer = new CHAR[nLen + 1];
    nLen = WideCharToMultiByte(CP_ACP, NULL, wstr.data(), -1, szBuffer, nLen, NULL, NULL);
    szBuffer[nLen] = 0;
    return std::string(szBuffer);
}

std::wstring MultiStringToWideString(const std::string& wstr) {
    UINT nLen = MultiByteToWideChar(CP_ACP, NULL, wstr.data(), -1, NULL, NULL);
    WCHAR* szBuffer = new WCHAR[nLen + 1];
    nLen = MultiByteToWideChar(CP_ACP, NULL, wstr.data(), -1, szBuffer, nLen);
    szBuffer[nLen] = 0;
    return std::wstring(szBuffer);
}

static LinkedList<SystemFontInfo> GlobalSystemFontsInfo;

bool initialized = false;

int CALLBACK EnumFontFamiliesExCallbackStyle(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD FontType, LPARAM lParam) {
    if (FontType & TRUETYPE_FONTTYPE) {
        LOGFONT lf = lpelfe->elfLogFont;
        LinkedList<SystemFontInfo>* systemFonts = (LinkedList<SystemFontInfo>*)lParam;
        std::string fontName;
        std::stringstream ss;
        ss << lpelfe->elfFullName;
        fontName = ss.str();
        bool found = false;
        for (size_t i = 0; i < systemFonts->size(); i++) {
            if ((*systemFonts)[i].name == fontName.c_str()) {
                found = true;
                (*systemFonts)[i].info.add(*(SystemFontFaceInfo*)&lf);
                break;
            }
        }
        if (!found) {
            SystemFontInfo fontInfo;
            fontInfo.name = fontName.c_str();
            fontInfo.info.add(*(SystemFontFaceInfo*)&lf);
            systemFonts->add(fontInfo);
        }
    }
    return 1;
}

int CALLBACK EnumFontFamiliesExCallback(ENUMLOGFONTEX* lpelfe, NEWTEXTMETRICEX* lpntme, DWORD FontType, LPARAM lParam) {
    LOGFONT lf = lpelfe->elfLogFont;
    HDC hdc = GetDC(NULL);
    int result = EnumFontFamiliesExA(hdc, &lf, (FONTENUMPROC)EnumFontFamiliesExCallbackStyle, lParam, 0);
    DeleteDC(hdc);
    return 1;
}

LinkedList<SystemFontInfo>& GetSystemFonts(bool refresh) {
    if (refresh) {
        if (initialized) {
            GlobalSystemFontsInfo.clear();
        }
    }
    if (!initialized) {
        initialized = true;
        HDC hdc = GetDC(NULL);
        EnumFontFamiliesExA(hdc, NULL, (FONTENUMPROC)(EnumFontFamiliesExCallback), (LPARAM)&GlobalSystemFontsInfo, 0);
        DeleteDC(hdc);
    }
    return GlobalSystemFontsInfo;
}

static void BlitGray8Bitmap(unsigned char* dstBitmap, size_t dstRowPitch, const uint8_t* srcBitmap, size_t srcRowPitch, uint32_t width, uint32_t height) {
    unsigned char* dst = dstBitmap;
    const uint8_t* src = srcBitmap;
    for (uint32_t iy = 0; iy < height; ++iy) {
        for (uint32_t ix = 0; ix < width; ++ix) {
            const uint8_t val = src[ix];
            // Input range is 0..64, output is 0..255.
            auto value = val >= 64 ? 255 : val * 4;
            auto pixel_offest = iy * width * 4 + ix * 4;
            dst[pixel_offest] = value;
            dst[pixel_offest + 1] = value;
            dst[pixel_offest + 2] = value;
            dst[pixel_offest + 3] = value;
        }
        src += srcRowPitch;
    }
}

// Align given value up to nearest multiply of align value.
// For example: AlignUp(11, 8) = 16. Use types like uint32_t, uint64_t as T.
template <typename T>
constexpr inline T AlignUp(T val, T align) {
    return (val + align - 1) / align * align;
}

std::vector<wchar_t> GetFontGlpyhIndicies(HDC hdc, HFONT font, uint32_t char_code, LOGFONTW* logFont) {
    std::vector<wchar_t> glyphIndices;
    std::wstring unicodeString = UnicodeToWideString(char_code);
    HGDIOBJ oldfont = ::SelectObject(hdc, font);
    ::GetObjectW(font, sizeof(LOGFONTW), logFont);
    SCRIPT_CACHE sc = NULL;
    SCRIPT_FONTPROPERTIES fp = {sizeof(fp)};
    ::ScriptGetFontProperties(hdc, &sc, &fp);
    ::ScriptFreeCache(&sc);
    int nchSzStr = static_cast<int>(wcslen(unicodeString.data()));
    GCP_RESULTSW gcpResults = {sizeof(GCP_RESULTSW)};
    gcpResults.nGlyphs = nchSzStr;
    wchar_t* wstrGlyphMemory = static_cast<wchar_t*>(calloc(wcslen(unicodeString.data()) + 1, sizeof(wchar_t)));
    gcpResults.lpGlyphs = wstrGlyphMemory;
    ::GetCharacterPlacementW(hdc, unicodeString.data(), nchSzStr, 0, &gcpResults, GCP_GLYPHSHAPE);
    bool hasValidGlyph = true;
    for (UINT i = 0; i < gcpResults.nGlyphs; i++) {
        wchar_t n = gcpResults.lpGlyphs[i];
        glyphIndices.push_back(n);
        if (char_code == 32) {
            if (n == fp.wgInvalid || n == fp.wgDefault) {
                hasValidGlyph = false;
                break;
            }
        } else {
            if (n == fp.wgBlank || n == fp.wgInvalid || n == fp.wgDefault) {
                hasValidGlyph = false;
                break;
            }
        }
    }
    free(wstrGlyphMemory);
    ::SelectObject(hdc, oldfont);
    if (!hasValidGlyph) {
        glyphIndices.clear();
    }
    return glyphIndices;
}

std::wstring GetSystemFontName(FontInfo* font, uint32_t char_code, std::vector<wchar_t>& glyphIndices, bool enbaleRemap, bool enableFallback) {
    glyphIndices.clear();
    bool bComInitted = SUCCEEDED(::CoInitialize(NULL));
    std::wstring result = L"";
    if (true)  // Need this so that CoUninitialize() is called after destructors below
    {
        CComPtr<IMultiLanguage> imultilang;
        CComPtr<IMLangFontLink> ifont;
        CComPtr<IMLangCodePages> icodepages;
        if (FAILED(imultilang.CoCreateInstance(CLSID_CMultiLanguage))) return L"";
        if (FAILED(imultilang.QueryInterface(&ifont))) return L"";
        if (FAILED(imultilang.QueryInterface(&icodepages))) return L"";

        std::wstring unicodeString = UnicodeToWideString(char_code);
        const wchar_t* single_codepoint = unicodeString.data();
        std::shared_ptr<LOGFONTW> finalFont = nullptr;
        size_t szchLn = wcslen(single_codepoint);
        BOOL bSurrogatePair = szchLn >= 1 && IS_SURROGATE_PAIR(single_codepoint[0], single_codepoint[1]);
        std::wstring mainFontName = MultiStringToWideString(font->name.data());

        std::shared_ptr<LOGFONTW> mainLogFont = std::make_shared<LOGFONTW>();
        HFONT reMappedFont = NULL;
        std::shared_ptr<LOGFONTW> remappedLogFont = std::make_shared<LOGFONTW>();
        std::vector<wchar_t> mainFontGlpyhIndicies;

        HDC hdc = ::CreateCompatibleDC(NULL);
        if (!hdc) {
            goto RELEASE_DC_AND_FONT;
        }
        HFONT mainFont = ::CreateFontW(font->size, 0, 0, 0, font->bold ? FW_BOLD : 0, font->italic, 0, 0, 0, 0, 0, 0, 0, mainFontName.data());
        if (!mainFont) {
            goto RELEASE_DC_AND_FONT;
        }
        mainFontGlpyhIndicies = GetFontGlpyhIndicies(hdc, mainFont, char_code, mainLogFont.get());
        if (mainFontGlpyhIndicies.size() > 0) {
            glyphIndices = mainFontGlpyhIndicies;
            result = wcslen(mainLogFont->lfFaceName) ? mainLogFont->lfFaceName : mainFontName;
            goto RELEASE_DC_AND_FONT;
        }

        if (!bSurrogatePair && enbaleRemap) {
            DWORD codepages = 0;
            if (icodepages->GetCharCodePages(single_codepoint[0], &codepages) >= 0) {
                if (ifont->MapFont(hdc, codepages, mainFont, &reMappedFont) >= 0) {
                    HGDIOBJ hOldFont = SelectObject(hdc, mainFont);
                    ::SelectObject(hdc, reMappedFont);
                    ::GetObjectW(reMappedFont, sizeof(LOGFONTW), remappedLogFont.get());
                    ::SelectObject(hdc, hOldFont);
                }
            }
        } else {
            remappedLogFont = nullptr;
        }
        if (reMappedFont) {
            std::shared_ptr<LOGFONTW> remappedLogFont2 = std::make_shared<LOGFONTW>();
            auto remappedFontGlpyhIndicies = GetFontGlpyhIndicies(hdc, reMappedFont, char_code, remappedLogFont2.get());
            if (remappedFontGlpyhIndicies.size() > 0) {
                glyphIndices = remappedFontGlpyhIndicies;
                result = wcslen(remappedLogFont2->lfFaceName) ? remappedLogFont2->lfFaceName : remappedLogFont->lfFaceName;
                goto RELEASE_DC_AND_FONT;
            }
        }

        if (enableFallback && !wcsstr(single_codepoint, L" ")) {
            HFONT testFont = reMappedFont ? reMappedFont : mainFont;
            HFONT fallbackFont = NULL;
            HDC metafileHDC = ::CreateEnhMetaFileW(NULL, NULL, NULL, NULL);
            HGDIOBJ metafileOldfont = ::SelectObject(metafileHDC, testFont);
            SCRIPT_STRING_ANALYSIS ssa;
            ::ScriptStringAnalyse(metafileHDC, single_codepoint, static_cast<int>(wcslen(single_codepoint)), 0, -1, SSA_METAFILE | SSA_FALLBACK | SSA_GLYPHS | SSA_LINK, 0, NULL, NULL, NULL, NULL, NULL, &ssa);
            ::ScriptStringOut(ssa, 0, 0, 0, NULL, 0, 0, FALSE);
            ::ScriptStringFree(&ssa);
            ::SelectObject(metafileHDC, metafileOldfont);
            HENHMETAFILE hmetafile = ::CloseEnhMetaFile(metafileHDC);
            std::shared_ptr<LOGFONTW> fallbackLogFont = std::make_shared<LOGFONTW>();
            ::EnumEnhMetaFile(0, hmetafile, MetaFileProc, fallbackLogFont.get(), NULL);
            if (wcslen(fallbackLogFont->lfFaceName)) {
                fallbackFont = CreateFontIndirectW(fallbackLogFont.get());
            }
            ::DeleteEnhMetaFile(hmetafile);
            if (fallbackFont) {
                std::shared_ptr<LOGFONTW> fallbackLogFont2 = std::make_shared<LOGFONTW>();
                auto remappedFontGlpyhIndicies = GetFontGlpyhIndicies(hdc, fallbackFont, char_code, fallbackLogFont2.get());
                ::DeleteObject(fallbackFont);
                if (remappedFontGlpyhIndicies.size() > 0) {
                    glyphIndices = remappedFontGlpyhIndicies;
                    result = wcslen(fallbackLogFont2->lfFaceName) ? fallbackLogFont2->lfFaceName : fallbackLogFont->lfFaceName;
                    goto RELEASE_DC_AND_FONT;
                }
            }
        }
    RELEASE_DC_AND_FONT:
        if (mainFont) {
            ::DeleteObject(mainFont);
        }
        if (hdc) {
            ::DeleteDC(hdc);
        }
        if (reMappedFont) {
            ifont->ReleaseFont(reMappedFont);
        }
    }
    if (bComInitted) ::CoUninitialize();
    return result;
}

std::shared_ptr<GlyphBitmapInfo> SystemFont(FontInfo* ttffont, const std::wstring& fontName, uint32_t glyphIndex) {
    BITMAPINFO dummyBitmapInfo = {{
        sizeof(BITMAPINFOHEADER),  // biSize
        32, -32,                   // biWidth, biHeight
        1,                         // biPlanes
        24,                        // biBitCount
        BI_RGB,                    // biCompression
        0,                         // biSizeImage
        72, 72,                    // biXPelsPerMeter, biYPelsPerMeter
        0, 0,                      // biClrUsed, biClrImportant
    }};
    unsigned char* dummyBitmapData = nullptr;
    HBITMAP dummyBitmap = ::CreateDIBSection(NULL, &dummyBitmapInfo, DIB_RGB_COLORS, (void**)&dummyBitmapData, NULL, 0);
    if (dummyBitmap == NULL) return nullptr;
    HDC dc = ::CreateCompatibleDC(NULL);
    if (dc == NULL) return nullptr;
    HGDIOBJ oldBitmap = ::SelectObject(dc, dummyBitmap);
    HGDIOBJ oldFont = NULL;
    HFONT font = ::CreateFontW(ttffont->size,                        // cHeight
                               0,                                    // cWidth
                               0,                                    // cEscapement
                               0,                                    // cOrientation
                               ttffont->bold ? FW_BOLD : FW_NORMAL,  // cWeight
                               ttffont->italic ? TRUE : FALSE,       // bItalic
                               FALSE,                                // bUnderline. Doesn't seem to work when I set TRUE.
                               FALSE,                                // bStrikeOut. Doesn't seem to work when I set TRUE.
                               DEFAULT_CHARSET,                      // iCharSet
                               OUT_DEFAULT_PRECIS,                   // iOutPrecision
                               CLIP_DEFAULT_PRECIS,                  // iClipPrecision
                               ANTIALIASED_QUALITY,                  // iQuality
                               DEFAULT_PITCH | FF_DONTCARE,          // iPitchAndFamily
                               fontName.data());
    if (font == NULL) {
        ::SelectObject(dc, oldBitmap);
        ::DeleteDC(dc);
        ::DeleteObject(dummyBitmap);
        return nullptr;
    }
    oldFont = ::SelectObject(dc, font);

    const MAT2 mat2 = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};
    std::shared_ptr<GlyphBitmapInfo> result = std::make_shared<GlyphBitmapInfo>();
    std::vector<uint8_t> glyphData;

    bool success = true;
    GLYPHMETRICS metrics = {};

    DWORD currGlyphDataSize = ::GetGlyphOutlineW(dc, (UINT)glyphIndex, GGO_GLYPH_INDEX | GGO_GRAY8_BITMAP, &metrics, 0, NULL, &mat2);
    if (currGlyphDataSize != GDI_ERROR) {
        if (currGlyphDataSize) {
            glyphData.resize(currGlyphDataSize);
            uint8_t* currGlyphData = glyphData.data();
            DWORD res = ::GetGlyphOutlineW(dc, (UINT)glyphIndex, GGO_GLYPH_INDEX | GGO_GRAY8_BITMAP, &metrics, currGlyphDataSize, currGlyphData, &mat2);
            if (res == 0 || res == GDI_ERROR) {
                success = false;
            } else {
                if (metrics.gmBlackBoxX && metrics.gmBlackBoxY) {
                    result->error_code = GlyphErrorCode::Success;
                    result->width = metrics.gmBlackBoxX;
                    result->height = metrics.gmBlackBoxY;
                    result->bearing_x = metrics.gmptGlyphOrigin.x;
                    result->bearing_y = metrics.gmptGlyphOrigin.y;
                    result->advance = metrics.gmCellIncX;
                    result->emoji = false;
                } else {
                    success = false;
                }
            }
        } else {
            result->error_code = GlyphErrorCode::NoBitmapData;
            result->width = 0;
            result->height = 0;
            result->bearing_x = 0;
            result->bearing_y = 0;
            result->advance = metrics.gmCellIncX;
            result->emoji = false;
        }
    }

    if (success) {
        if (glyphData.size() && result->width && result->height) {
            const uint32_t glyphDataRowPitch = AlignUp<uint32_t>(result->width, sizeof(DWORD));
            result->height = static_cast<uint32_t>(glyphData.size()) / glyphDataRowPitch;
            unsigned char* textureData = static_cast<unsigned char*>(malloc(result->width * result->height * 4 * sizeof(char)));
            BlitGray8Bitmap(textureData, result->width, glyphData.data(), glyphDataRowPitch, result->width, result->height);
            result->data = textureData;
            result->error_code = GlyphErrorCode::Success;
        } else {
            result->data = 0;
            result->error_code = GlyphErrorCode::NoBitmapData;
        }
    } else {
        result = nullptr;
    }

    SelectObject(dc, oldFont);
    DeleteObject(font);
    SelectObject(dc, oldBitmap);
    DeleteDC(dc);
    DeleteObject(dummyBitmap);
    return result;
}

std::vector<GlyphBitmapInfo> GetGlyphBitmapSystem(FontInfo* font, uint32_t char_code, bool enbaleRemap, bool enableFallback) {
    std::vector<wchar_t> glyphIndices;
    std::wstring fontName = GetSystemFontName(font, char_code, glyphIndices, enbaleRemap, enableFallback);
    std::vector<GlyphBitmapInfo> result;
    if (fontName != L"") {
        if (glyphIndices.size()) {
            for (auto glyphIndex : glyphIndices) {
                auto glyph = SystemFont(font, fontName, glyphIndex);
                if (glyph) {
                    result.push_back(*glyph);
                }
            }
        }
    }
    return result;
}
}  // namespace Font