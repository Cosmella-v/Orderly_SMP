// I HATE IMGUI SO MUCH lol
#include "CreateProfile.hpp"
#include "Settings.hpp"
#include "downloader.hpp"
#include "parser.hpp"
#include "unzipper.hpp"
#include <filesystem>
#include <glad/glad.h>

#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include <GLFW/glfw3.h>

#include <algorithm>
#include <atomic>
#include <thread>

#include "json.hpp"
#include <cpr/cpr.h>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

static bool started = false;
static bool finished = false;
static int resultCode = -1;

#include <iostream>
#include <mutex>
#include <streambuf>
#include <string>
#include <vector>

enum class LogType {
    Info,
    Error
};

struct LogEntry {
    std::string message;
    LogType type;
};


class ImGuiLoggerBuffer : public std::stringbuf {
public:
    ImGuiLoggerBuffer(std::vector<LogEntry>& logs, std::mutex& mutex, LogType type)
        : logLines(logs), logMutex(mutex), logType(type) {}

    ~ImGuiLoggerBuffer() {
        pubsync();
    }

    int sync() override {
        std::lock_guard<std::mutex> lock(logMutex);
        std::string line = str();
        if (!line.empty()) {
            logLines.push_back({ line, logType });
        }
        str("");
        return 0;
    }

private:
    std::vector<LogEntry>& logLines;
    std::mutex& logMutex;
    LogType logType;
};

std::string fetch_url(const std::string &url) {
	auto response = cpr::Get(cpr::Url{url}, cpr::Header{{"User-Agent", "Mozilla/5.0"}});

	if (response.error) {
		std::cerr << "[X] Request error: " << response.error.message << std::endl;
		return {};
	}
	if (response.status_code != 200) {
		std::cerr << "[X] HTTP error: " << response.status_code << std::endl;
		return {};
	}
	return response.text;
}
std::string getLatest() {
	std::string api_url = "https://api.github.com/repos/JuJubeanYT/Orderly_SMP/releases/latest";
	std::string json_data = fetch_url(api_url);
	if (json_data.empty()) {
		std::cerr << "Failed to fetch release info." << std::endl;
		return "";
	}

	try {
		auto json = nlohmann::json::parse(json_data);
		std::string tag_name = json["tag_name"];

		for (const auto &asset : json["assets"]) {
			std::string name = asset["name"];
			if (name.size() >= 7 && name.compare(name.size() - 7, 7, ".mrpack") == 0) {
				std::string download_url = asset["browser_download_url"];
				std::cout << "Release tag: " << tag_name << std::endl;
				std::cout << "Download URL: " << download_url << std::endl;
				return download_url;
				break;
			}
		}
	} catch (const std::exception &e) {
		std::cerr << "[X] JSON parse error: " << e.what() << std::endl;
	}
	return "";
};

int runInstaller() {
	settingsLauncherProcessing.memoryMB = settings.memoryMB;

    #ifdef _WIN32
        FreeConsole();
    #endif

	std::cout << "Getting latest build!\n";
	const std::string url = getLatest();
    if (url.empty()) {
        return 0;
    };
	const std::string mrpackFile = "OrderlySMP.zip";
	const std::string extractDir = "temp_extract";
	const std::string profileDir = "OrderlySMP";
	std::cerr << "[!!!!!!] CLOSE YOUR MINECRAFT LAUNCHER [!!!!!!]\n";
	std::cout << "[*] Downloading mrpack...\n";
	if (!downloadFile(url, mrpackFile)) {
		std::cerr << "[!] Download failed.\n";
		return 1;
	}

	std::cout << "[*] Extracting mrpack...\n";
	if (!unzip_file(mrpackFile, extractDir)) {
		std::cerr << "[!] Unzip failed.\n";
		return 1;
	}

	std::cout << "[*] Parsing modrinth.index.json...\n";
	auto mods = parseModrinthIndex(extractDir + "/modrinth.index.json");

	std::cout << "[*] Downloading mods...\n";
	for (const auto &mod : mods) {
		std::string dest = profileDir + "/" + mod.path;
		std::filesystem::create_directories(std::filesystem::path(dest).parent_path());
		if (!downloadFile(mod.url, dest)) {
			std::cerr << "[!] Failed to download: " << mod.url << "\n";
		}
	}

	std::cout << "[*] Copying overrides...\n";
	copyOverrides(extractDir + "/overrides", profileDir);

	std::cout << "[✓] Created Profile At " << profileDir << "\nImporting this into the mc launcher" << "\n";
	if (fs::is_directory(extractDir)) {
		fs::remove_all(extractDir);
	}
	if (fs::is_regular_file(mrpackFile)) {
		fs::remove_all(mrpackFile);
	}
	Process(settingsLauncherProcessing.memoryMB);
	return 1;
}

