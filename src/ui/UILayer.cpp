#include "ui/UILayer.h"

#include "data/RegionSerializer.h"
#include "data/RegionGeometry.h"
#include "input/Input.h"
#include "rendering/Camera.h"

#include "imgui.h"

#include <GLFW/glfw3.h>
#include <string>
#include <cmath>

// ============================================================
// Edit mode helpers
// ============================================================

static void enterEditMode(Core& core)
{
    Region* target = core.getSelection().selectedRegion;
    if (!target) return;
    core.getInput().cancelPolygon();
    core.getInput().cancelRect();
    core.getInput().setDrawTool(DrawTool::Edit);
    core.getEditState().target = target;
    core.getEditState().clearDrag();
}

static void exitEditMode(Core& core, bool save)
{
    if (save && core.getEditState().isActive())
        RegionSerializer::save(core.getRegionTree(), "regions.json", core);
    core.getEditState().clear();
    core.getInput().cancelEdit();
    core.getInput().setDrawTool(DrawTool::Navigate);
}

static void setHiddenRecursive(Region& region, bool hidden, RegionTree& tree)
{
    region.hidden = hidden;
    for (auto& child : region.children)
        setHiddenRecursive(*child, hidden, tree);
}

static bool allPointsInsideParent(const Region& region, const RegionGeometry& parentGeom)
{
    for (const Vec2& pt : region.geometry.getPoints())
        if (!parentGeom.contains(pt)) return false;
    return true;
}

// ============================================================
// UILayer::render
// ============================================================

void UILayer::render(Core& core)
{
    if (dirtyTime_ >= 0.0)
    {
        bool fieldStillActive = nameFieldActive_ || noteFieldActive_;
        double now = glfwGetTime();
        if (!fieldStillActive && (now - dirtyTime_) >= SAVE_DEBOUNCE_S)
        {
            RegionSerializer::save(core.getRegionTree(), "regions.json", core);
            dirtyTime_ = -1.0;
            snapshotPushedForDirty_ = false;
        }
    }

    renderRegionTree(core);
    renderSidebar(core);
    renderPopup(core);
}

bool UILayer::onKeyPress(int key, int mods, Core& core)
{
    auto& input     = core.getInput();
    auto& selection = core.getSelection();
    auto& editState = core.getEditState();
    Region* r       = selection.selectedRegion;

    if (nameFieldActive_ || noteFieldActive_) return true;

    bool ctrl  = (mods & GLFW_MOD_CONTROL) != 0;
    bool shift = (mods & GLFW_MOD_SHIFT)   != 0;

    if (ctrl && key == GLFW_KEY_Z && !shift) { core.undo(); return true; }
    if (ctrl && (key == GLFW_KEY_Y || (key == GLFW_KEY_Z && shift))) { core.redo(); return true; }

    if (key == GLFW_KEY_S)
    {
        RegionSerializer::save(core.getRegionTree(), "regions.json", core);
        dirtyTime_ = -1.0;
        return true;
    }
    if (key == GLFW_KEY_E)
    {
        if (input.getDrawTool() == DrawTool::Edit) exitEditMode(core, true);
        else if (r && !r->hidden)                  enterEditMode(core);
        return true;
    }
    if (key == GLFW_KEY_C)
    {
        if (r && !r->hidden && selection.popupOpen)
            colourPickerOpen_ = !colourPickerOpen_;
        return true;
    }
    if (input.getDrawTool() != DrawTool::Edit)
    {
        if (key == GLFW_KEY_R) { input.setDrawTool(DrawTool::Rectangle); input.cancelPolygon(); return true; }
        if (key == GLFW_KEY_P) { input.setDrawTool(DrawTool::Polygon); return true; }
    }
    if (key == GLFW_KEY_B) { if (selection.canGoBack()) selection.popView(); return true; }
    if (key == GLFW_KEY_ESCAPE)
    {
        if (input.getDrawTool() == DrawTool::Edit)      exitEditMode(core, true);
        else if (input.isDrawingPolygon())               input.cancelPolygon();
        else if (input.isDrawingRect())                  input.cancelRect();
        else if (selection.popupOpen)                    selection.clear();
        if (input.getDrawTool() != DrawTool::Edit)       input.setDrawTool(DrawTool::Navigate);
        core.clearPendingParent(); // cancel a pending "Add sub-region"
        return true;
    }
    if (key == GLFW_KEY_DELETE)
    {
        if (input.getDrawTool() == DrawTool::Edit)
        {
            if (editState.handleType == EditHandleType::PolyPoint)
                core.deleteEditPoint(); // pushSnapshot() is inside deleteEditPoint()
        }
        else if (r && !r->hidden)
        {
            core.pushSnapshot();
            RegionId id = r->id;
            selection.clear();
            core.getRegionTree().removeRegion(id);
            RegionSerializer::save(core.getRegionTree(), "regions.json", core);
        }
        return true;
    }
    return false;
}

