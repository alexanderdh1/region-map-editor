#include "ui/UILayer.h"

#include "data/RegionStatus.h"
#include "input/Input.h"
#include "rendering/Camera.h"

#include <GLFW/glfw3.h>
#include <string>
#include <cmath>

// ============================================================
// Bitmap font (5x7) — replace entirely when switching to ImGui
// ============================================================

static const unsigned char font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // ' '
    {0x00,0x00,0x5f,0x00,0x00}, // '!'
    {0x00,0x07,0x00,0x07,0x00}, // '"'
    {0x14,0x7f,0x14,0x7f,0x14}, // '#'
    {0x24,0x2a,0x7f,0x2a,0x12}, // '$'
    {0x23,0x13,0x08,0x64,0x62}, // '%'
    {0x36,0x49,0x55,0x22,0x50}, // '&'
    {0x00,0x05,0x03,0x00,0x00}, // '\''
    {0x00,0x1c,0x22,0x41,0x00}, // '('
    {0x00,0x41,0x22,0x1c,0x00}, // ')'
    {0x14,0x08,0x3e,0x08,0x14}, // '*'
    {0x08,0x08,0x3e,0x08,0x08}, // '+'
    {0x00,0x50,0x30,0x00,0x00}, // ','
    {0x08,0x08,0x08,0x08,0x08}, // '-'
    {0x00,0x60,0x60,0x00,0x00}, // '.'
    {0x20,0x10,0x08,0x04,0x02}, // '/'
    {0x3e,0x51,0x49,0x45,0x3e}, // '0'
    {0x00,0x42,0x7f,0x40,0x00}, // '1'
    {0x42,0x61,0x51,0x49,0x46}, // '2'
    {0x21,0x41,0x45,0x4b,0x31}, // '3'
    {0x18,0x14,0x12,0x7f,0x10}, // '4'
    {0x27,0x45,0x45,0x45,0x39}, // '5'
    {0x3c,0x4a,0x49,0x49,0x30}, // '6'
    {0x01,0x71,0x09,0x05,0x03}, // '7'
    {0x36,0x49,0x49,0x49,0x36}, // '8'
    {0x06,0x49,0x49,0x29,0x1e}, // '9'
    {0x00,0x36,0x36,0x00,0x00}, // ':'
    {0x00,0x56,0x36,0x00,0x00}, // ';'
    {0x08,0x14,0x22,0x41,0x00}, // '<'
    {0x14,0x14,0x14,0x14,0x14}, // '='
    {0x00,0x41,0x22,0x14,0x08}, // '>'
    {0x02,0x01,0x51,0x09,0x06}, // '?'
    {0x32,0x49,0x79,0x41,0x3e}, // '@'
    {0x7e,0x11,0x11,0x11,0x7e}, // 'A'
    {0x7f,0x49,0x49,0x49,0x36}, // 'B'
    {0x3e,0x41,0x41,0x41,0x22}, // 'C'
    {0x7f,0x41,0x41,0x22,0x1c}, // 'D'
    {0x7f,0x49,0x49,0x49,0x41}, // 'E'
    {0x7f,0x09,0x09,0x09,0x01}, // 'F'
    {0x3e,0x41,0x49,0x49,0x7a}, // 'G'
    {0x7f,0x08,0x08,0x08,0x7f}, // 'H'
    {0x00,0x41,0x7f,0x41,0x00}, // 'I'
    {0x20,0x40,0x41,0x3f,0x01}, // 'J'
    {0x7f,0x08,0x14,0x22,0x41}, // 'K'
    {0x7f,0x40,0x40,0x40,0x40}, // 'L'
    {0x7f,0x02,0x0c,0x02,0x7f}, // 'M'
    {0x7f,0x04,0x08,0x10,0x7f}, // 'N'
    {0x3e,0x41,0x41,0x41,0x3e}, // 'O'
    {0x7f,0x09,0x09,0x09,0x06}, // 'P'
    {0x3e,0x41,0x51,0x21,0x5e}, // 'Q'
    {0x7f,0x09,0x19,0x29,0x46}, // 'R'
    {0x46,0x49,0x49,0x49,0x31}, // 'S'
    {0x01,0x01,0x7f,0x01,0x01}, // 'T'
    {0x3f,0x40,0x40,0x40,0x3f}, // 'U'
    {0x1f,0x20,0x40,0x20,0x1f}, // 'V'
    {0x3f,0x40,0x38,0x40,0x3f}, // 'W'
    {0x63,0x14,0x08,0x14,0x63}, // 'X'
    {0x07,0x08,0x70,0x08,0x07}, // 'Y'
    {0x61,0x51,0x49,0x45,0x43}, // 'Z'
    {0x00,0x7f,0x41,0x41,0x00}, // '['
    {0x02,0x04,0x08,0x10,0x20}, // '\\'
    {0x00,0x41,0x41,0x7f,0x00}, // ']'
    {0x04,0x02,0x01,0x02,0x04}, // '^'
    {0x40,0x40,0x40,0x40,0x40}, // '_'
    {0x00,0x01,0x02,0x04,0x00}, // '`'
    {0x20,0x54,0x54,0x54,0x78}, // 'a'
    {0x7f,0x48,0x44,0x44,0x38}, // 'b'
    {0x38,0x44,0x44,0x44,0x20}, // 'c'
    {0x38,0x44,0x44,0x48,0x7f}, // 'd'
    {0x38,0x54,0x54,0x54,0x18}, // 'e'
    {0x08,0x7e,0x09,0x01,0x02}, // 'f'
    {0x0c,0x52,0x52,0x52,0x3e}, // 'g'
    {0x7f,0x08,0x04,0x04,0x78}, // 'h'
    {0x00,0x44,0x7d,0x40,0x00}, // 'i'
    {0x20,0x40,0x44,0x3d,0x00}, // 'j'
    {0x7f,0x10,0x28,0x44,0x00}, // 'k'
    {0x00,0x41,0x7f,0x40,0x00}, // 'l'
    {0x7c,0x04,0x18,0x04,0x78}, // 'm'
    {0x7c,0x08,0x04,0x04,0x78}, // 'n'
    {0x38,0x44,0x44,0x44,0x38}, // 'o'
    {0x7c,0x14,0x14,0x14,0x08}, // 'p'
    {0x08,0x14,0x14,0x18,0x7c}, // 'q'
    {0x7c,0x08,0x04,0x04,0x08}, // 'r'
    {0x48,0x54,0x54,0x54,0x20}, // 's'
    {0x04,0x3f,0x44,0x40,0x20}, // 't'
    {0x3c,0x40,0x40,0x20,0x7c}, // 'u'
    {0x1c,0x20,0x40,0x20,0x1c}, // 'v'
    {0x3c,0x40,0x30,0x40,0x3c}, // 'w'
    {0x44,0x28,0x10,0x28,0x44}, // 'x'
    {0x0c,0x50,0x50,0x50,0x3c}, // 'y'
    {0x44,0x64,0x54,0x4c,0x44}, // 'z'
    {0x00,0x08,0x36,0x41,0x00}, // '{'
    {0x00,0x00,0x7f,0x00,0x00}, // '|'
    {0x00,0x41,0x36,0x08,0x00}, // '}'
    {0x10,0x08,0x08,0x10,0x08}, // '~'
};

