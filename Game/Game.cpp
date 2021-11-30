// Game.cpp : Defines the entry point for the application.
//

#define VK_USE_PLATFORM_WIN32_KHR
#include <vk_mem_alloc.hpp>


#include "framework.h"
#include "Game.h"
#include "Entities/ImportableEntity.h"
#include "Entities/CubeEntity.h"
#include "Entities/PlaneEntity.h"
#include "KitamoriMovingSystem.h"
#include "RotateSystem.h"
#include "BaseComponents/CameraComponent.h"
#include "InputEvents.h"
#include "BaseComponents/PossessedComponent.h"
#include "CameraSystem.h"
#include "Archiver.h"
#include "BaseComponents/Relation.h"
#include "BaseComponents/LitMaterial.h"
#include "BaseComponents/UnlitMaterial.h"
#include "BaseComponents/MeshComponent.h"
#include "Entities/DirectionalLightEntity.h"
#include "BaseComponents/DirectionalLightComponent.h"
#include "BaseComponents/PointLightComponent.h"
#include "BaseComponents/TagComponent.h"
#include "BaseComponents/DebugComponent.h"
#include "Entities/DebugCube.h"

#include <Engine.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <chrono>
#include <iostream>
#include <map>
#include <fstream>

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

void generate_scene();
void import_scene();

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

    generate_scene();
    //import_scene();


    vulkan.SecondInitialize();

    // Systems Initialization
    diffusion::CameraSystem camera_system(vulkan.get_registry());
    diffusion::move_forward.append([&camera_system]() {camera_system.move_forward(0.1f); });
    diffusion::move_backward.append([&camera_system]() {camera_system.move_backward(0.1f); });
    diffusion::move_left.append([&camera_system]() {camera_system.move_left(0.1f); });
    diffusion::move_right.append([&camera_system]() {camera_system.move_right(0.1f); });
    diffusion::move_up.append([&camera_system]() {camera_system.move_up(0.1f); });
    diffusion::move_down.append([&camera_system]() {camera_system.move_down(0.1f); });

    diffusion::KitamoriMovingSystem kitamori(vulkan.get_registry());
    camera_system.callback_list.append([&kitamori](glm::vec3 direction) {
        kitamori.update_position(direction);
    });


    diffusion::RotateSystem rotate_system(vulkan.get_registry());


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