// ============================================================
// Private — popup
// ============================================================

void UILayer::renderPopup(Core& core)
{
    SelectionState& selection = core.getSelection();
    if (!selection.popupOpen || !selection.selectedRegion)
    {
        colourPickerOpen_ = false;
        return;
    }

    Region& region    = *selection.selectedRegion;
    const Camera& cam = core.getCamera();
    bool isEditMode   = core.getInput().getDrawTool() == DrawTool::Edit;
    bool isEditTarget = isEditMode && core.getEditState().target == &region;
    bool isHidden     = region.hidden;

    float viewW    = static_cast<float>(cam.viewportSize.x);
    float viewH    = static_cast<float>(cam.viewportSize.y);
    float sidebarW = 44.0f;

    ImGui::SetNextWindowPos(ImVec2(viewW - sidebarW - 280.0f, 8.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints(ImVec2(270.0f, 100.0f), ImVec2(270.0f, viewH - 16.0f));
    ImGui::SetNextWindowSize(ImVec2(270.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.92f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_AlwaysVerticalScrollbar |
        ImGuiWindowFlags_NoFocusOnAppearing;

    float titleA = isHidden ? 0.35f : 0.85f;
    ImGui::PushStyleColor(ImGuiCol_TitleBg,
        ImVec4(region.colorR, region.colorG, region.colorB, titleA));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive,
        ImVec4(region.colorR, region.colorG, region.colorB, titleA));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.13f, 0.92f));

    std::string windowTitle = isHidden
        ? "Region (hidden)###RegionPopup"
        : "Region###RegionPopup";

    bool open = true;
    ImGui::Begin(windowTitle.c_str(), &open, flags);

    if (!open)
    {
        if (isEditMode) exitEditMode(core, true);
        selection.clear();
        ImGui::End();
        ImGui::PopStyleColor(3);
        return;
    }

    if (selection.canGoBack())
    {
        if (ImGui::Button(("< " + selection.viewStack.back()->name).c_str()))
            selection.popView();
        ImGui::Separator();
    }

    ImGui::Text("Name");
    static char nameBuf[256];
    if (!nameFieldActive_) { strncpy(nameBuf, region.name.c_str(), 255); nameBuf[255] = 0; }
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf)))
    {
        if (!snapshotPushedForDirty_) { core.pushSnapshot(); snapshotPushedForDirty_ = true; }
        region.name = nameBuf;
        dirtyTime_ = glfwGetTime();
    }
    if (ImGui::IsItemActive()) nameFieldActive_ = true;
    else nameFieldActive_ = false;

    ImGui::Spacing();
    ImGui::Text("Note");
    static char noteBuf[2048];
    if (!noteFieldActive_) { strncpy(noteBuf, region.note.c_str(), 2047); noteBuf[2047] = 0; }

    int lineCount = 1;
    for (const char* c = noteBuf; *c; c++) if (*c == '\n') lineCount++;
    float lineH = ImGui::GetTextLineHeightWithSpacing();
    float minH  = lineH * 3.0f;
    float noteH = std::min(std::max(lineH * (lineCount + 1), minH),
                           std::max(minH, ImGui::GetContentRegionAvail().y - 80.0f));

#ifdef ImGuiInputTextFlags_WordWrap
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputTextMultiline("##note", noteBuf, sizeof(noteBuf),
        ImVec2(-1, noteH), ImGuiInputTextFlags_WordWrap))
    {
        if (!snapshotPushedForDirty_) { core.pushSnapshot(); snapshotPushedForDirty_ = true; }
        region.note = noteBuf;
        dirtyTime_ = glfwGetTime();
    }
