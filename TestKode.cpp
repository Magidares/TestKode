#include "Tpch.h"

struct Space {

	GLFWwindow* window;
	int width = 600;
	int height = 650;

	Space() {

		if (!glfwInit()) {
			std::cerr << "failed to init glfw";
			exit(EXIT_FAILURE);
		}

		window = glfwCreateWindow(width, height, "TestKode", nullptr, nullptr);
		if (!window) {
			std::cerr << "failed to init window";
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		glfwMakeContextCurrent(window);
		glViewport(0, 0, width, height);
	}
};
Space space;

int main() {
	FreeConsole();

	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);

	std::ifstream file;
	file.open("src/list.txt", std::fstream::app);

	std::vector<std::string> contents;
	std::string line;

	while (!file.eof())
	{
		std::getline(file, line);
		contents.push_back(line);
	}

	file.close();

	std::vector<std::vector<std::string>> result;
	std::vector<std::vector<std::string>> resultBuffer;

	for (const std::string line : contents)
	{
		std::vector<std::string> words;

		std::stringstream ss(line);
		std::string word;

		while (ss >> word)
		{
			words.push_back(word);
		}
		result.push_back(words);
	}

	resultBuffer = result;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF(
		"src/arial.ttf",
		18.0f,
		nullptr,
		io.Fonts->GetGlyphRangesCyrillic()
	);
	(void)io;
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(space.window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	
	while (!glfwWindowShouldClose(space.window)) {
	
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClear(GL_COLOR_BUFFER_BIT);

		ImGuiID my_dockspace = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
			ImGuiDockNodeFlags_NoUndocking);
		ImGui::SetNextWindowDockID(my_dockspace, ImGuiCond_Once);

		ImGui::Begin(u8"Список");

		for (const auto& row : resultBuffer)
		{
			std::string line;

			for (const auto& item : row)
			{
				line += item + " ";
			}

			ImGui::TextWrapped("%s", line.c_str());
		}

		ImGui::End();

		ImGui::SetNextWindowDockID(my_dockspace, ImGuiCond_Once);

		ImGui::Begin(u8"Действия");
		if (ImGui::Button(u8"Сортировка по расстоянию"))
		{
			std::sort(result.begin(), result.end(),
				[](const std::vector<std::string>& a,
					const std::vector<std::string>& b)
				{
					if (a.size() < 3) return false;
					if (b.size() < 3) return true;

					float ax = std::stof(a[1]);
					float ay = std::stof(a[2]);

					float bx = std::stof(b[1]);
					float by = std::stof(b[2]);

					float distanceA = std::sqrt(ax * ax + ay * ay);
					float distanceB = std::sqrt(bx * bx + by * by);

					return distanceA < distanceB;
				});

			std::vector<std::vector<std::string>> groupedResult;

			std::map<std::string, std::vector<std::vector<std::string>>> groups;

			for (const auto& row : result)
			{
				if (row.size() < 3)
					continue;

				float x = std::stof(row[1]);
				float y = std::stof(row[2]);

				float distance = std::sqrt(x * x + y * y);

				std::string groupName;

				if (distance <= 100.0f)
				{
					groupName = u8"До 100";
				}
				else if (distance <= 1000.0f)
				{
					groupName = u8"До 1000";
				}
				else if (distance <= 10000.0f)
				{
					groupName = u8"До 10000";
				}
				else
				{
					groupName = u8"Слишком далеко";
				}

				groups[groupName].push_back(row);
			}

			std::vector<std::string> order =
			{
				u8"До 100",
				u8"До 1000",
				u8"До 10000",
				u8"Слишком далеко"
			};

			for (const auto& groupName : order)
			{
				auto it = groups.find(groupName);

				if (it == groups.end())
					continue;

				groupedResult.push_back(
					{ "=== " + groupName + " ===" });

				groupedResult.insert(
					groupedResult.end(),
					it->second.begin(),
					it->second.end()
				);
			}

			resultBuffer = groupedResult;
		}
		if (ImGui::Button(u8"Сортировка по имени"))
		{
			std::vector<std::vector<std::string>> groupedResult;

			std::map<std::string, std::vector<std::vector<std::string>>> groups;

			for (const auto& row : result)
			{
				if (row.empty())
					continue;

				const std::string& name = row[0];

				std::string groupName = "#";

				if (!name.empty())
				{
					unsigned char ch = static_cast<unsigned char>(name[0]);

					if (std::isalpha(ch) && ch < 128)
					{
						groupName = std::string(1, static_cast<char>(std::toupper(ch)));
					}

					else
					{

						if ((unsigned char)name[0] == 0xD0 ||
							(unsigned char)name[0] == 0xD1)
						{
							groupName = name.substr(0, 2);

							std::wstring_convert<
							std::codecvt_utf8<wchar_t>> conv;

							std::wstring ws = conv.from_bytes(groupName);

							if (!ws.empty())
							ws[0] = std::towupper(ws[0]);

							groupName = conv.to_bytes(ws);
						}
					}
				}

				groups[groupName].push_back(row);
			}

			for (auto& group : groups)
			{
				auto& rows = group.second;

				std::sort(rows.begin(), rows.end(),
					[](const std::vector<std::string>& a,
						const std::vector<std::string>& b)
					{
						if (a.empty()) return false;
						if (b.empty()) return true;

						return a[0] < b[0];
					});
			}

			for (auto& group : groups)
			{
				groupedResult.push_back(
					{ "=== " + group.first + " ===" });

				groupedResult.insert(
					groupedResult.end(),
					group.second.begin(),
					group.second.end()
				);
			}

			resultBuffer = groupedResult;
		}
		if (ImGui::Button(u8"Сортировка по времени создания"))
		{
			std::vector<std::vector<std::string>> groupedResult;

			std::map<std::string, std::vector<std::vector<std::string>>> groups;

			std::time_t now = std::time(nullptr);

			tm nowTm{};
			localtime_s(&nowTm, &now);

			tm todayTm = nowTm;
			todayTm.tm_hour = 0;
			todayTm.tm_min = 0;
			todayTm.tm_sec = 0;

			std::time_t todayStart = mktime(&todayTm);

			std::time_t yesterdayStart = todayStart - 86400;

			tm weekTm = todayTm;
			weekTm.tm_mday -= (nowTm.tm_wday == 0 ? 6 : nowTm.tm_wday - 1);

			std::time_t weekStart = mktime(&weekTm);

			tm monthTm = todayTm;
			monthTm.tm_mday = 1;

			std::time_t monthStart = mktime(&monthTm);

			tm yearTm = todayTm;
			yearTm.tm_mon = 0;
			yearTm.tm_mday = 1;

			std::time_t yearStart = mktime(&yearTm);

			for (const auto& row : result)
			{
				if (row.size() < 5)
					continue;

				double timestampDouble = std::stod(row[4]);
				std::time_t timestamp = static_cast<std::time_t>(timestampDouble);

				std::string groupName;

				if (timestamp >= todayStart)
				{
					groupName = u8"Сегодня";
				}
				else if (timestamp >= yesterdayStart)
				{
					groupName = u8"Вчера";
				}
				else if (timestamp >= weekStart)
				{
					groupName = u8"На этой неделе";
				}
				else if (timestamp >= monthStart)
				{
					groupName = u8"В этом месяце";
				}
				else if (timestamp >= yearStart)
				{
					groupName = u8"В этом году";
				}
				else
				{
					groupName = u8"Ранее";
				}

				groups[groupName].push_back(row);
			}

			std::vector<std::string> order =
			{
				u8"Сегодня",
				u8"Вчера",
				u8"На этой неделе",
				u8"В этом месяце",
				u8"В этом году",
				u8"Ранее"
			};

			for (const auto& groupName : order)
			{
				auto it = groups.find(groupName);

				if (it == groups.end())
					continue;

				auto& rows = it->second;

				std::sort(rows.begin(), rows.end(),
					[](const std::vector<std::string>& a,
						const std::vector<std::string>& b)
					{
						double timeA = std::stod(a[4]);
						double timeB = std::stod(b[4]);

						return timeA > timeB;
					});

				groupedResult.push_back({ "=== " + groupName + " ===" });

				groupedResult.insert(
					groupedResult.end(),
					rows.begin(),
					rows.end()
				);
			}

			resultBuffer = groupedResult;
		}
		if (ImGui::Button(u8"Сортировка по типу"))
		{
			std::vector<std::vector<std::string>> groupedResult;

			std::map<std::string, std::vector<std::vector<std::string>>> groups;

			for (const auto& row : result)
			{
				if (row.size() < 4)
					continue;

				groups[row[3]].push_back(row);
			}

			std::vector<std::vector<std::string>> miscGroup;

			for (auto& group : groups)
			{
				const std::string& groupName = group.first;
				auto& rows = group.second;

				std::sort(rows.begin(), rows.end(),
					[](const std::vector<std::string>& a,
						const std::vector<std::string>& b)
					{
						return a[0] < b[0];
					});

				if (rows.size() < 3)
				{
					miscGroup.insert(miscGroup.end(), rows.begin(), rows.end());
					continue;
				}

				groupedResult.push_back({ "=== " + groupName + " ===" });

				groupedResult.insert(
					groupedResult.end(),
					rows.begin(),
					rows.end()
				);
			}

			std::sort(miscGroup.begin(), miscGroup.end(),
				[](const std::vector<std::string>& a,
					const std::vector<std::string>& b)
				{
					return a[0] < b[0];
				});

			if (!miscGroup.empty())
			{
				groupedResult.push_back({ u8"=== Разное ===" });

				groupedResult.insert(
					groupedResult.end(),
					miscGroup.begin(),
					miscGroup.end()
				);
			}

			resultBuffer = groupedResult;
		}
		if (ImGui::Button(u8"Сохранить"))
		{
			std::ofstream file("src/list.txt");

			if (file.is_open())
			{
				for (const auto& row : result)
				{
					for (size_t i = 0; i < row.size(); ++i)
					{
						file << row[i];

						if (i < row.size() - 1)
							file << " ";
					}

					file << "\n";
				}

				file.close();
			}
		}
		if (ImGui::Button(u8"Выход"))
		{
			exit(0);
		}
		ImGui::End();

		ImGui::SetNextWindowDockID(my_dockspace, ImGuiCond_Once);

		ImGui::Begin(u8"Добавить строку");

		static char name[128] = "";
		static char x[64] = "";
		static char y[64] = "";
		static char type[128] = "";

		ImGui::InputText(u8"Название", name, IM_ARRAYSIZE(name));
		ImGui::InputText(u8"X", x, IM_ARRAYSIZE(x));
		ImGui::InputText(u8"Y", y, IM_ARRAYSIZE(y));
		ImGui::InputText(u8"Тип", type, IM_ARRAYSIZE(type));

		if (ImGui::Button(u8"Добавить"))
		{
			double timestamp = static_cast<double>(std::time(nullptr));

			result.push_back({
				std::string(name),
				std::string(x),
				std::string(y),
				std::string(type),
				std::to_string(timestamp)
				});

			name[0] = '\0';
			x[0] = '\0';
			y[0] = '\0';
			type[0] = '\0';
		}

		ImGui::End();

		ImGui::Render();

		int display_w, display_h;
		glfwGetFramebufferSize(space.window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(space.window);
	}
	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

}