void generate_scene()
{
    auto cat = diffusion::import_entity(
        g_vulkan->get_registry(),
        std::filesystem::path("Models") / "CatWithAnim7.fbx",
        glm::vec3(0, 10, -5),
        glm::vec3(0, 0, 0),
        glm::vec3(0.0005, 0.0005, 0.0005));
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(cat, "SLAVA cat");
    g_vulkan->get_registry().set<diffusion::RotateTag>(cat);
    // for serialization prposes only
    g_vulkan->get_registry().emplace<diffusion::RotateTag>(cat, cat);

    /*
    * griffon
        vulkan,
        "E:\\programming\\Graphics\\Game\\Game\\Griffon.fbx",
        glm::vec3(3, 0, -6),
        glm::vec3(0, 0, 0),
        glm::vec3(0.01, 0.01, 0.01));
    */

    auto tv_1 = diffusion::import_entity(
        g_vulkan->get_registry(),
        std::filesystem::path("Models") / "uploads_files_2941243_retrotv0319.fbx",
        glm::vec3(4, 1, -4),
        glm::vec3(glm::pi<float>() / 2, 0, 0),
        glm::vec3(0.01, 0.01, 0.01));
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(tv_1, "tv 1 - small");

    auto tv_2 = diffusion::import_entity(
        g_vulkan->get_registry(),
        std::filesystem::path("Models") / "uploads_files_2941243_retrotv0319.fbx",
        glm::vec3(4, 5, -5),
        glm::vec3(glm::pi<float>() / 2, 0, 0),
        glm::vec3(0.02, 0.02, 0.02));
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(tv_2, "tv 2 - big");

    auto stool = diffusion::import_entity(
        g_vulkan->get_registry(),
        std::filesystem::path("Models") / "ukopadbaw_LOD3.fbx",
        glm::vec3(-4, 5, -5),
        glm::vec3(glm::pi<float>() / 2, 0, 0),
        glm::vec3(0.1, 0.1, 0.1));
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(stool, "stool");

    auto floor = diffusion::create_plane_entity_lit(g_vulkan->get_registry(), glm::vec3{ 0, 0, -5 }, glm::vec3{ 0,0,0 }, glm::vec3{ 90, 180, 1 });
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(floor, "floor");
    //diffusion::create_plane_entity_lit(g_vulkan->get_registry(), glm::vec3{ 0, 0, -5 }, glm::vec3{ 0,glm::pi<float>(),0 }, glm::vec3{ 30, 90, 1 });
    //BoundingSphere{ glm::vec3(0.0f, 0.0f, 0.0f), 3.0f }


    auto main_entity = diffusion::create_cube_entity_unlit(g_vulkan->get_registry());
    g_vulkan->get_registry().set<diffusion::PossessedEntity>(main_entity);
    g_vulkan->get_registry().set<diffusion::MainCameraTag>(main_entity);
    // for serialization prposes only
    g_vulkan->get_registry().emplace<diffusion::PossessedEntity>(main_entity, main_entity);
    g_vulkan->get_registry().emplace<diffusion::MainCameraTag>(main_entity, main_entity);
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(main_entity, "main cube");

    auto right_kitamori_cube = diffusion::create_cube_entity_unlit(g_vulkan->get_registry(), glm::vec3{ 3.0, 0, 0 });
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(right_kitamori_cube, "right kitamori cube");
    auto left_kitamori_cube = diffusion::create_cube_entity_unlit(g_vulkan->get_registry(), glm::vec3{ -3.0, 0, 0 });
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(left_kitamori_cube, "left kitamori cube");
    auto right_wall = diffusion::create_cube_entity_unlit(g_vulkan->get_registry(), glm::vec3{ 15.0,  0, 5 }, glm::vec3{ 0, 0, 0 }, glm::vec3{ 5, 40,20 });
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(right_wall, "right wall");
    auto left_wall = diffusion::create_cube_entity_unlit(g_vulkan->get_registry(), glm::vec3{ -15.0, 0, 5 }, glm::vec3{ 0, 0, 0 }, glm::vec3{ 5, 40,20 });
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(left_wall, "left wall");

    //diffusion::create_directional_light_entity(g_vulkan->get_registry(), glm::vec3(4.0f, -4.0f, -3.0f), glm::vec3(4.0f, 2.0f, -4.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    //diffusion::create_directional_light_entity(g_vulkan->get_registry(), glm::vec3(8.0f, 3.0f, -3.0f), glm::vec3(4.0f, 3.0f, -4.0f), glm::vec3(0.0f, 0.0f, -1.0f));

    auto directional_light = diffusion::create_directional_light_entity(g_vulkan->get_registry(), glm::vec3(4.0f, -4.0f, -3.0f), glm::vec3(3.0f, -4.0f, -3.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(directional_light, "directional light");
    auto point_light = diffusion::create_point_light_entity(g_vulkan->get_registry(), glm::vec3(0.0f, -4.0f, -1.0f));
    g_vulkan->get_registry().emplace<diffusion::TagComponent>(point_light, "point light");
    //diffusion::create_point_light_entity(g_vulkan->get_registry(), glm::vec3(8.0f, 3.0f, -3.0f));

    /*
    diffusion::create_debug_sphere_entity(
        g_vulkan->get_registry(),
        glm::vec3(0, 1, -3));

    g_vulkan->get_registry().view<diffusion::SubMesh>(entt::exclude<diffusion::debug_tag>).each([](const diffusion::SubMesh & mesh) {
        auto parrent = entt::to_entity(g_vulkan->get_registry(), mesh);

        glm::vec3 delta = mesh.m_bounding_box.max - mesh.m_bounding_box.min;
        glm::vec3 delta_2 = glm::vec3(delta.x / 2, delta.y / 2, delta.z / 2);

        auto entity = diffusion::create_debug_cube_entity(
            g_vulkan->get_registry(),
            mesh.m_bounding_box.min + delta_2,
            glm::vec3(0),
            delta
        );
        g_vulkan->get_registry().emplace<diffusion::Relation>(entity, parrent);
    });
    */

    NJSONOutputArchive output;
    entt::snapshot{ g_vulkan->get_registry() }
        .entities(output)
        .component<diffusion::BoundingComponent, diffusion::CameraComponent, diffusion::SubMesh, diffusion::PossessedEntity,
                   diffusion::Relation, diffusion::LitMaterialComponent, diffusion::UnlitMaterialComponent, diffusion::TransformComponent,
                   diffusion::MainCameraTag, diffusion::DirectionalLightComponent, diffusion::TagComponent, diffusion::PointLightComponent,
                   diffusion::debug_tag /* should be ignored in runtime*/, diffusion::RotateTag>(output);
    output.Close();
    std::string json_output = output.AsString();

    // Scene is generated and exported in sample_scene.json
    std::ofstream fout("sample_scene.json");
    fout << json_output;
}

void import_scene()
{
    std::ifstream fin("sample_scene.json");
    std::string str{ std::istreambuf_iterator<char>(fin),
                     std::istreambuf_iterator<char>() };

    NJSONInputArchive json_in(str);
    entt::basic_snapshot_loader loader(g_vulkan->get_registry());
    loader.entities(json_in)
        .component<diffusion::BoundingComponent, diffusion::CameraComponent, diffusion::SubMesh, diffusion::PossessedEntity,
                   diffusion::Relation, diffusion::LitMaterialComponent, diffusion::UnlitMaterialComponent, diffusion::TransformComponent,
                   diffusion::MainCameraTag, diffusion::DirectionalLightComponent, diffusion::TagComponent, diffusion::RotateTag>(json_in);

    // restore serialized tags
    auto cat = g_vulkan->get_registry().view<diffusion::RotateTag>().front();
    g_vulkan->get_registry().set<diffusion::RotateTag>(cat);

    auto main_entity = g_vulkan->get_registry().view<diffusion::PossessedEntity>().front();
    g_vulkan->get_registry().set<diffusion::PossessedEntity>(main_entity);
    g_vulkan->get_registry().set<diffusion::MainCameraTag>(main_entity);
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
   vulkan.InitializePresentationEngine(vulkan.create_default_presentation_engine(hInstance, hWnd));

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

            //g_vulkan->Update(width, height);
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