#else
    struct WrapState { int maxCharsPerLine; };
    static WrapState ws{ 33 };
    auto wrapCb = [](ImGuiInputTextCallbackData* d) -> int {
        if (d->EventFlag != ImGuiInputTextFlags_CallbackEdit) return 0;
        WrapState* w = static_cast<WrapState*>(d->UserData);
        int ls = 0;
        for (int i = 0; i < d->CursorPos; i++) if (d->Buf[i] == '\n') ls = i + 1;
        if (d->CursorPos - ls < w->maxCharsPerLine) return 0;
        int brk = -1;
        for (int i = d->CursorPos - 1; i >= ls; i--) if (d->Buf[i] == ' ') { brk = i; break; }
        if (brk >= 0) d->Buf[brk] = '\n'; else d->InsertChars(d->CursorPos, "\n");
        d->BufDirty = true; return 0;
    };
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputTextMultiline("##note", noteBuf, sizeof(noteBuf),
        ImVec2(-1, noteH), ImGuiInputTextFlags_CallbackEdit, wrapCb, &ws))
    {
        if (!snapshotPushedForDirty_) { core.pushSnapshot(); snapshotPushedForDirty_ = true; }
        region.note = noteBuf;
        dirtyTime_ = glfwGetTime();
    }
#endif
    if (ImGui::IsItemActive()) noteFieldActive_ = true;
    else noteFieldActive_ = false;

    int numChildren = static_cast<int>(region.children.size());
    if (numChildren > 0)
    {
        ImGui::Spacing(); ImGui::Separator(); ImGui::Text("Sub-regions");
        for (int i = 0; i < numChildren; i++)
        {
            Region* child = region.children[i].get();
            ImGui::PushID(i);
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImVec4(child->colorR, child->colorG, child->colorB, 0.35f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                ImVec4(child->colorR, child->colorG, child->colorB, 0.55f));
            if (ImGui::Button(child->name.c_str(), ImVec2(-1, 0)))
                selection.pushView(child);
            ImGui::PopStyleColor(2);
            ImGui::PopID();
        }
    }

    ImGui::Spacing(); ImGui::Separator(); ImGui::Text("Tools"); ImGui::Spacing();

    if (isHidden)
    {
        ImGui::BeginDisabled(true);
        ImGui::Button("Colour",         ImVec2(-1, 0));
        ImGui::Button("Edit region",    ImVec2(-1, 0));
        ImGui::Button("Add sub-region", ImVec2(-1, 0));
        ImGui::EndDisabled();
    }
    else
    {
        float col[4] = { region.colorR, region.colorG, region.colorB, region.colorA };
        ImGui::PushStyleColor(ImGuiCol_Button,
            ImVec4(region.colorR, region.colorG, region.colorB, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
            ImVec4(region.colorR, region.colorG, region.colorB, 1.0f));
        if (ImGui::Button("Colour", ImVec2(-1, 0))) colourPickerOpen_ = !colourPickerOpen_;
        ImGui::PopStyleColor(2);

        if (colourPickerOpen_)
        {
            float pickerW = 270.0f;
            float pickerX = viewW - sidebarW - 280.0f - pickerW;
            float pickerY = 8.0f;
            float pickerH = 0.0f; // auto-fit to content
            ImGui::SetNextWindowPos(ImVec2(pickerX, pickerY), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(pickerW, pickerH), ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.96f);
            ImGuiWindowFlags pickerFlags =
                ImGuiWindowFlags_NoResize    | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
            ImGui::Begin("##colourpicker", nullptr, pickerFlags);
            float availW = ImGui::GetContentRegionAvail().x;
            ImGui::SetNextItemWidth(availW);
            if (ImGui::ColorPicker4("##picker", col,
                ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview |
                ImGuiColorEditFlags_NoInputs      | ImGuiColorEditFlags_AlphaBar,
                nullptr))
            { region.colorR=col[0]; region.colorG=col[1]; region.colorB=col[2]; region.colorA=col[3]; }
            if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
                ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                colourPickerOpen_ = false;
            ImGui::End();
            ImGui::PopStyleVar();
        }
        if (isEditTarget)
        {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.25f,0.45f,0.85f,1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f,0.55f,1.00f,1.0f));
            if (ImGui::Button("Stop editing", ImVec2(-1,0))) exitEditMode(core, true);
            ImGui::PopStyleColor(2);
        }
        else
        {
            if (ImGui::Button("Edit region", ImVec2(-1,0))) enterEditMode(core);
        }
        if (ImGui::Button("Add sub-region", ImVec2(-1,0)))
        { core.setPendingParent(region.id); selection.clear(); }
    }

    ImGui::Separator();

    if (isHidden)
    {
        ImGui::BeginDisabled(true);
        ImGui::Button("Delete region", ImVec2(-1, 0));
        ImGui::EndDisabled();
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f,0.15f,0.15f,0.8f));
        if (ImGui::Button("Delete region", ImVec2(-1,0)))
        {
            if (isEditMode) exitEditMode(core, false);
            core.pushSnapshot();
            RegionId id = region.id;
            selection.clear();
            core.getRegionTree().removeRegion(id);
            RegionSerializer::save(core.getRegionTree(), "regions.json", core);
            ImGui::PopStyleColor();
            ImGui::End();
            ImGui::PopStyleColor(3);
            return;
        }
        ImGui::PopStyleColor();
    }

    ImGui::End();
    ImGui::PopStyleColor(3);
}
// Tree drag-and-drop
//
// KEY DESIGN: drop actions are DEFERRED.
// During tree rendering we only collect what the user wants to do
// (pendingSourceId_ / pendingTargetId_). The actual moveRegion call
// happens AFTER the entire tree is drawn, so we never modify the
// children vector while iterating over it.
// ============================================================

