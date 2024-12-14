#include "sql_app.h"
#include "imgui.h"
#include "includes/sql/sql.h"
#include "includes/table/table.h"

namespace App {
    std::vector<int> selected_fields;
    static std::vector<std::string> insert_buf;
    static bool select_flag;
    void RenderUI() {

        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", nullptr, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Options"))
            {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
                ImGui::MenuItem("Padding", NULL, &opt_padding);
                ImGui::Separator();

                if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode; }
                if (ImGui::MenuItem("Flag: NoDockingSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit; }
                if (ImGui::MenuItem("Flag: NoUndocking", "", (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking; }
                if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
                if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
                if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
                ImGui::Separator();

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::Begin("Settings");
        SQL sql;
        static std::vector<Table> created_tables;
        static std::vector<Table> select_tables;
        static std::vector<string> fields;

        // Choosing which Table will appear in GUI tab
        ImGui::SeparatorText("Tables");
        static std::vector<std::string> tables = {};
        static int table_current_list;
        static int table_current_combo = table_current_list;

        std::vector<const char*> table_items;
        for (const auto &table : tables) {
            table_items.push_back(table.c_str());
        }


        ImGui::ListBox("Tables", &table_current_list, table_items.data(), table_items.size());
        //ImGui::Text(std::to_string(table_current_list).c_str());
        static string selected_table;
        if (tables.size() > 0 && table_current_list >= 0 && table_current_list <= tables.size()) {
            selected_table = tables[table_current_list];
        }

        // Updates tables for list
        if (tables.size() > 0) {
            for (int i = 0; i < tables.size(); i++) {
                table_items[i] = tables[i].c_str();
            }

            // Remove Table Button
            if (ImGui::Button("Remove Table") && table_current_list >= 0) {
                created_tables.erase(created_tables.begin() + table_current_list);
                tables.erase(tables.begin() + table_current_list);
                if (table_current_list >= tables.size()) {
                    table_current_list = tables.size() - 1;
                    table_current_combo = table_current_list;
                }
            }
        }
        else {
            table_current_list = -1;
            table_current_combo = -1;
            ImGui::Text("No tables available.");
        }

        
        if (created_tables.size() > 0) {

            RenderTable(created_tables);
        }


        ImGui::SeparatorText("Commands");
        // Choosing a command to use
        const char* commands[] = { "Create", "Insert", "Select" };
        static int command_current = 0;
        ImGui::ListBox("Commands", &command_current, commands, IM_ARRAYSIZE(commands));

        // Create command:
        static char table_name_buffer[256] = "";
        static std::vector<std::string> field_bufs;
        if (command_current == 0) { 

            // Text box for entering table name
            static ImGuiInputTextFlags flags = ImGuiInputTextFlags_EscapeClearsAll;
            ImGui::InputText("Table name", table_name_buffer, IM_ARRAYSIZE(table_name_buffer), flags);


            static int field_count = 1;
            ImGui::Text("Define Table Fields:");

            if (field_bufs.size() < field_count) {
                field_bufs.push_back("");
            }

            for (int i = 0; i < field_count; i++) {
                char buf[32];
                strncpy_s(buf, field_bufs[i].c_str(), sizeof(buf) - 1);

                if (ImGui::InputText(("Field " + std::to_string(i + 1)).c_str(), buf, sizeof(buf))) {
                    field_bufs[i] = std::string(buf);
                }
                ImGui::SameLine();
                if (ImGui::Button( ("Remove " + std::to_string(i)).c_str())) {
                    field_bufs.erase(field_bufs.begin() + i);
                    field_count--;
                    break;
                }
            }

            if (ImGui::Button("Add Field")) {
                field_bufs.push_back("");
                field_count++;
            }


        }
        else {

            ImGui::Combo("Table", &table_current_combo, table_items.data(), tables.size());

            // Insert Command
            if (command_current == 1 && created_tables.size() > 0) {
                if (table_current_combo >= 0) {
                    insert_buf.resize(created_tables[table_current_combo].get_fields().size());
                    for (int i = 0; i < created_tables[table_current_combo].get_fields().size(); i++) {
                        char buf[32];
                        strncpy_s(buf, insert_buf[i].c_str(), sizeof(buf) - 1);
                        
                        if (ImGui::InputText(("insert_field" + std::to_string(i)).c_str(), buf, sizeof(buf))) {
                            insert_buf[i] = std::string(buf);
                        }
                    }
                }

            }

            // Select Command
            else if (command_current == 2 && created_tables.size() > 0 && table_current_combo >= 0) {
                ImGui::Text("Select Fields");
                fields = created_tables[table_current_combo].get_fields();

                if (selected_fields.size() != fields.size()) {
                    selected_fields.resize(fields.size(), 0);
                }

                for (int i = 0; i < fields.size(); i++) {
                    ImGui::Checkbox(fields[i].c_str(), reinterpret_cast<bool*>(&selected_fields[i]));
                }
            }

        }

        if (ImGui::Button(commands[command_current])) {
            std::string table_name(table_name_buffer);
            // Create   
            if (command_current == 0) {
                auto it = std::find(tables.begin(), tables.end(), table_name);

                if (it == tables.end()) {
                    tables.push_back(table_name_buffer);
                    string command_str = "create table ";
                    command_str += table_name;
                    command_str += " fields ";

                    for (int i = 0; i < field_bufs.size(); i++) {
                        command_str += field_bufs[i];
                        if (field_bufs.size() > 1 && i < field_bufs.size() - 1) {
                            command_str += ", ";
                        }
                    }

                    created_tables.push_back(sql.command(command_str));
                }
                else {
                    ImGui::Text("Table name already exists!");
                }
            }

            // Insert
            else if (command_current == 1 && created_tables.size() > 0) {
                created_tables[table_current_combo].insert_into(insert_buf);

            }

            // Select
            else if (command_current == 2 && created_tables.size() > 0) {
                table_name = tables[table_current_combo];
                vector<string> checked_fields;
                for (int i = 0; i < selected_fields.size(); i++) {
                    if (selected_fields[i] == 1) {
                        checked_fields.push_back(fields[i]);
                    }
                }

                string command_str = "select ";
                for (int i = 0; i < checked_fields.size(); i++) {
                    command_str += checked_fields[i];
                    if (checked_fields.size() > 1 && i < checked_fields.size() - 1) {
                        command_str += ", ";
                    }
                }
                command_str += " from ";
                command_str += table_name;

                select_tables.push_back(sql.command(command_str));
                select_flag = true;
                //RenderSelectTable(sql.command(command_str));
            }
        }

        if (select_flag) {
            RenderSelectTable(select_tables[select_tables.size()-1]);
        }

        ImGui::SeparatorText("Batch");
        ImGui::End();




        ImGui::ShowDemoWindow();
        ImGui::End();

	}

    //void RenderTable(std::string table_name) {
    void RenderTable(std::vector<Table> tables) {
        ImGui::Begin("Tables");
        // Expose a few Borders related flags interactively
        enum ContentsType { CT_Text, CT_FillButton };
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        static bool display_headers = true;
        static int contents_type = CT_Text;

        for (Table table : tables) {

            if (ImGui::BeginTable(table.get_file().c_str(), table.get_num_fields(), flags))
            {
                for (int i = 0; i < table.get_num_fields(); i++) {
                    ImGui::TableSetupColumn(table.get_fields()[i].c_str());
                }
                ImGui::TableHeadersRow();
     
                FileRecord rec;
                fstream f;
                open_fileRW(f, table.get_file().c_str());
                long recno = 0;

                vector<string> record;
                for (int row = 0; row < table.get_last_rec(); row++)
                {
                    rec.read(f, recno);
                    record = rec.get_record();
                    recno++;

                    ImGui::TableNextRow();
                    for (int column = 0; column < table.get_num_fields(); column++)
                    {
                        ImGui::TableSetColumnIndex(column);
                        char buf[32];

                        sprintf_s(buf, record[column].c_str(), column, row);
                        if (contents_type == CT_Text)
                            ImGui::TextUnformatted(buf);
                        else if (contents_type == CT_FillButton)
                            ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    void RenderSelectTable(Table table) {
        ImGui::Begin("Select Table");
        // Expose a few Borders related flags interactively
        enum ContentsType { CT_Text, CT_FillButton };
        static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        static bool display_headers = true;
        static int contents_type = CT_Text;

        if (ImGui::BeginTable(table.get_file().c_str(), table.get_num_fields(), flags))
        {
            for (int i = 0; i < table.get_num_fields(); i++) {
                ImGui::TableSetupColumn(table.get_fields()[i].c_str());
            }
            ImGui::TableHeadersRow();

            FileRecord rec;
            fstream f;
            open_fileRW(f, table.get_file().c_str());
            long recno = 0;

            vector<string> record;
            for (int row = 0; row < table.get_last_rec(); row++)
            {
                rec.read(f, recno);
                record = rec.get_record();
                recno++;

                ImGui::TableNextRow();
                for (int column = 0; column < table.get_num_fields(); column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    char buf[32];

                    sprintf_s(buf, record[column].c_str(), column, row);
                    if (contents_type == CT_Text)
                        ImGui::TextUnformatted(buf);
                    else if (contents_type == CT_FillButton)
                        ImGui::Button(buf, ImVec2(-FLT_MIN, 0.0f));
                }
            }
            ImGui::EndTable();
        }
        ImGui::End();
    }
} 