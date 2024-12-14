#pragma once
#include <string>
#include <vector>
#include "includes/table/table.h"

namespace App {
	void RenderUI();
	void RenderTable(std::vector<Table> tables);
	void RenderSelectTable(Table table);
}