static void drawRegionNode(
    const Region& region,
    SelectionState& selection,
    RegionTree& tree,
    Core& core,
    RegionId& pendingSourceId,   // out: set when a valid drop is detected
    RegionId& pendingTargetId,   // out: 0 = root, otherwise target region id
    bool& pendingDropReady,      // out: true = execute the move this frame
    int depth)
{
    ImDrawList* draw    = ImGui::GetWindowDrawList();
    const float indentW = 14.0f;
    const float rowH    = ImGui::GetTextLineHeight() + 6.0f;
    const float arrowW  = 16.0f;
    const float circleR = 5.0f;
    const float circleW = circleR * 2 + 8.0f;

    if (depth > 0) ImGui::Indent(indentW * depth);

    // Collapse arrow
    bool hasChildren = !region.children.empty();
    if (hasChildren)
    {
        const char* arrow = region.collapsed ? ">" : "v";
        ImGui::PushID((std::string("col##") + std::to_string(region.id)).c_str());
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,1,1,0.08f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1,1,1,0.15f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
        if (ImGui::Button(arrow, ImVec2(arrowW, rowH)))
        {
            const_cast<Region&>(region).collapsed = !region.collapsed;
            RegionSerializer::save(tree, "regions.json", core);
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }
    else { ImGui::Dummy(ImVec2(arrowW, rowH)); }
    ImGui::SameLine(0, 2);

    // Visibility circle
    ImVec2 circlePos = ImGui::GetCursorScreenPos();
    float cx = circlePos.x + circleW * 0.5f;
    float cy = circlePos.y + rowH   * 0.5f;
    ImU32 circleCol = region.hidden
        ? IM_COL32(90,90,90,200)
        : IM_COL32((int)(region.colorR*255),(int)(region.colorG*255),(int)(region.colorB*255),220);

    ImGui::PushID((std::string("vis##") + std::to_string(region.id)).c_str());
    ImGui::InvisibleButton("##vis", ImVec2(circleW, rowH));
    bool circleHov = ImGui::IsItemHovered();
    if (ImGui::IsItemClicked())
    {
        setHiddenRecursive(const_cast<Region&>(region), !region.hidden, tree);
        RegionSerializer::save(tree, "regions.json", core);
    }
    if (circleHov)
        ImGui::SetTooltip(region.hidden ? "Click to show on map" : "Click to hide on map");
    float r = circleHov ? circleR + 1.0f : circleR;
    draw->AddCircleFilled(ImVec2(cx, cy), r, circleCol);
    ImGui::PopID();

    ImGui::SameLine(0, 4);

    // Selectable name
    bool isSelected = (selection.selectedRegion == &region);
    ImVec4 textCol = region.hidden
        ? ImVec4(0.45f,0.45f,0.45f,1.0f)
        : (isSelected ? ImVec4(0.4f,0.7f,1.0f,1.0f) : ImGui::GetStyleColorVec4(ImGuiCol_Text));

    ImGui::PushStyleColor(ImGuiCol_Text, textCol);
    ImGui::PushID(static_cast<int>(region.id));

    std::string label = region.name + "##" + std::to_string(region.id);
    if (ImGui::Selectable(label.c_str(), isSelected,
        ImGuiSelectableFlags_None, ImVec2(0, rowH - ImGui::GetStyle().ItemSpacing.y)))
    {
        selection.viewStack.clear();
        const Region* anc = region.parent;
        while (anc)
        {
            selection.viewStack.insert(selection.viewStack.begin(), const_cast<Region*>(anc));
            anc = anc->parent;
        }
        selection.selectedRegion = const_cast<Region*>(&region);
        selection.popupOpen      = true;
    }

    // Drag source
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        RegionId id = region.id;
        ImGui::SetDragDropPayload("REGION_ID", &id, sizeof(RegionId));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f,0.7f,1.0f,1.0f));
        ImGui::Text("  %s", region.name.c_str());
        ImGui::PopStyleColor();
        ImGui::EndDragDropSource();
    }

    // Drop target — only collect, never execute here
    if (ImGui::BeginDragDropTarget())
    {
        ImVec2 rowMin = ImGui::GetItemRectMin();
        ImVec2 rowMax = ImGui::GetItemRectMax();
        draw->AddRectFilled(rowMin, rowMax, IM_COL32(60,120,220,60));
        draw->AddRect(rowMin, rowMax, IM_COL32(80,160,255,200), 0.0f, 0, 1.5f);

        if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload("REGION_ID",
                ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
        {
            if (payload->IsDelivery())
            {
                pendingSourceId  = *(const RegionId*)payload->Data;
                pendingTargetId  = region.id;
                pendingDropReady = true;
            }
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::PopID();
    ImGui::PopStyleColor();

    if (depth > 0) ImGui::Unindent(indentW * depth);

    // Recurse — children vector is NOT modified here, only pendingDrop is set
    if (!region.collapsed)
    {
        // Copy children ids to avoid iterator invalidation if anything changes
        std::vector<Region*> childPtrs;
        childPtrs.reserve(region.children.size());
        for (const auto& c : region.children) childPtrs.push_back(c.get());

        for (Region* child : childPtrs)
            drawRegionNode(*child, selection, tree, core,
                           pendingSourceId, pendingTargetId, pendingDropReady, depth + 1);
    }
}

static int countRegions(const RegionTree& tree)
{
    int n = 0;
    tree.forEach([&](const Region&) { n++; });
    return n;
}

void UILayer::renderRegionTree(Core& core)
{
    const Camera& cam   = core.getCamera();
    SelectionState& sel = core.getSelection();
    RegionTree& tree    = core.getRegionTree();

    const float viewH      = static_cast<float>(cam.viewportSize.y);
    const float collapsedW = 30.0f;
    const float expandedW  = static_cast<float>(cam.viewportSize.x) * 0.20f;
    const float panelW     = treeExpanded_ ? expandedW : collapsedW;

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelW, viewH), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.92f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize  |
        ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(4, 4));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f,0.10f,0.13f,0.92f));
    ImGui::Begin("##regiontree", nullptr, flags);

    // Toggle button
    {
        const float btnH   = 22.0f;
        ImVec2      btnPos = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##toggle", ImVec2(panelW, btnH));
        if (ImGui::IsItemClicked()) treeExpanded_ = !treeExpanded_;
        if (ImGui::IsItemHovered())
            ImGui::GetWindowDrawList()->AddRectFilled(btnPos,
                ImVec2(btnPos.x + panelW, btnPos.y + btnH), IM_COL32(255,255,255,18));
        const char* arrow = treeExpanded_ ? "<" : ">";
        ImVec2 gsz = ImGui::CalcTextSize(arrow);
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(btnPos.x + (panelW - gsz.x) * 0.5f,
                   btnPos.y + (btnH   - gsz.y) * 0.5f),
            IM_COL32(200,200,200,255), arrow);
    }

    ImGui::SetWindowFontScale(1.10f);

    if (treeExpanded_)
    {
        ImGui::Separator();
        ImGui::Text("Regions (%d)", countRegions(tree));
        ImGui::Separator();

        // Pending drop — collected during tree render, executed after
        RegionId pendingSource  = 0;
        RegionId pendingTarget  = 0;
        bool     dropReady      = false;

        if (tree.roots().empty())
        {
            ImGui::TextDisabled("No regions yet.");
        }
        else
        {
            // Snapshot root pointers before rendering to avoid issues
            // if the vector is somehow modified (belt-and-suspenders)
            std::vector<Region*> rootPtrs;
            rootPtrs.reserve(tree.roots().size());
            for (const auto& r : tree.roots()) rootPtrs.push_back(r.get());

            for (Region* root : rootPtrs)
                drawRegionNode(*root, sel, tree, core,
                               pendingSource, pendingTarget, dropReady, 0);
        }

        // Root-level drop zone — fills all remaining space in the panel.
        // Any area below the last region row acts as "drop to root".
        float remainingH = ImGui::GetContentRegionAvail().y;
        if (remainingH < 20.0f) remainingH = 20.0f;

        ImGui::Dummy(ImVec2(panelW - 8.0f, remainingH));
        if (ImGui::BeginDragDropTarget())
        {
            // Show a horizontal line at the top of the drop zone
            ImVec2 dropMin = ImGui::GetItemRectMin();
            ImGui::GetWindowDrawList()->AddLine(
                dropMin, ImVec2(dropMin.x + panelW, dropMin.y),
                IM_COL32(80,160,255,200), 2.0f);

            if (const ImGuiPayload* payload =
                ImGui::AcceptDragDropPayload("REGION_ID",
                    ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
            {
                if (payload->IsDelivery())
                {
                    pendingSource = *(const RegionId*)payload->Data;
                    pendingTarget = 0; // root
                    dropReady     = true;
                }
            }
            ImGui::EndDragDropTarget();
        }

        // ---- Execute deferred drop AFTER tree is fully rendered ----
        if (dropReady && pendingSource != 0)
        {
            bool valid = (pendingSource != pendingTarget)
                && !tree.isAncestorOf(pendingSource, pendingTarget);

            if (valid)
            {
                Region* src = tree.findById(pendingSource);

                bool geometryOk = true;
                if (pendingTarget != 0)
                {
                    Region* tgt = tree.findById(pendingTarget);
                    geometryOk = tgt && allPointsInsideParent(*src, tgt->geometry);
                }

                if (src && geometryOk)
                {
                    core.pushSnapshot();
                    tree.moveRegion(pendingSource, pendingTarget);
                    RegionSerializer::save(tree, "regions.json", core);
                    sel.clear(); // clear stale parent pointers in viewStack
                }
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

// ============================================================
// Private — right sidebar
// ============================================================

void UILayer::renderSidebar(Core& core)
{
    Input& input      = core.getInput();
    const Camera& cam = core.getCamera();

    const float sidebarW = 44.0f;
    const float viewH    = static_cast<float>(cam.viewportSize.y);
    const float viewW    = static_cast<float>(cam.viewportSize.x);

    ImGui::SetNextWindowPos(ImVec2(viewW - sidebarW, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(sidebarW, viewH), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.92f);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoResize    |
        ImGuiWindowFlags_NoMove      | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 8));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(4, 8));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f,0.10f,0.13f,0.92f));
    ImGui::Begin("##sidebar", nullptr, flags);

    ImDrawList* draw  = ImGui::GetWindowDrawList();
    bool rectTool     = input.getDrawTool() == DrawTool::Rectangle;
    bool polyTool     = input.getDrawTool() == DrawTool::Polygon;
    bool drawingRect  = input.isDrawingRect();
    bool drawingPoly  = input.isDrawingPolygon();

    auto btnCol = [](bool active, bool drawing) -> ImVec4 {
        if (drawing) return ImVec4(0.25f,0.45f,0.85f,1.0f);
        if (active)  return ImVec4(0.20f,0.35f,0.65f,1.0f);
        return ImVec4(0.14f,0.14f,0.18f,1.0f);
    };
    auto iconCol = [](bool active, bool drawing) -> ImU32 {
        return (active||drawing) ? IM_COL32(120,180,255,255) : IM_COL32(180,180,200,255);
    };

    ImGui::PushStyleColor(ImGuiCol_Button, btnCol(rectTool, drawingRect));
    if (ImGui::Button("##rect", ImVec2(36,36)))
    { input.setDrawTool(DrawTool::Rectangle); input.cancelPolygon(); }
    ImGui::PopStyleColor();
    {
        ImVec2 mn=ImGui::GetItemRectMin(), mx=ImGui::GetItemRectMax();
        ImVec2 c=ImVec2((mn.x+mx.x)*0.5f,(mn.y+mx.y)*0.5f); float sz=10.0f;
        draw->AddRect(ImVec2(c.x-sz,c.y-sz*0.65f),ImVec2(c.x+sz,c.y+sz*0.65f),
            iconCol(rectTool,drawingRect),2.0f,0,1.8f);
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Rectangle [R]");

    ImGui::PushStyleColor(ImGuiCol_Button, btnCol(polyTool, drawingPoly));
    if (ImGui::Button("##poly", ImVec2(36,36))) input.setDrawTool(DrawTool::Polygon);
    ImGui::PopStyleColor();
    {
        ImVec2 mn=ImGui::GetItemRectMin(), mx=ImGui::GetItemRectMax();
        ImVec2 c=ImVec2((mn.x+mx.x)*0.5f,(mn.y+mx.y)*0.5f);
        float r=11.0f; ImU32 col=iconCol(polyTool,drawingPoly);
        const float PI=3.14159265f;
        for (int i=0;i<6;i++)
        {
            float a0=PI/2.0f+i*2.0f*PI/6.0f, a1=PI/2.0f+(i+1)*2.0f*PI/6.0f;
            draw->AddLine(ImVec2(c.x+r*cosf(a0),c.y-r*sinf(a0)),
                          ImVec2(c.x+r*cosf(a1),c.y-r*sinf(a1)),col,1.8f);
        }
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Polygon [P]");

    // Undo / Redo buttons
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    bool canUndo = core.canUndo();
    bool canRedo = core.canRedo();

    ImGui::BeginDisabled(!canUndo);
    ImGui::PushStyleColor(ImGuiCol_Button, canUndo
        ? ImVec4(0.14f, 0.14f, 0.18f, 1.0f)
        : ImVec4(0.10f, 0.10f, 0.12f, 1.0f));
    if (ImGui::Button("##undo", ImVec2(36, 36))) core.undo();
    ImGui::PopStyleColor();
    ImGui::EndDisabled();
    {
        ImVec2 mn = ImGui::GetItemRectMin(), mx = ImGui::GetItemRectMax();
        ImVec2 c  = ImVec2((mn.x + mx.x) * 0.5f, (mn.y + mx.y) * 0.5f);
        ImU32  col = canUndo ? IM_COL32(180, 180, 200, 255) : IM_COL32(70, 70, 80, 255);
        // left-pointing arrow: head + stem
        float hw = 8.0f, hh = 6.0f, sw = 4.0f, sh = 3.0f;
        draw->AddTriangleFilled(
            ImVec2(c.x - hw,       c.y),
            ImVec2(c.x - hw + hh,  c.y - hh),
            ImVec2(c.x - hw + hh,  c.y + hh), col);
        draw->AddRectFilled(
            ImVec2(c.x - hw + hh - 1.0f, c.y - sh),
            ImVec2(c.x + sw,              c.y + sh), col);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("Undo [Ctrl+Z]");

    ImGui::BeginDisabled(!canRedo);
    ImGui::PushStyleColor(ImGuiCol_Button, canRedo
        ? ImVec4(0.14f, 0.14f, 0.18f, 1.0f)
        : ImVec4(0.10f, 0.10f, 0.12f, 1.0f));
    if (ImGui::Button("##redo", ImVec2(36, 36))) core.redo();
    ImGui::PopStyleColor();
    ImGui::EndDisabled();
    {
        ImVec2 mn = ImGui::GetItemRectMin(), mx = ImGui::GetItemRectMax();
        ImVec2 c  = ImVec2((mn.x + mx.x) * 0.5f, (mn.y + mx.y) * 0.5f);
        ImU32  col = canRedo ? IM_COL32(180, 180, 200, 255) : IM_COL32(70, 70, 80, 255);
        // right-pointing arrow: head + stem
        float hw = 8.0f, hh = 6.0f, sw = 4.0f, sh = 3.0f;
        draw->AddTriangleFilled(
            ImVec2(c.x + hw,       c.y),
            ImVec2(c.x + hw - hh,  c.y - hh),
            ImVec2(c.x + hw - hh,  c.y + hh), col);
        draw->AddRectFilled(
            ImVec2(c.x - sw,              c.y - sh),
            ImVec2(c.x + hw - hh + 1.0f, c.y + sh), col);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("Redo [Ctrl+Y]");

    ImGui::End();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}