// ============================================================
// Low-level draw helpers
// NOTE: These are the ONLY things that need to change for ImGui
// ============================================================

static void uiDrawPanel(double x, double y, double w, double h,
                        float r, float g, float b, float a)
{
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2d(x,     y);
    glVertex2d(x + w, y);
    glVertex2d(x + w, y + h);
    glVertex2d(x,     y + h);
    glEnd();
}

static void uiDrawChar(double x, double y, char c, float scale)
{
    int idx = static_cast<int>(c) - 32;
    if (idx < 0 || idx >= 95) return;
    const unsigned char* col = font5x7[idx];
    for (int cx = 0; cx < 5; cx++)
        for (int row = 0; row < 7; row++)
            if (col[cx] & (1 << row))
            {
                double px = x + cx * scale;
                double py = y + row * scale;
                glBegin(GL_QUADS);
                glVertex2d(px,         py);
                glVertex2d(px + scale, py);
                glVertex2d(px + scale, py + scale);
                glVertex2d(px,         py + scale);
                glEnd();
            }
}

static void uiDrawString(double x, double y, const std::string& text,
                         float r, float g, float b, float scale = 1.5f)
{
    glDisable(GL_TEXTURE_2D);
    glColor3f(r, g, b);
    double cx = x;
    for (char c : text)
    {
        uiDrawChar(cx, y, c, scale);
        cx += (5 + 1) * scale;
    }
}