ImGuiKey GlfwKeyToImGuiKey(int key) {
	switch (key) {
	case GLFW_KEY_TAB:
		return ImGuiKey_Tab;
	case GLFW_KEY_LEFT:
		return ImGuiKey_LeftArrow;
	case GLFW_KEY_RIGHT:
		return ImGuiKey_RightArrow;
	case GLFW_KEY_UP:
		return ImGuiKey_UpArrow;
	case GLFW_KEY_DOWN:
		return ImGuiKey_DownArrow;
	case GLFW_KEY_PAGE_UP:
		return ImGuiKey_PageUp;
	case GLFW_KEY_PAGE_DOWN:
		return ImGuiKey_PageDown;
	case GLFW_KEY_HOME:
		return ImGuiKey_Home;
	case GLFW_KEY_END:
		return ImGuiKey_End;
	case GLFW_KEY_INSERT:
		return ImGuiKey_Insert;
	case GLFW_KEY_DELETE:
		return ImGuiKey_Delete;
	case GLFW_KEY_BACKSPACE:
		return ImGuiKey_Backspace;
	case GLFW_KEY_SPACE:
		return ImGuiKey_Space;
	case GLFW_KEY_ENTER:
		return ImGuiKey_Enter;
	case GLFW_KEY_ESCAPE:
		return ImGuiKey_Escape;
	case GLFW_KEY_LEFT_SHIFT:
		return ImGuiKey_LeftShift;
	case GLFW_KEY_RIGHT_SHIFT:
		return ImGuiKey_RightShift;
	case GLFW_KEY_LEFT_CONTROL:
		return ImGuiKey_LeftCtrl;
	case GLFW_KEY_RIGHT_CONTROL:
		return ImGuiKey_RightCtrl;
	case GLFW_KEY_LEFT_ALT:
		return ImGuiKey_LeftAlt;
	case GLFW_KEY_RIGHT_ALT:
		return ImGuiKey_RightAlt;
	case GLFW_KEY_LEFT_SUPER:
		return ImGuiKey_LeftSuper;
	case GLFW_KEY_RIGHT_SUPER:
		return ImGuiKey_RightSuper;
	default:
		if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)
			return (ImGuiKey)(ImGuiKey_A + (key - GLFW_KEY_A));
		if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
			return (ImGuiKey)(ImGuiKey_0 + (key - GLFW_KEY_0));
		return ImGuiKey_None;
	}
}

void SetupImGuiInputCallbacks(GLFWwindow *window) {
	// keyboard input
	ImGuiIO &io = ImGui::GetIO();

	glfwSetCharCallback(window, [](GLFWwindow *, unsigned int c) {
		ImGui::GetIO().AddInputCharacter(c);
	});

	glfwSetKeyCallback(window, [](GLFWwindow *, int key, int, int action, int mods) {
		ImGuiIO &io = ImGui::GetIO();
		ImGuiKey imgui_key = GlfwKeyToImGuiKey(key);
		if (imgui_key != ImGuiKey_None)
			io.AddKeyEvent(imgui_key, action != GLFW_RELEASE);

		io.AddKeyEvent(ImGuiMod_Ctrl, (mods & GLFW_MOD_CONTROL) != 0);
		io.AddKeyEvent(ImGuiMod_Shift, (mods & GLFW_MOD_SHIFT) != 0);
		io.AddKeyEvent(ImGuiMod_Alt, (mods & GLFW_MOD_ALT) != 0);
		io.AddKeyEvent(ImGuiMod_Super, (mods & GLFW_MOD_SUPER) != 0);
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow *, int button, int action, int) {
		ImGui::GetIO().AddMouseButtonEvent(button, action == GLFW_PRESS);
	});

	glfwSetScrollCallback(window, [](GLFWwindow *, double xoffset, double yoffset) {
		ImGui::GetIO().AddMouseWheelEvent((float)xoffset, (float)yoffset);
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow *, double xpos, double ypos) {
		ImGui::GetIO().AddMousePosEvent((float)xpos, (float)ypos);
	});

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <GLFW/glfw3.h>
GLFWimage GetGLFWImageFromHICON(HICON hIcon) {
    ICONINFO iconInfo;
    GetIconInfo(hIcon, &iconInfo);

    BITMAP bmpColor;
    GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmpColor);

    int width = bmpColor.bmWidth;
    int height = bmpColor.bmHeight;

    std::vector<unsigned char> pixels(width * height * 4, 0);

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; 
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HBITMAP hBmp = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMem, hBmp);

    DrawIconEx(hdcMem, 0, 0, hIcon, width, height, 0, nullptr, DI_NORMAL);

    GetDIBits(hdcMem, hBmp, 0, height, pixels.data(), &bmi, DIB_RGB_COLORS);

    DeleteObject(hBmp);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    GLFWimage image;
    image.width = width;
    image.height = height;
    image.pixels = new unsigned char[width * height * 4];
    unsigned char* px = pixels.data();
    for (int i = 0; i < width * height; ++i) {
        unsigned char b = px[i * 4 + 0];
        unsigned char g = px[i * 4 + 1];
        unsigned char r = px[i * 4 + 2];
        unsigned char a = px[i * 4 + 3];

        px[i * 4 + 0] = r;
        px[i * 4 + 1] = g;
        px[i * 4 + 2] = b;
        px[i * 4 + 3] = a;
    }
    std::copy(pixels.begin(), pixels.end(), image.pixels); // it doesn't feel safe memcpy here

    return image;
}

