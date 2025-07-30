#include "UI/gui.h"
#include "UI/vars.h"
#include "netblocker.h"
#include <thread>
#include <windows.h>
#include "saver.h"
void close()
{
    gui::DestroyImGui();
    gui::DestroyDevice();
    gui::DestroyHWindow();
}

void changeVisibility()
{
    while (varsUI::isRunning)
    {
        if (GetAsyncKeyState(VK_INSERT) & 0x8000)
        {
            varsUI::isVisible = !varsUI::isVisible;
            Sleep(200);
        }
        Sleep(varsUI::uiUpdateTime);
    }
}
int __stdcall wWinMain(
    HINSTANCE instance,
    HINSTANCE previousInstance,
    PWSTR arguments,
    int commandShow)
{

    // create gui
    saver::load();
    gui::CreateHWindow(varsUI::menuName.c_str());
    gui::CreateDevice();
    gui::CreateImGui();
    std::thread(changeVisibility).detach();
    std::thread(netBlock::getInput).detach();

    while (varsUI::isRunning)
    {

        gui::BeginRender();
        gui::Render();
        gui::EndRender();

        Sleep(varsUI::uiUpdateTime);
    }
    saver::save();
    netBlock::UnblockApp(netBlock::attachedRuleName);

    close();
    return EXIT_SUCCESS;
}