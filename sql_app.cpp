#include "sql_app.h"
#include "imgui.h"

namespace App {
	void RenderUI() {
		ImGui::Begin("Settings");
		ImGui::Button("Hello");
		static float value = 0.0f;
		ImGui::DragFloat("Value", &value);
		ImGui::End();
	}
}