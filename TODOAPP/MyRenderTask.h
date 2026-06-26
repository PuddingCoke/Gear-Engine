#pragma once

#include<Gear/Core/RenderTask.h>
#include<Gear/DevEssential.h>

#include<fstream>
#include<filesystem>
#include<sstream>

// ============================================================
// TodoTaskManager - 任务存储与 JSON 持久化
// ============================================================
struct TodoTaskManager
{
	std::vector<std::string> tasks;
	std::wstring filePath;

	TodoTaskManager() = default;

	void setPath(const std::wstring& path)
	{
		filePath = path;
		load();
	}

	void load()
	{
		tasks.clear();
		std::ifstream file(filePath);
		if (!file.is_open())
			return;

		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		// 简单解析 JSON 数组 ["str1", "str2", ...]
		std::istringstream stream(content);
		std::string line;
		while (std::getline(stream, line))
		{
			size_t first = line.find_first_of('"');
			if (first == std::string::npos)
				continue;
			size_t last = line.find_last_of('"');
			if (last == std::string::npos || last <= first)
				continue;
			std::string task = line.substr(first + 1, last - first - 1);
			size_t trimFirst = task.find_first_not_of(" \t\r\n");
			if (trimFirst != std::string::npos)
			{
				size_t trimLast = task.find_last_not_of(" \t\r\n");
				task = task.substr(trimFirst, trimLast - trimFirst + 1);
				if (!task.empty())
					tasks.push_back(task);
			}
		}
	}

	void save() const
	{
		std::ofstream file(filePath);
		if (!file.is_open())
			return;

		file << "[\n";
		for (size_t i = 0; i < tasks.size(); i++)
		{
			file << "  \"" << tasks[i] << "\"";
			if (i < tasks.size() - 1)
				file << ",";
			file << "\n";
		}
		file << "]\n";
	}

	void addTask(const std::string& text)
	{
		if (text.empty())
			return;
		tasks.push_back(text);
		save();
	}

	void removeTask(const size_t index)
	{
		if (index >= tasks.size())
			return;
		tasks.erase(tasks.begin() + index);
		save();
	}

	void clearAll()
	{
		tasks.clear();
		save();
	}

	size_t size() const { return tasks.size(); }
};

// ============================================================
// MyRenderTask - TODO 应用渲染任务
// ============================================================
class MyRenderTask : public RenderTask
{
public:
	MyRenderTask()
	{
		const std::wstring solutionFolder = File::getParentFolder(File::getParentFolder(File::getRootFolder()));

		// 引擎已在 RenderEngine 中完成了字体图集构建
		// 无法再动态添加字体；UI 文本统一使用英文
		taskManager.setPath(solutionFolder + L"TODOAPP/tasks.json");
	}

	~MyRenderTask()
	{
		// 窗口关闭时保存
		taskManager.save();
	}

protected:
	void recordCommand() override
	{
		// 仅清屏，所有 UI 由 ImGui 负责
		context->setDefRenderTarget();
		static constexpr float clearColor[4] = { 0.12f, 0.12f, 0.14f, 1.0f };
		context->clearDefRenderTarget(clearColor);
	}

	void imGuiCall() override
	{
		ImGui::PushFont(RenderEngine::getLargeFont());
		renderTodoUI();
		ImGui::PopFont();
	}

private:
	// ----------------------------------------------------
	// TODO UI 渲染
	// ----------------------------------------------------
	void renderTodoUI()
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoScrollbar;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::Begin("##MainWindow", nullptr, windowFlags);
		ImGui::PopStyleVar(2);

		// ---- 输入区域 ----
		renderInputBar();

		ImGui::Spacing();
		ImGui::Separator();

		// ---- 任务列表 ----
		renderTaskList();

		ImGui::Spacing();
		ImGui::Separator();

		// ---- 底部状态栏 ----
		renderFooter();

		ImGui::End();
	}

	// ----------------------------------------------------
	// 输入栏：文本框 + "添加任务"按钮
	// ----------------------------------------------------
	void renderInputBar()
	{
		static char inputBuffer[512] = "";

		ImGui::PushItemWidth(-ImGui::GetFrameHeightWithSpacing() - 60.0f);
		bool enterPressed = ImGui::InputTextWithHint("##TaskInput", "输入一项任务后按 Enter...",
			inputBuffer, sizeof(inputBuffer),
			ImGuiInputTextFlags_EnterReturnsTrue);
		ImGui::PopItemWidth();

		ImGui::SameLine();
		if (ImGui::Button("添加任务", ImVec2(-1.0f, 0.0f)) || enterPressed)
		{
			std::string text(inputBuffer);
			// 去除首尾空白
			size_t start = text.find_first_not_of(" \t\r\n");
			size_t end = text.find_last_not_of(" \t\r\n");
			if (start != std::string::npos)
			{
				text = text.substr(start, end - start + 1);
				taskManager.addTask(text);
			}
			inputBuffer[0] = '\0';
			ImGui::SetKeyboardFocusHere(-1);  // 保持焦点在输入框
		}
	}

	// ----------------------------------------------------
	// 可滚动任务列表
	// ----------------------------------------------------
	void renderTaskList()
	{
		const float footerHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y * 2;
		ImGui::BeginChild("##TaskList", ImVec2(0.0f, -footerHeight), ImGuiChildFlags_Borders);

		if (taskManager.size() == 0)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
			ImGui::TextWrapped("暂无任务。在上方输入任务后按 Enter 或点击\"添加任务\"。");
			ImGui::PopStyleColor();
		}
		else
		{
			size_t removeIndex = SIZE_MAX;

			for (size_t i = 0; i < taskManager.size(); i++)
			{
				ImGui::PushID(static_cast<int>(i));

				ImGui::TextWrapped("%s", taskManager.tasks[i].c_str());
				ImGui::SameLine(ImGui::GetWindowWidth() - 32.0f);
				if (ImGui::SmallButton("×"))
				{
					removeIndex = i;
				}

				ImGui::PopID();
			}

			// 在循环外删除，避免迭代器失效
			if (removeIndex != SIZE_MAX)
			{
				taskManager.removeTask(removeIndex);
			}
		}

		ImGui::EndChild();
	}

	// ----------------------------------------------------
	// 底部：任务计数 + 清空按钮
	// ----------------------------------------------------
	void renderFooter()
	{
		int count = static_cast<int>(taskManager.size());
		ImGui::Text("未完成任务：%d", count);

		ImGui::SameLine(ImGui::GetWindowWidth() - 108.0f);

		if (taskManager.size() > 0)
		{
			if (ImGui::Button("清空全部", ImVec2(96.0f, 0.0f)))
			{
				ImGui::OpenPopup("确认清空");
			}
		}
		else
		{
			ImGui::BeginDisabled();
			ImGui::Button("清空全部", ImVec2(96.0f, 0.0f));
			ImGui::EndDisabled();
		}

		// ---- 确认对话框 ----
		renderClearConfirmPopup();
	}

	void renderClearConfirmPopup()
	{
		if (ImGui::BeginPopupModal("确认清空", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("确定要清空所有未完成任务吗？");
			ImGui::Spacing();

			if (ImGui::Button("确定", ImVec2(120.0f, 0.0f)))
			{
				taskManager.clearAll();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("取消", ImVec2(120.0f, 0.0f)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	TodoTaskManager taskManager;
};