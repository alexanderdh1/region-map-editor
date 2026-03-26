#include "ui/UILayer.h"

#include "data/RegionStatus.h"
#include "data/RegionSerializer.h"
#include "input/Input.h"
#include "rendering/Camera.h"

#include "imgui.h"

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
    subRegionZones_.clear();

    renderSidebar(core);
    renderContextMenu(core);
    renderPopup(core);
}

void UILayer::onCharInput(unsigned int /*codepoint*/, Core& /*core*/)
{
    // ImGui handles all text input directly via its own char callback
}

bool UILayer::onMouseClick(const Vec2& /*screenPos*/, Core& /*core*/)
{
    // ImGui handles all UI clicks via WantCaptureMouse in WindowCallbacks.
    // This function is kept for any future non-ImGui click handling.
    return false;
}

bool UILayer::onKeyPress(int key, Core& core)
{
    auto& input     = core.getInput();
    auto& selection = core.getSelection();
    Region* r       = selection.selectedRegion;

    // ImGui text fields handle their own input — swallow keys when active
    if (nameFieldActive_ || noteFieldActive_)
        return true;

    // --- Save ---
    if (key == GLFW_KEY_S)
    {
        RegionSerializer::save(core.getRegionTree(), "regions.json");
        return true;
    }

    // --- Tool shortcuts ---
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

    // --- Escape ---
    if (key == GLFW_KEY_ESCAPE)
    {
        if (input.isDrawingPolygon())
            input.cancelPolygon();
        else if (input.isDrawingRect())
            input.cancelRect();
        else if (selection.contextMenuOpen)
            selection.closeContextMenu();
        else
            selection.clear();
        input.setDrawTool(DrawTool::Navigate);
        return true;
    }

    // --- Colour cycle (kept as keyboard shortcut) ---
    if (key == GLFW_KEY_C && r)
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

    // --- Delete region shortcut ---
    if (key == GLFW_KEY_DELETE && r)
    {
        RegionId id = r->id;
        selection.clear();
        core.getRegionTree().removeRegion(id);
        return true;
    }

    return false;
}

// ============================================================
// Private — popup panel (ImGui)
// ============================================================

void UILayer::renderPopup(Core& core)
{
    SelectionState& selection = core.getSelection();
    if (!selection.popupOpen || !selection.selectedRegion) return;

    Region& region = *selection.selectedRegion;
    const Camera& camera = core.getCamera();

    float viewW = static_cast<float>(camera.viewportSize.x);
    float sidebarW = 44.0f;

    ImGui::SetNextWindowPos(ImVec2(viewW - sidebarW - 280.0f, 8.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(270.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.92f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize        |
        ImGuiWindowFlags_NoMove          |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoCollapse      |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    // Coloured top border via header colour
    ImGui::PushStyleColor(ImGuiCol_TitleBg,       ImVec4(region.colorR, region.colorG, region.colorB, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive,  ImVec4(region.colorR, region.colorG, region.colorB, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg,       ImVec4(0.10f, 0.10f, 0.13f, 0.92f));

    bool open = true;
    ImGui::Begin("Region", &open, flags);

    if (!open)
    {
        selection.clear();
        ImGui::End();
        ImGui::PopStyleColor(3);
        return;
    }

    // Back button
    if (selection.canGoBack())
    {
        if (ImGui::Button(("< " + selection.viewStack.back()->name).c_str()))
            selection.popView();
        ImGui::Separator();
    }

    // Name field
    ImGui::Text("Name");
    static char nameBuf[256];
    if (!nameFieldActive_)
        strncpy(nameBuf, region.name.c_str(), sizeof(nameBuf) - 1);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf)))
    {
        region.name = nameBuf;
        nameFieldActive_ = true;
    }
    if (!ImGui::IsItemActive()) nameFieldActive_ = false;

    ImGui::Spacing();

    // Status buttons
    ImGui::Text("Status");
    auto statusBtn = [&](const char* label, RegionStatus s) {
        bool active = region.status == s;
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.45f, 0.85f, 1.0f));
        if (ImGui::Button(label)) region.status = s;
        if (active) ImGui::PopStyleColor();
        ImGui::SameLine();
    };
    statusBtn("None",        RegionStatus::None);
    statusBtn("In Progress", RegionStatus::InProgress);
    statusBtn("Done",        RegionStatus::Done);
    ImGui::NewLine();

    ImGui::Spacing();

    // Note field
    ImGui::Text("Note");
    static char noteBuf[1024];
    if (!noteFieldActive_)
        strncpy(noteBuf, region.note.c_str(), sizeof(noteBuf) - 1);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputTextMultiline("##note", noteBuf, sizeof(noteBuf), ImVec2(-1, 60)))
    {
        region.note = noteBuf;
        noteFieldActive_ = true;
    }
    if (!ImGui::IsItemActive()) noteFieldActive_ = false;

    ImGui::Spacing();

    // Colour hint
    ImGui::TextDisabled("[C] Cycle colour");

    // Sub-regions
    int numChildren = static_cast<int>(region.children.size());
    if (numChildren > 0)
    {
        ImGui::Separator();
        ImGui::Text("Sub-regions");
        for (int i = 0; i < numChildren; i++)
        {
            Region* child = region.children[i].get();
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4(child->colorR, child->colorG, child->colorB, 0.35f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                ImVec4(child->colorR, child->colorG, child->colorB, 0.55f));
            if (ImGui::Button(child->name.c_str(), ImVec2(-1, 0)))
                selection.pushView(child);
            ImGui::PopStyleColor(2);
        }
    }

    ImGui::Separator();

    // Delete button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.15f, 0.15f, 0.8f));
    if (ImGui::Button("Delete region", ImVec2(-1, 0)))
    {
        RegionId id = region.id;
        selection.clear();
        core.getRegionTree().removeRegion(id);
        ImGui::PopStyleColor();
        ImGui::End();
        ImGui::PopStyleColor(3);
        return;
    }
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleColor(3);
}