void glfwSetWindowIcon_Force(GLFWwindow* window) {
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);

    HICON hIcon = nullptr;
    ExtractIconExW(path, 0, nullptr, &hIcon, 1);
    if (!hIcon) {
       return;
    }

    GLFWimage img = GetGLFWImageFromHICON(hIcon);
    DestroyIcon(hIcon);
    glfwSetWindowIcon(window, 1, &img);
    delete[] img.pixels;
}
#elif defined(__APPLE__)
#include <ImageIO/ImageIO.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <GLFW/glfw3.h>
#include <vector>

void glfwSetWindowIcon_Force(GLFWwindow* window) {
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef iconURL = CFBundleCopyResourceURL(mainBundle, CFSTR("icon"), CFSTR("icns"), nullptr);

    if (!iconURL) {
        return;
    }

    CGImageSourceRef imageSource = CGImageSourceCreateWithURL(iconURL, nullptr);
    CFRelease(iconURL);
    if (!imageSource) {
        return;
    }

    CGImageRef image = CGImageSourceCreateImageAtIndex(imageSource, 0, nullptr);
    CFRelease(imageSource);
    if (!image) {
        return;
    }

    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);
    size_t bytesPerPixel = 4;
    size_t bytesPerRow = width * bytesPerPixel;
    size_t bufferSize = bytesPerRow * height;

    std::vector<unsigned char> pixels(bufferSize);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(
        pixels.data(), width, height, 8, bytesPerRow, colorSpace,
        kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Host);
    CGColorSpaceRelease(colorSpace);

    if (!context) {
        CGImageRelease(image);
        return;
    }

    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);
    CGContextRelease(context);
    CGImageRelease(image);

    GLFWimage glfwImage;
    glfwImage.width = (int)width;
    glfwImage.height = (int)height;
    glfwImage.pixels = new unsigned char[bufferSize];
    std::copy(pixels.begin(), pixels.end(), glfwImage.pixels); // you could memcopy but i feel unsafe about that
    glfwSetWindowIcon(window, 1, &glfwImage);
    delete[] glfwImage.pixels;
}
#else
void glfwSetWindowIcon_Force(GLFWwindow* window) {
// todo (never)
return;
};
#endif

