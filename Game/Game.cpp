// Game.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Game.h"
#include "ImportableEntity.h"
#include "Material.h"
#include "CubeEntity.h"
#include "PlaneEntity.h"
#include "VulkanMeshComponentManager.h"
#include "KitamoriMovingSystem.h"
#include "RotateSystem.h"
#include "CameraComponent.h"
#include "InputSystem.h"
#include "InputEvents.h"

#include <Engine.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <chrono>
#include <iostream>
#include <map>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, Game& vulkan);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


Game* g_vulkan;

std::set<std::unique_ptr<diffusion::Entity>> s_entity_manager;

void init_components();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GAME, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    Game vulkan; g_vulkan = &vulkan;

    if (!InitInstance (hInstance, nCmdShow, vulkan))
    {
        return FALSE;
    }

    diffusion::s_vulkan_mesh_component_manager.register_material(MaterialType::Opaque, new DefaultMaterial(vulkan));
    diffusion::s_vulkan_mesh_component_manager.register_material(MaterialType::Opaque, new ImportableMaterial(vulkan, "default.png", "default.png"));

    init_components();


    for (int i = 0; i < 1; ++i) {
        vulkan.add_light(glm::vec3(4.0f, -4.0f, -3.0f), glm::vec3(4.0f, -6.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    }
    /*
    vulkan.add_light(glm::vec3(2.0f, 5.0f, 0.0f), glm::vec3(4.0f, 5.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    vulkan.add_light(glm::vec3(6.0f, 5.0f, 0.0f), glm::vec3(4.0f, 5.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    vulkan.add_light(glm::vec3(6.0f, 5.0f, 5.0f), glm::vec3(4.0f, 5.0f, 5.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    vulkan.add_light(glm::vec3(2.0f, 7.0f, 0.0f), glm::vec3(4.0f, 5.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    */

    vulkan.SecondInitialize();


    // Systems Initialization

    diffusion::InputSystem input_system;
    diffusion::move_forward.append([&input_system]() {input_system.move_forward(); });
    diffusion::move_backward.append([&input_system]() {input_system.move_backward(); });
    diffusion::move_left.append([&input_system]() {input_system.move_left(); });
    diffusion::move_right.append([&input_system]() {input_system.move_right(); });
    diffusion::move_up.append([&input_system]() {input_system.move_up(); });
    diffusion::move_down.append([&input_system]() {input_system.move_down(); });


    diffusion::KitamoriMovingSystem kitamori;
    static_cast<diffusion::CameraComponent&>(diffusion::s_component_manager_instance.get_components_by_tag(diffusion::CameraComponent::s_main_camera_component_tag)[0].get()).callback_list.append([&kitamori](glm::vec3 direction) {
        kitamori.update_position(direction);
    });

    diffusion::RotateSystem rotate_system;


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAME));

    std::chrono::steady_clock::time_point time_point = std::chrono::steady_clock::now();

    MSG msg;
    while (TRUE) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            if (std::chrono::steady_clock::now() - time_point > std::chrono::milliseconds(100)) {
                rotate_system.tick();
                time_point = std::chrono::steady_clock::now();
            }
            vulkan.Draw();
        }
    }
    return (int) msg.wParam;
}

void init_components()
{
    s_entity_manager.emplace(std::make_unique<diffusion::CatImportableEntity>(
        *g_vulkan,
        //"E:\\programming\\Graphics\\Game\\Game\\untitled.fbx",
        "E:\\programming\\Graphics\\Game\\Game\\CatWithAnim7.fbx",
        glm::vec3(0, -5, 10),
        glm::vec3(glm::pi<float>() / 2, glm::pi<float>(), -glm::pi<float>() / 2),
        glm::vec3(0.05, 0.05, 0.05)));

    /*
    * griffon
        vulkan,
        "E:\\programming\\Graphics\\Game\\Game\\Griffon.fbx",
        glm::vec3(3, 0, -6),
        glm::vec3(0, 0, 0),
        glm::vec3(0.01, 0.01, 0.01));
    */

    s_entity_manager.emplace(std::make_unique<diffusion::ImportableEntity>(
        *g_vulkan,
        "E:\\programming\\Graphics\\Game\\Game\\uploads_files_2941243_retrotv0319.fbx",
        glm::vec3(4, -4, 1),
        glm::vec3(-glm::pi<float>() / 2, 0, glm::pi<float>()),
        glm::vec3(1, 1, 1)));

    s_entity_manager.emplace(std::make_unique<diffusion::ImportableEntity>(
        *g_vulkan,
        "E:\\programming\\Graphics\\Game\\Game\\uploads_files_2941243_retrotv0319.fbx",
        glm::vec3(4, -5, 5),
        glm::vec3(-glm::pi<float>() / 2, 0, glm::pi<float>()),
        glm::vec3(2, 2, 2)));

    s_entity_manager.emplace(std::make_unique<diffusion::PlaneLitEntity>(*g_vulkan, glm::vec3{0,-5,0}, glm::vec3{ 0,0,0 }, glm::vec3{ 30, 1, 30 }));
    //BoundingSphere{ glm::vec3(0.0f, 0.0f, 0.0f), 3.0f }


    s_entity_manager.emplace(std::make_unique<diffusion::CubePossesedEntity>(*g_vulkan, glm::vec3{ 0, 0, 0 }));
    s_entity_manager.emplace(std::make_unique<diffusion::CubeEntity>(*g_vulkan, glm::vec3{ 3.0, 0, 0 }));
    s_entity_manager.emplace(std::make_unique<diffusion::CubeEntity>(*g_vulkan, glm::vec3{ -3.0, 0, 0 }));


    s_entity_manager.emplace(std::make_unique<diffusion::CubeEntity>(*g_vulkan, glm::vec3{ 15.0,  5, 0 }, glm::vec3{ 0, 0, 0 }, glm::vec3{ 5, 20,40}));
    s_entity_manager.emplace(std::make_unique<diffusion::CubeEntity>(*g_vulkan, glm::vec3{ -15.0, 5, 0 }, glm::vec3{ 0, 0, 0 }, glm::vec3{ 5, 20,40 }));
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GAME);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, Game & vulkan)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   vulkan.Initialize(hInstance, hWnd, 1904, 962);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE:
    {
        // Resize the application to the new window size, except when
        // it was minimized. Vulkan doesn't support images or swapchains
        // with width=0 and height=0.
        if (wParam != SIZE_MINIMIZED) {
            auto width = lParam & 0xffff;
            auto height = (lParam & 0xffff0000) >> 16;

            g_vulkan->Update(width, height);
        }
        break;
    }
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case 'w':
        case 'W':
        {
            diffusion::move_forward();
            break;
        }
        case 's':
        case 'S':
        {
            diffusion::move_backward();
            break;
        }
        case 'a':
        case 'A':
        {
            diffusion::move_left();
            break;
        }
        case 'd':
        case 'D':
        {
            diffusion::move_right();
            break;
        }
        case VK_SPACE:
        {
            diffusion::move_up();
            break;
        }
        case VK_SHIFT:
        {
            diffusion::move_down();
            break;
        }
        default:
            break;
        }
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