// ============================================================
// Private — context menu (right-click, ImGui)
// ============================================================

void UILayer::renderContextMenu(Core& core)
{
    SelectionState& selection = core.getSelection();
    if (!selection.contextMenuOpen || !selection.contextRegion) return;

    ImVec2 pos = ImVec2(
        static_cast<float>(selection.contextMenuScreen.x),
        static_cast<float>(selection.contextMenuScreen.y)
    );
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);

    ImGui::OpenPopup("##contextmenu");
    if (ImGui::BeginPopup("##contextmenu"))
    {
        ImGui::TextDisabled(selection.contextRegion->name.c_str());
        ImGui::Separator();

        if (ImGui::MenuItem("Add sub-region"))
        {
            core.setPendingParent(selection.contextRegion->id);
            selection.closeContextMenu();
            selection.clear();
        }

        if (ImGui::MenuItem("Cancel"))
            selection.closeContextMenu();

        ImGui::EndPopup();
    }
    else
    {
        // Popup closed by clicking outside
        selection.closeContextMenu();
    }
}

// ============================================================
// Private — sidebar (ImGui)
// ============================================================

void UILayer::renderSidebar(Core& core)
{
    Input& input = core.getInput();
    const Camera& camera = core.getCamera();

    const float sidebarW = 44.0f;
    const float viewH    = static_cast<float>(camera.viewportSize.y);
    const float viewW    = static_cast<float>(camera.viewportSize.x);

    ImGui::SetNextWindowPos(ImVec2(viewW - sidebarW, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(sidebarW, viewH), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.92f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar      |
        ImGuiWindowFlags_NoResize        |
        ImGuiWindowFlags_NoMove          |
        ImGuiWindowFlags_NoScrollbar     |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(4, 8));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.13f, 0.92f));

    ImGui::Begin("##sidebar", nullptr, flags);

    // ---- Region tool icon ----
    bool isDrawing = input.isDrawingRect() || input.isDrawingPolygon();

    // Only highlight button when actively drawing
    if (isDrawing)
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.45f, 0.85f, 1.0f));

    if (ImGui::Button("##region", ImVec2(36, 36)))
        ImGui::OpenPopup("RegionToolPopup");

    if (isDrawing)
        ImGui::PopStyleColor();

    // Draw a simple region icon inside the button area
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 btnMin = ImGui::GetItemRectMin();
    ImVec2 btnMax = ImGui::GetItemRectMax();
    ImVec2 center = ImVec2((btnMin.x + btnMax.x) * 0.5f,
                           (btnMin.y + btnMax.y) * 0.5f);

    float iconSize = 11.0f;
    draw->AddRect(
        ImVec2(center.x - iconSize, center.y - iconSize * 0.7f),
        ImVec2(center.x + iconSize, center.y + iconSize * 0.7f),
        isDrawing ? IM_COL32(120, 180, 255, 255)
                  : IM_COL32(180, 180, 200, 255),
        2.0f, 0, 1.5f
    );

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Region");

    // ---- Region tool popup ----
    ImGui::SetNextWindowPos(
        ImVec2(viewW - sidebarW - 130.0f, 8.0f),
        ImGuiCond_Always
    );

    if (ImGui::BeginPopup("RegionToolPopup"))
    {
        ImGui::TextDisabled("Draw region");
        ImGui::Separator();

        bool rectActive = input.getDrawTool() == DrawTool::Rectangle;
        bool polyActive = input.getDrawTool() == DrawTool::Polygon;

        // Uniform highlight colour for active tool
        const ImVec4 activeCol = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);

        if (rectActive) ImGui::PushStyleColor(ImGuiCol_Text, activeCol);
        if (ImGui::MenuItem("Rectangle  [R]", nullptr, rectActive))
        {
            input.setDrawTool(DrawTool::Rectangle);
            input.cancelPolygon();
        }
        if (rectActive) ImGui::PopStyleColor();

        if (polyActive) ImGui::PushStyleColor(ImGuiCol_Text, activeCol);
        if (ImGui::MenuItem("Polygon    [P]", nullptr, polyActive))
            input.setDrawTool(DrawTool::Polygon);
        if (polyActive) ImGui::PopStyleColor();

        ImGui::EndPopup();
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}
