#include "FontRenderer.h"
#include <iostream>
#include <cstdint>
#include <algorithm>

GLuint FontRenderer::textureId = 0;
int FontRenderer::atlasWidth = 1024;
int FontRenderer::atlasHeight = 1024;
int FontRenderer::currentX = 4;
int FontRenderer::currentY = 4;
int FontRenderer::rowHeight = 0;
int FontRenderer::baseSize = 36;
int FontRenderer::fontAscent = 0;

HDC FontRenderer::memDC = NULL;
HBITMAP FontRenderer::hBitmap = NULL;
HFONT FontRenderer::hFont = NULL;
uint32_t* FontRenderer::pDIBits = nullptr;
std::unordered_map<wchar_t, GlyphInfo> FontRenderer::glyphMap;

std::wstring FontRenderer::utf8ToWide(const std::string& utf8Str) {
    if (utf8Str.empty()) return L"";
    int count = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (count <= 1) return L"";
    std::wstring wstr(count - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, &wstr[0], count);
    return wstr;
}

bool FontRenderer::init(HDC hdc, const wchar_t* fontName, int baseFontSize) {
    baseSize = baseFontSize;
    currentX = 4;
    currentY = 4;
    rowHeight = 0;
    glyphMap.clear();

    memDC = CreateCompatibleDC(hdc);
    if (!memDC) return false;

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = atlasWidth;
    bmi.bmiHeader.biHeight = -atlasHeight; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    hBitmap = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, (void**)&pDIBits, NULL, 0);
    if (!hBitmap || !pDIBits) return false;

    SelectObject(memDC, hBitmap);

    // Xóa toàn bộ atlas về màu đen nền trong suốt
    std::fill(pDIBits, pDIBits + (atlasWidth * atlasHeight), 0x00000000);

    // Tạo font hệ thống Segoe UI sắc nét (chuẩn Windows) với ANTIALIASED_QUALITY
    hFont = CreateFontW(
        baseSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, VARIABLE_PITCH, fontName
    );

    if (!hFont) {
        // Fallback sang Arial nếu không có Segoe UI
        hFont = CreateFontW(
            baseSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, VARIABLE_PITCH, L"Arial"
        );
    }
    SelectObject(memDC, hFont);

    TEXTMETRICW tm;
    GetTextMetricsW(memDC, &tm);
    fontAscent = tm.tmAscent;

    SetBkColor(memDC, RGB(0, 0, 0));
    SetTextColor(memDC, RGB(255, 255, 255));
    SetBkMode(memDC, TRANSPARENT);

    // Tạo OpenGL Texture
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    // Nạp sẵn toàn bộ ký tự ASCII và Tiếng Việt phổ biến
    preloadCommonGlyphs();

    // Chuyển đổi dữ liệu DIB sang RGBA trong suốt và tải lên GPU
    std::vector<uint32_t> rgbaBuffer(atlasWidth * atlasHeight);
    for (int i = 0; i < atlasWidth * atlasHeight; ++i) {
        uint8_t alpha = (uint8_t)(pDIBits[i] & 0xFF);
        rgbaBuffer[i] = ((uint32_t)alpha << 24) | 0x00FFFFFF;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaBuffer.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void FontRenderer::cleanup() {
    if (textureId != 0) {
        glDeleteTextures(1, &textureId);
        textureId = 0;
    }
    if (hFont) {
        DeleteObject(hFont);
        hFont = NULL;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
        hBitmap = NULL;
    }
    if (memDC) {
        DeleteDC(memDC);
        memDC = NULL;
    }
    pDIBits = nullptr;
    glyphMap.clear();
}

bool FontRenderer::cacheGlyph(wchar_t ch) {
    if (glyphMap.find(ch) != glyphMap.end()) return true;

    if (ch == L' ') {
        SIZE sz;
        GetTextExtentPoint32W(memDC, L" ", 1, &sz);
        GlyphInfo g = {};
        g.width = 0;
        g.height = sz.cy > 0 ? sz.cy : baseSize;
        g.advance = sz.cx > 0 ? sz.cx : (baseSize / 3);
        g.u0 = g.v0 = g.u1 = g.v1 = 0.0f;
        glyphMap[ch] = g;
        return true;
    }

    SIZE sz;
    if (!GetTextExtentPoint32W(memDC, &ch, 1, &sz)) {
        sz.cx = baseSize / 2;
        sz.cy = baseSize;
    }
    if (sz.cx <= 0) sz.cx = baseSize / 3;

    int pad = 3;
    int glyphW = sz.cx + pad * 2;
    int glyphH = sz.cy + pad * 2;

    if (currentX + glyphW >= atlasWidth) {
        currentX = 4;
        currentY += rowHeight + 4;
        rowHeight = 0;
    }

    if (currentY + glyphH >= atlasHeight) {
        // Đã hết không gian trên Atlas
        return false;
    }

    // Vẽ ký tự vào DIB
    RECT rc = { currentX, currentY, currentX + glyphW, currentY + glyphH };
    FillRect(memDC, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

    ExtTextOutW(memDC, currentX + pad, currentY + pad, 0, NULL, &ch, 1, NULL);

    GlyphInfo g;
    g.u0 = (float)(currentX + pad) / (float)atlasWidth;
    g.v0 = (float)(currentY + pad) / (float)atlasHeight;
    g.u1 = (float)(currentX + pad + sz.cx) / (float)atlasWidth;
    g.v1 = (float)(currentY + pad + sz.cy) / (float)atlasHeight;
    g.width = sz.cx;
    g.height = sz.cy;
    g.advance = sz.cx;

    currentX += glyphW + 2;
    if (glyphH > rowHeight) rowHeight = glyphH;

    glyphMap[ch] = g;
    return true;
}

void FontRenderer::preloadCommonGlyphs() {
    // 1. Toàn bộ bảng ASCII (32 -> 126)
    for (wchar_t c = 32; c <= 126; ++c) {
        cacheGlyph(c);
    }

    // 2. Toàn bộ ký tự Tiếng Việt (chữ thường và chữ hoa)
    const wchar_t* vietnameseChars =
        L"ÀÁẢÃẠĂẰẮẲẴẶÂẦẤẨẪẬàáảãạăằắẳẵặâầấẩẫậ"
        L"Đđ"
        L"ÈÉẺẼẸÊỀẾỂỄỆèéẻẽẹêềếểễệ"
        L"ÌÍỈĨỊìíỉĩị"
        L"ÒÓỎÕỌÔỒỐỔỖỘƠỜỚỞỠỢòóỏõọôồốổỗộơờớởỡợ"
        L"ÙÚỦŨỤƯỪỨỬỮỰùúủũụưừứửữự"
        L"ỲÝỶỸỴỳýỷỹỵ"
        L"°×•★▶◀▲▼■…→←↑↓✓✗";

    for (const wchar_t* p = vietnameseChars; *p != 0; ++p) {
        cacheGlyph(*p);
    }
}

Vec2 FontRenderer::measureText(const std::string& utf8Text, float size) {
    if (utf8Text.empty()) return Vec2(0.0f, 0.0f);
    std::wstring wstr = utf8ToWide(utf8Text);
    float scale = size / (float)baseSize;

    float maxWidth = 0.0f;
    float curWidth = 0.0f;
    float totalHeight = (float)baseSize * scale;

    for (wchar_t ch : wstr) {
        if (ch == L'\n') {
            if (curWidth > maxWidth) maxWidth = curWidth;
            curWidth = 0.0f;
            totalHeight += (float)baseSize * scale * 1.25f;
            continue;
        }

        auto it = glyphMap.find(ch);
        if (it == glyphMap.end()) {
            cacheGlyph(ch);
            it = glyphMap.find(ch);
        }

        if (it != glyphMap.end()) {
            curWidth += (float)it->second.advance * scale;
        } else {
            curWidth += (float)baseSize * 0.5f * scale;
        }
    }

    if (curWidth > maxWidth) maxWidth = curWidth;
    return Vec2(maxWidth, totalHeight);
}

void FontRenderer::drawText(float x, float y, const std::string& utf8Text, const Color4f& color, float size, bool withShadow, bool center) {
    if (utf8Text.empty() || textureId == 0) return;

    std::wstring wstr = utf8ToWide(utf8Text);
    if (wstr.empty()) return;

    float scale = size / (float)baseSize;
    float startX = x;

    if (center) {
        Vec2 sz = measureText(utf8Text, size);
        startX = x - sz.x * 0.5f;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Vẽ 2 lượt: Lượt 1 bóng đổ màu đen nhẹ, Lượt 2 chữ chính
    int passes = withShadow ? 2 : 1;

    for (int pass = 0; pass < passes; ++pass) {
        float curX = startX;
        float curY = y;

        if (withShadow && pass == 0) {
            float shadowOffset = std::max(1.0f, scale * 1.5f);
            curX += shadowOffset;
            curY += shadowOffset;
            glColor4f(0.0f, 0.0f, 0.0f, color.a * 0.85f);
        } else {
            glColor4f(color.r, color.g, color.b, color.a);
        }

        glBegin(GL_QUADS);
        for (wchar_t ch : wstr) {
            if (ch == L'\n') {
                curY += (float)baseSize * scale * 1.25f;
                curX = (pass == 0 && withShadow) ? (startX + std::max(1.0f, scale * 1.5f)) : startX;
                continue;
            }

            auto it = glyphMap.find(ch);
            if (it == glyphMap.end()) {
                cacheGlyph(ch);
                it = glyphMap.find(ch);
            }

            if (it != glyphMap.end()) {
                const GlyphInfo& g = it->second;
                if (g.width > 0 && g.height > 0) {
                    float x0 = curX;
                    float y0 = curY;
                    float x1 = curX + (float)g.width * scale;
                    float y1 = curY + (float)g.height * scale;

                    glTexCoord2f(g.u0, g.v0); glVertex2f(x0, y0);
                    glTexCoord2f(g.u1, g.v0); glVertex2f(x1, y0);
                    glTexCoord2f(g.u1, g.v1); glVertex2f(x1, y1);
                    glTexCoord2f(g.u0, g.v1); glVertex2f(x0, y1);
                }
                curX += (float)g.advance * scale;
            } else {
                curX += (float)baseSize * 0.5f * scale;
            }
        }
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
}
