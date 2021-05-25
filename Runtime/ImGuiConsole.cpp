#include "ImGuiConsole.hpp"

#include "CStateManager.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1/MP1.hpp"

#include "imgui.h"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce {

struct EntityInfo {
  TUniqueId uid;
  bool closed = false;

  explicit EntityInfo(TUniqueId uid) : uid(uid) {}
};

static std::vector<EntityInfo> inspectingEntities;

static void ShowMenuFile() {
  static bool paused;
  paused = g_Main->IsPaused();
  if (ImGui::MenuItem("Paused", nullptr, &paused)) {
    g_Main->SetPaused(paused);
  }
  if (ImGui::MenuItem("Quit", "Alt+F4")) {
    g_Main->Quit();
  }
}

static void ShowInspectWindow(bool* isOpen) {
  if (ImGui::Begin("Inspect", isOpen)) {
    if (ImGui::BeginTable("Entities", 3,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ScrollY)) {
      ImGui::TableSetupColumn("ID",
                              ImGuiTableColumnFlags_PreferSortAscending | ImGuiTableColumnFlags_DefaultSort |
                                  ImGuiTableColumnFlags_WidthFixed,
                              0, 'id');
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0, 'name');
      ImGui::TableSetupColumn("", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupScrollFreeze(0, 1);
      ImGui::TableHeadersRow();
      CObjectList& list = *g_StateManager->GetObjectList();
      std::vector<CEntity*> items;
      items.reserve(list.size());
      for (auto* ent : list) {
        items.push_back(ent);
      }
      if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
        for (int i = 0; i < sortSpecs->SpecsCount; ++i) {
          const auto& specs = sortSpecs->Specs[i];
          if (specs.ColumnUserID == 'id') {
            std::sort(items.begin(), items.end(), [&](CEntity* a, CEntity* b) {
              u16 aId = a->GetUniqueId().Value();
              u16 bId = b->GetUniqueId().Value();
              return specs.SortDirection == ImGuiSortDirection_Ascending ? aId < bId : aId > bId;
            });
          } else if (specs.ColumnUserID == 'name') {
            std::sort(items.begin(), items.end(), [&](CEntity* a, CEntity* b) {
              int compare = a->GetName().compare(b->GetName());
              return specs.SortDirection == ImGuiSortDirection_Ascending ? compare < 0 : compare > 0;
            });
          }
        }
      }
      for (const auto& item : items) {
        TUniqueId uid = item->GetUniqueId();
        ImGui::PushID(uid.Value());
        ImGui::TableNextRow();
        if (ImGui::TableNextColumn()) {
          ImGui::Text("%x", uid.Value());
        }
        if (ImGui::TableNextColumn()) {
          ImGui::Text("%s", item->GetName().data());
        }
        if (ImGui::TableNextColumn()) {
          if (ImGui::SmallButton("View")) {
            if (!std::any_of(inspectingEntities.begin(), inspectingEntities.end(),
                             [=](EntityInfo& v) { return v.uid == uid; })) {
              inspectingEntities.emplace_back(uid);
            }
          }
        }
        ImGui::PopID();
      }
      ImGui::EndTable();
    }
    ImGui::End();
  }
}

static bool showEntityInfoWindow(EntityInfo& info) {
  CEntity* ent = g_StateManager->ObjectById(info.uid);
  if (ent == nullptr) {
    return true;
  }
  const char* name = "Entity";
  if (!ent->GetName().empty()) {
    name = ent->GetName().data();
  }
  ImGui::PushID(info.uid.Value());
  if (ImGui::Begin(name, &info.closed, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("ID: %x", info.uid.Value());
    ImGui::Text("Name: %s", ent->GetName().data());
    if (const TCastToPtr<CActor> act = ent) {
      const zeus::CVector3f& pos = act->GetTranslation();
      ImGui::Text("Position: %f, %f, %f", pos.x(), pos.y(), pos.z());
    }
    ImGui::End();
  }
  ImGui::PopID();
  return false;
}

static void ShowAppMainMenuBar() {
  static bool showInspectWindow = false;
  static bool showDemoWindow = false;
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ShowMenuFile();
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Tools")) {
      ImGui::MenuItem("Inspect", nullptr, &showInspectWindow);
      ImGui::Separator();
      ImGui::MenuItem("Demo", nullptr, &showDemoWindow);
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  if (showInspectWindow) {
    ShowInspectWindow(&showInspectWindow);
  }
  {
    auto iter = inspectingEntities.begin();
    while (iter != inspectingEntities.end()) {
      if (iter->closed || showEntityInfoWindow(*iter)) {
        iter = inspectingEntities.erase(iter);
      } else {
        iter++;
      }
    }
  }
  if (showDemoWindow) {
    ImGui::ShowDemoWindow(&showDemoWindow);
  }
}

void ImGuiConsole::proc() { ShowAppMainMenuBar(); }
} // namespace metaforce