int main() {
	std::cout << "Loading GUI\n";
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
		return -1;
	}

	GLFWwindow *window = glfwCreateWindow(800, 600, "OrderlySMPInstaller", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}


    #ifdef _WIN32
        FreeConsole();
    #endif

    std::cout << "Loaded, If you cannot see it please report this!";

	settings.load();

	glfwMakeContextCurrent(window);
    glfwSetWindowIcon_Force(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize OpenGL loader\n";
		return -1;
	}

    // hooks
	std::vector<LogEntry> logLines;
    std::mutex logMutex;

    ImGuiLoggerBuffer coutBuf(logLines, logMutex, LogType::Info);
    ImGuiLoggerBuffer cerrBuf(logLines, logMutex, LogType::Error);

    std::ostream myCout(&coutBuf);
    std::ostream myCerr(&cerrBuf);

    std::streambuf* oldCoutBuf = std::cout.rdbuf(&coutBuf);
    std::streambuf* oldCerrBuf = std::cerr.rdbuf(&cerrBuf);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();

	ImGuiStyle &style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg].w = 0.9f;

	ImGui_ImplOpenGL3_Init("#version 330");

	SetupImGuiInputCallbacks(window);

	int display_w, display_h;
	std::thread installerThread;
	int currentstate = 0;
	int resultCode = -1;
	path = getDefaultMinecraftPath();
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glfwGetFramebufferSize(window, &display_w, &display_h);
		io.DisplaySize = ImVec2((float)display_w, (float)display_h);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(io.DisplaySize);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
		                                ImGuiWindowFlags_NoResize |
		                                ImGuiWindowFlags_NoCollapse |
		                                ImGuiWindowFlags_NoMove |
		                                ImGuiWindowFlags_NoSavedSettings |
		                                ImGuiWindowFlags_NoBringToFrontOnFocus;

		ImGui::Begin("Install Options", nullptr, window_flags);
		ImGui::Text(" Install the Orderly Smp Profile");
		ImGui::SeparatorText(" Profile Settings");
		ImGui::Text(" Memory (MB)");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		ImGui::SliderInt("##mem_slider", &settings.memoryMB, 1024, 16384, "%d MB");
        if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Recommended: 4096 MB (4 GB) or more for this modpack.");
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100);
		if (ImGui::InputInt("##mem_input", &settings.memoryMB)) {
			settings.memoryMB = std::clamp(settings.memoryMB, 1024, 16384);
		}

		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Recommended: 4096 MB (4 GB) or more for this modpack.");
		}
        ImGui::SameLine();
        if (ImGui::Button("Default")) {
            settings.memoryMB = 4096;
        }

		if (currentstate != 1) {
            ImGui::SeparatorText(" Install Path");
			if (!path.empty()) {
				ImGui::TextWrapped(" Path: %s", path.c_str());
			} else {
				ImGui::TextWrapped(" Path: unknown");
			}
			ImGui::SameLine();
			if (ImGui::Button("Pick Path")) {
				path = promptUserForMinecraftPath2();
			}
			ImGui::SameLine();
			if (ImGui::Button("Try Auto Detect")) {
				path = getDefaultMinecraftPath();
			}
		} else {
            ImGui::SeparatorText(" Current Profile");
        }
		if (currentstate != 1 && ImGui::Button("Start Installation")) {
			settings.save();
            std::lock_guard<std::mutex> lock(logMutex);
            logLines.clear();
            logLines.shrink_to_fit();
			currentstate = 1;
			resultCode = -1;
			installerThread = std::thread([&]() {
				resultCode = runInstaller();
				currentstate = 2;
			});
			installerThread.detach();
		}
        
        if (currentstate == 1) {
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "> Installing...");
            ImGui::Text(" Memory allocated: %s MB", std::to_string(settingsLauncherProcessing.memoryMB).c_str());
			ImGui::SetNextWindowSize(ImVec2(display_w / 1.3, display_h / 1.3), ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2(display_w / 1.3f, display_h / 1.3f), ImGuiCond_Always);
			ImGui::Begin("Installer Logs", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			{
				std::lock_guard<std::mutex> lock(logMutex);

				for (const auto &line : logLines) {
					 switch (line.type) {
                        case LogType::Info:
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 200, 200, 255));
                            break;
                        case LogType::Error:
                            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 100, 255)); 
                            break;
                    }

                    ImGui::TextWrapped("%s", line.message.c_str());
                    ImGui::PopStyleColor();
				}

				if ((ImGui::GetScrollMaxY() - ImGui::GetScrollY()) < 5.0f) {
					ImGui::SetScrollHereY(1.0f);
				}
			}
			ImGui::End();
		} else {
			if (currentstate == 2) {
                ImGui::SeparatorText(" Current Profile");
				if (resultCode == 1) {
					ImGui::TextColored(ImVec4(0, 1, 0, 1), "> Installation complete!");
					ImGui::Text(" Memory allocated: %s MB", std::to_string(settingsLauncherProcessing.memoryMB).c_str());
				} else {
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "> Installation failed.");
					ImGui::SetNextWindowSize(ImVec2(display_w / 1.3, display_h / 1.3), ImGuiCond_Always);
					ImGui::Begin("Installer Logs", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
					{
						std::lock_guard<std::mutex> lock(logMutex);
						for (const auto &line : logLines) {
                            switch (line.type) {
                                case LogType::Info:
                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 200, 200, 255)); 
                                    break;
                                case LogType::Error:
                                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 100, 100, 255));
                                    break;
                            }

                            ImGui::TextWrapped("%s", line.message.c_str());
                            ImGui::PopStyleColor();
                        }

						if ((ImGui::GetScrollMaxY() - ImGui::GetScrollY()) < 5.0f) {
							ImGui::SetScrollHereY(1.0f);
						}
					}
					ImGui::End();
				}
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::Render();
		glViewport(0, 0, display_w, display_h);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	if (installerThread.joinable()) {
		installerThread.join();
	}

	std::cout.rdbuf(oldCoutBuf);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();

	settings.save();
	return 0;
}
