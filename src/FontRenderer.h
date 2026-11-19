#pragma once
#include <windows.h>
#include <GL/gl.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include "Types.h"

// Cấu trúc lưu thông tin tọa độ texture và kích thước của từng ký tự trong Font Atlas
struct GlyphInfo {
    float u0, v0, u1, v1; // Tọa độ UV trong OpenGL Texture
    int width;            // Chiều rộng pixel
    int height;           // Chiều cao pixel
    int advance;          // Khoảng cách bước nhảy sang ký tự kế tiếp
};

class FontRenderer {
public:
    // Khởi tạo Font Atlas và tải font hệ thống sắc nét (Segoe UI / Arial)
    static bool init(HDC hdc, const wchar_t* fontName = L"Segoe UI", int baseFontSize = 36);
    static void cleanup();

    // Vẽ chuỗi văn bản UTF-8 tiếng Việt hoàn chỉnh với màu sắc, kích thước và bóng đổ tùy chọn
    static void drawText(float x, float y, const std::string& utf8Text, const Color4f& color, float size = 18.0f, bool withShadow = true, bool center = false);

    // Đo kích thước (width, height) của chuỗi UTF-8 theo pixel ở một cỡ font nhất định
    static Vec2 measureText(const std::string& utf8Text, float size = 18.0f);

    // Chuyển đổi chuỗi UTF-8 sang chuỗi rộng UTF-16
    static std::wstring utf8ToWide(const std::string& utf8Str);

private:
    static GLuint textureId;
    static int atlasWidth;
    static int atlasHeight;
    static int currentX;
    static int currentY;
    static int rowHeight;
    static int baseSize;
    static int fontAscent;

    static HDC memDC;
    static HBITMAP hBitmap;
    static HFONT hFont;
    static uint32_t* pDIBits;
    static std::unordered_map<wchar_t, GlyphInfo> glyphMap;

    static void preloadCommonGlyphs();
    static bool cacheGlyph(wchar_t ch);
};