static int uiMaxChars(double panelW, double padding, float scale)
{
    return static_cast<int>((panelW - padding * 2) / (6.0 * scale));
}

static std::string uiTrunc(const std::string& s, int max)
{
    if (static_cast<int>(s.size()) <= max) return s;
    return s.substr(0, max - 3) + "...";
}

// ============================================================
// Colour presets (shared between popup and keyboard handler)
// ============================================================

static const float colourPresets[][4] = {
    { 0.40f, 0.60f, 1.00f, 0.35f }, // blue   (default)
    { 0.30f, 0.85f, 0.45f, 0.35f }, // green
    { 0.95f, 0.35f, 0.35f, 0.35f }, // red
    { 0.95f, 0.75f, 0.20f, 0.35f }, // yellow
    { 0.75f, 0.35f, 0.95f, 0.35f }, // purple
    { 0.95f, 0.55f, 0.20f, 0.35f }, // orange
};
static constexpr int NUM_COLOURS = 6;

// ============================================================
// UILayer
// ============================================================

void UILayer::render(Core& core)
{
    renderToolIndicator(core);
    renderPopup(core);
}

bool UILayer::onKeyPress(int key, Core& core)
{
    auto& input     = core.getInput();
    auto& selection = core.getSelection();
    Region* r       = selection.selectedRegion;

    // --- Tool switching ---
    if (key == GLFW_KEY_R)
    {
        input.setDrawTool(DrawTool::Rectangle);
        input.cancelPolygon();
        return true;
    }
    if (key == GLFW_KEY_P)
    {
        input.setDrawTool(DrawTool::Polygon);
        return true;
    }

    // --- Cancel drawing or close popup ---
    if (key == GLFW_KEY_ESCAPE)
    {
        if (input.isDrawingPolygon())
            input.cancelPolygon();
        else if (input.isDrawingRect())
            input.cancelRect();
        else
            selection.clear();
        return true;
    }

    // --- Region editing (only when a region is selected) ---
    if (!r) return false;

    switch (key)
    {
        case GLFW_KEY_1: r->status = RegionStatus::None;       return true;
        case GLFW_KEY_2: r->status = RegionStatus::InProgress; return true;
        case GLFW_KEY_3: r->status = RegionStatus::Done;       return true;

        case GLFW_KEY_C:
        {
            int next = 0;
            for (int i = 0; i < NUM_COLOURS; i++)
            {
                if (colourPresets[i][0] == r->colorR &&
                    colourPresets[i][1] == r->colorG)
                {
                    next = (i + 1) % NUM_COLOURS;
                    break;
                }
            }
            r->colorR = colourPresets[next][0];
            r->colorG = colourPresets[next][1];
            r->colorB = colourPresets[next][2];
            r->colorA = colourPresets[next][3];
            return true;
        }

        case GLFW_KEY_DELETE:
        {
            RegionId id = r->id;
            selection.clear();
            core.getRegionTree().removeRegion(id);
            return true;
        }

        default: return false;
    }
}

// ============================================================
// Private — popup panel
// ============================================================

