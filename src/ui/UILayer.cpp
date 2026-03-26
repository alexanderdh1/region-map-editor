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
// UILayer
// ============================================================

void UILayer::render(Core& core)
{
    renderRegionTree(core);
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

    // --- Back (navigate to parent region) ---
    if (key == GLFW_KEY_B)
    {
        if (selection.canGoBack())
            selection.popView();
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

    float viewW     = static_cast<float>(camera.viewportSize.x);
    float viewH     = static_cast<float>(camera.viewportSize.y);
    float sidebarW  = 44.0f;

    float maxPopupH = viewH - 16.0f;
    ImGui::SetNextWindowPos(ImVec2(viewW - sidebarW - 280.0f, 8.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints(ImVec2(270.0f, 100.0f), ImVec2(270.0f, maxPopupH));
    ImGui::SetNextWindowSize(ImVec2(270.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.92f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize        |
        ImGuiWindowFlags_NoMove          |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoCollapse      |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_AlwaysVerticalScrollbar;

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

    // Name field — single line, capped width
    ImGui::Text("Name");
    static char nameBuf[256];
    if (!nameFieldActive_)
    {
        strncpy(nameBuf, region.name.c_str(), sizeof(nameBuf) - 1);
        nameBuf[sizeof(nameBuf) - 1] = '\0';
    }
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

    // Note field — auto word wrap via callback
    ImGui::Text("Note");
    static char noteBuf[2048];
    if (!noteFieldActive_)
    {
        strncpy(noteBuf, region.note.c_str(), sizeof(noteBuf) - 1);
        noteBuf[sizeof(noteBuf) - 1] = '\0';
    }

    // Calculate height based on line count
    int lineCount = 1;
    for (const char* c = noteBuf; *c; c++)
        if (*c == '\n') lineCount++;
    float lineH   = ImGui::GetTextLineHeightWithSpacing();
    float minH    = lineH * 3.0f;
    float wantedH = lineH * (lineCount + 1);
    float maxH    = std::max(minH, ImGui::GetContentRegionAvail().y - 80.0f);
    float noteH   = std::min(wantedH, maxH);
    noteH         = std::max(noteH, minH);

    // Callback to auto-insert newline when line gets too long
    struct WrapState { int maxCharsPerLine; };
    static WrapState wrapState{ 30 }; // ~30 chars at 16px JetBrains Mono in 270px panel

    auto wrapCallback = [](ImGuiInputTextCallbackData* data) -> int
    {
        if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit)
        {
            WrapState* ws = static_cast<WrapState*>(data->UserData);
            // Find last newline before cursor
            int lineStart = 0;
            for (int i = 0; i < data->CursorPos; i++)
                if (data->Buf[i] == '\n') lineStart = i + 1;

            int lineLen = data->CursorPos - lineStart;
            if (lineLen >= ws->maxCharsPerLine)
            {
                // Find last space to break at word boundary
                int breakAt = -1;
                for (int i = data->CursorPos - 1; i >= lineStart; i--)
                {
                    if (data->Buf[i] == ' ')
                    {
                        breakAt = i;
                        break;
                    }
                }
                if (breakAt >= 0)
                    data->Buf[breakAt] = '\n';
                else
                    data->InsertChars(data->CursorPos, "\n");
                data->BufDirty = true;
            }
        }
        return 0;
    };

    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputTextMultiline("##note", noteBuf, sizeof(noteBuf),
        ImVec2(-1, noteH),
        ImGuiInputTextFlags_CallbackEdit,
        wrapCallback, &wrapState))
    {
        region.note = noteBuf;
        noteFieldActive_ = true;
    }
    if (!ImGui::IsItemActive()) noteFieldActive_ = false;

    ImGui::Spacing();

    // Colour button + picker
    ImGui::Text("Colour");
    float col[4] = { region.colorR, region.colorG, region.colorB, region.colorA };

    ImGui::PushStyleColor(ImGuiCol_Button,
        ImVec4(region.colorR, region.colorG, region.colorB, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
        ImVec4(region.colorR, region.colorG, region.colorB, 1.0f));
    if (ImGui::Button("Colour", ImVec2(-1, 0)))
        ImGui::OpenPopup("ColourPicker");
    ImGui::PopStyleColor(2);

    if (ImGui::BeginPopup("ColourPicker"))
    {
        if (ImGui::ColorPicker4("##picker", col,
            ImGuiColorEditFlags_NoSidePreview  |
            ImGuiColorEditFlags_NoSmallPreview |
            ImGuiColorEditFlags_NoInputs       |
            ImGuiColorEditFlags_AlphaBar))
        {
            region.colorR = col[0];
            region.colorG = col[1];
            region.colorB = col[2];
            region.colorA = col[3];
        }
        ImGui::EndPopup();
    }

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
// Private — left region tree sidebar (ImGui)
// ============================================================

// Recursive helper to draw one region row + its children indented
static void drawRegionNode(
    const Region& region,
    SelectionState& selection,
    int depth)
{
    ImDrawList* draw = ImGui::GetWindowDrawList();

    // Indentation per depth level
    if (depth > 0)
        ImGui::Indent(14.0f * depth);

    // Colour circle icon
    ImVec2 pos = ImGui::GetCursorScreenPos();
    float circleR = 5.0f;
    float circleX = pos.x + circleR + 2.0f;
    float circleY = pos.y + ImGui::GetTextLineHeight() * 0.5f;
    draw->AddCircleFilled(
        ImVec2(circleX, circleY), circleR,
        IM_COL32(
            static_cast<int>(region.colorR * 255),
            static_cast<int>(region.colorG * 255),
            static_cast<int>(region.colorB * 255),
            220
        )
    );

    // Offset text to make room for circle
    ImGui::SetCursorScreenPos(ImVec2(pos.x + circleR * 2 + 6.0f, pos.y));

    bool isSelected = (selection.selectedRegion == &region);

    // Highlight selected
    if (isSelected)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));

    // Selectable row
    std::string label = region.name + "##" + std::to_string(region.id);
    if (ImGui::Selectable(label.c_str(), isSelected, 0, ImVec2(0, 0)))
    {
        // Build viewStack from parent chain
        selection.viewStack.clear();
        const Region* ancestor = region.parent;
        while (ancestor)
        {
            selection.viewStack.insert(selection.viewStack.begin(),
                const_cast<Region*>(ancestor));
            ancestor = ancestor->parent;
        }
        selection.selectedRegion = const_cast<Region*>(&region);
        selection.popupOpen      = true;
    }

    if (isSelected)
        ImGui::PopStyleColor();

    if (depth > 0)
        ImGui::Unindent(14.0f * depth);

    // Draw children recursively
    for (const auto& child : region.children)
        drawRegionNode(*child, selection, depth + 1);
}

void UILayer::renderRegionTree(Core& core)
{
    const Camera& camera      = core.getCamera();
    SelectionState& selection = core.getSelection();
    const RegionTree& tree    = core.getRegionTree();

    const float viewH      = static_cast<float>(camera.viewportSize.y);
    const float collapsedW = 24.0f;
    const float expandedW  = static_cast<float>(camera.viewportSize.x) * 0.30f;
    const float panelW     = treeExpanded_ ? expandedW : collapsedW;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelW, viewH), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.92f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar      |
        ImGuiWindowFlags_NoResize        |
        ImGuiWindowFlags_NoMove          |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(4, 4));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.13f, 0.92f));

    ImGui::Begin("##regiontree", nullptr, flags);

    // Toggle button — arrow pointing right/left
    const char* arrow = treeExpanded_ ? "<" : ">";
    if (ImGui::Button(arrow, ImVec2(16, 16)))
        treeExpanded_ = !treeExpanded_;

    if (treeExpanded_)
    {
        ImGui::Separator();
        ImGui::Text("Regions");
        ImGui::Separator();

        if (tree.roots().empty())
        {
            ImGui::TextDisabled("No regions yet.");
        }
        else
        {
            for (const auto& root : tree.roots())
                drawRegionNode(*root, selection, 0);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
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