void UILayer::renderPopup(Core& core)
{
    const SelectionState& selection = core.getSelection();
    if (!selection.popupOpen || !selection.selectedRegion) return;

    const Region& region = *selection.selectedRegion;

    const double PX  = 20.0;
    const double PY  = 20.0;
    const double PW  = 270.0;
    const double PH  = 210.0;
    const double PAD = 12.0;
    const float  S   = 1.5f;
    const int    MC  = uiMaxChars(PW, PAD, S);

    uiDrawPanel(PX, PY, PW, PH, 0.1f, 0.1f, 0.12f, 0.92f);
    uiDrawPanel(PX, PY, PW, 6.0,
                region.colorR, region.colorG, region.colorB, 1.0f);

    uiDrawString(PX + PAD, PY + 15,
                 uiTrunc(region.name, uiMaxChars(PW, PAD, 1.8f)),
                 1.f, 1.f, 1.f, 1.8f);

    uiDrawString(PX + PAD, PY + 44,
                 uiTrunc("Status: " + regionStatusToString(region.status), MC),
                 0.8f, 0.8f, 0.8f, S);

    std::string note = region.note.empty() ? "(no note)" : region.note;
    uiDrawString(PX + PAD, PY + 64,
                 uiTrunc("Note: " + note, MC),
                 0.7f, 0.7f, 0.7f, S);

    uiDrawString(PX + PAD, PY + 88, "Colour:", 0.7f, 0.7f, 0.7f, S);
    uiDrawPanel(PX + PAD + 54, PY + 83, 24.0, 13.0,
                region.colorR, region.colorG, region.colorB, 1.0f);

    uiDrawPanel(PX + 8, PY + 109, PW - 16, 1.0, 0.3f, 0.3f, 0.35f, 1.0f);

    uiDrawString(PX + PAD, PY + 120,
                 uiTrunc("[1] None [2] In Progress [3] Done", MC),
                 0.5f, 0.5f, 0.6f, S);
    uiDrawString(PX + PAD, PY + 143,
                 uiTrunc("[C] Cycle colour  [Esc] Close", MC),
                 0.5f, 0.5f, 0.6f, S);
    uiDrawString(PX + PAD, PY + 166,
                 uiTrunc("[Del] Delete region", MC),
                 0.5f, 0.5f, 0.6f, S);
}

// ============================================================
// Private — tool indicator
// ============================================================

void UILayer::renderToolIndicator(const Core& core)
{
    const Input&  input  = core.getInput();
    const Camera& camera = core.getCamera();

    bool isRect = input.getDrawTool() == DrawTool::Rectangle;
    bool isPoly = input.getDrawTool() == DrawTool::Polygon;

    const double W = 200.0;
    const double H = 32.0;
    const double X = 20.0;
    const double Y = camera.viewportSize.y - H - 20.0;

    uiDrawPanel(X, Y, W, H, 0.1f, 0.1f, 0.12f, 0.88f);

    float ar = isRect ? 0.4f : 0.3f;
    float ag = isRect ? 0.6f : 0.85f;
    float ab = isRect ? 1.0f : 0.45f;
    uiDrawPanel(X, Y, W, 3.0, ar, ag, ab, 1.0f);

    float rr = isRect ? 0.4f : 0.25f;
    float rg = isRect ? 0.6f : 0.25f;
    float rb = isRect ? 1.0f : 0.25f;
    uiDrawPanel(X + 8, Y + 8, 16.0, 16.0, rr, rg, rb, isRect ? 0.95f : 0.4f);
    uiDrawString(X + 28, Y + 11, "[R] Rect",
                 isRect ? 1.f : 0.45f, isRect ? 1.f : 0.45f, isRect ? 1.f : 0.45f);

    float pr = isPoly ? 0.3f  : 0.25f;
    float pg = isPoly ? 0.85f : 0.25f;
    float pb = isPoly ? 0.45f : 0.25f;
    uiDrawPanel(X + 108, Y + 8, 16.0, 16.0, pr, pg, pb, isPoly ? 0.95f : 0.4f);
    uiDrawString(X + 128, Y + 11, "[P] Poly",
                 isPoly ? 1.f : 0.45f, isPoly ? 1.f : 0.45f, isPoly ? 1.f : 0.45f);
}
