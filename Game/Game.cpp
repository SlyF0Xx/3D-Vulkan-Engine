// Game.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Game.h"
#include "PrimitiveMesh.h"
#include "ImportableEntity.h"
#include "Material.h"
#include "CubeEntity.h"
#include "PlaneEntity.h"
#include "VulkanMeshComponentManager.h"

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
BOOL                InitInstance(HINSTANCE, int, Game& vulkan, const glm::mat4& camera_matrix, const glm::mat4& projectionMatrix);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

Game* g_vulkan;
glm::mat4 g_camera_matrix;
glm::mat4 g_projectionMatrix;
glm::vec3 cameraPosition{ 0.0f, 0.0f, -10.0f };
glm::vec3 cameraTarget{ 0.0f, 0.0f, 0.0f };
glm::vec3 upVector{ 0.0f, -1.0f, 0.0f };
/*
std::vector<PrimitiveMesh*> components;
std::vector<PrimitiveMesh*> potential_linked_components;
std::vector<PrimitiveMesh*> linked_components;
*/

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

    g_camera_matrix = glm::lookAt(
        cameraPosition, // Позиция камеры в мировом пространстве
        cameraTarget,   // Указывает куда вы смотрите в мировом пространстве
        upVector        // Вектор, указывающий направление вверх. Обычно (0, 1, 0)
    );

    g_projectionMatrix = glm::perspective(
        static_cast<float>(glm::radians(60.0f)),  // Вертикальное поле зрения в радианах. Обычно между 90&deg; (очень широкое) и 30&deg; (узкое)
        16.0f / 9.0f,                          // Отношение сторон. Зависит от размеров вашего окна. Заметьте, что 4/3 == 800/600 == 1280/960
        0.1f,                                  // Ближняя плоскость отсечения. Должна быть больше 0.
        100.0f                                 // Дальняя плоскость отсечения.
    );

    Game vulkan; g_vulkan = &vulkan;

    if (!InitInstance (hInstance, nCmdShow, vulkan, g_camera_matrix, g_projectionMatrix))
    {
        return FALSE;
    }


    for (int i = 0; i < 1; ++i) {
        vulkan.add_light(glm::vec3(4.0f, 5.0f, -3.0f), glm::vec3(4.0f, 5.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    }
    /*
    vulkan.add_light(glm::vec3(2.0f, 5.0f, 0.0f), glm::vec3(4.0f, 5.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    vulkan.add_light(glm::vec3(6.0f, 5.0f, 0.0f), glm::vec3(4.0f, 5.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    vulkan.add_light(glm::vec3(6.0f, 5.0f, 5.0f), glm::vec3(4.0f, 5.0f, 5.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    vulkan.add_light(glm::vec3(2.0f, 7.0f, 0.0f), glm::vec3(4.0f, 5.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    */

    diffusion::s_vulkan_mesh_component_manager.register_material(MaterialType::Opaque, new DefaultMaterial(vulkan));

    vulkan.update_camera_projection_matrixes(g_camera_matrix, g_projectionMatrix);

    diffusion::ImportableEntity cat(
        vulkan,
        "E:\\programming\\Graphics\\Game\\Game\\CatWithAnim7.fbx",
        glm::vec3(0, 3, 50),
        glm::vec3(glm::pi<float>() / 2, glm::pi<float>(), -glm::pi<float>() / 2),
        glm::vec3(0.1, 0.1, 0.1));

    /*
    * griffon
        vulkan,
        "E:\\programming\\Graphics\\Game\\Game\\Griffon.fbx",
        glm::vec3(3, 0, -6),
        glm::vec3(0, 0, 0),
        glm::vec3(0.01, 0.01, 0.01));
    */

    diffusion::ImportableEntity mandalorez(
        vulkan,
        "E:\\programming\\Graphics\\Game\\Game\\uploads_files_2941243_retrotv0319.fbx",
        glm::vec3(4, 4, 1),
        glm::vec3(-glm::pi<float>() / 2, 0, glm::pi<float>()),
        glm::vec3(1, 1, 1));

    diffusion::ImportableEntity tv(
        vulkan,
        "E:\\programming\\Graphics\\Game\\Game\\uploads_files_2941243_retrotv0319.fbx",
        glm::vec3(4, 3, 5),
        glm::vec3(-glm::pi<float>() / 2, 0, glm::pi<float>()),
        glm::vec3(2, 2, 2));

    diffusion::PlaneEntity plane(vulkan);
    //BoundingSphere{ glm::vec3(0.0f, 0.0f, 0.0f), 3.0f },
    //components.push_back(plane);


    diffusion::CubeEntity cube1(vulkan, { 0, 0, 0 });
    //BoundingSphere{ glm::vec3(-0.5f, 0.5f, 0.6f), 0.25f },
    //components.push_back(component);
    //linked_components.push_back(components.back());

    diffusion::CubeEntity cube2(vulkan, { 3.0, 0, 0 });
    //BoundingSphere{ glm::vec3(-0.5f, 0.5f, 0.6f), 0.25f },
    //components.push_back(component);
    //potential_linked_components.push_back(components.back());

    diffusion::CubeEntity cube3(vulkan, { -3.0, 0, 0 });
    //BoundingSphere{ glm::vec3(-0.5f, 0.5f, 0.6f), 0.25f },
    //components.push_back(component);
    //potential_linked_components.push_back(components.back());



    vulkan.SecondInitialize();



    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAME));


    glm::mat4 translation_matrix = glm::translate(glm::mat4(1), glm::vec3(3, 0, 0));

    glm::mat4 rotation_matrix(1);
    glm::vec3 RotationZ(0, 0, 1.0);

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
            /*
            if (std::chrono::steady_clock::now() - time_point > std::chrono::milliseconds(100)) {
                rotation_matrix = glm::rotate(glm::mat4(1.0f), 0.01f, RotationZ);
                for (auto& component : mandalorez.get_game_components()) {
                    component.UpdateWorldMatrix(component.get_world_matrix() * rotation_matrix);
                }
                //tv->UpdateWorldMatrix(rotation_matrix * translation_matrix);

                time_point = std::chrono::steady_clock::now();
            }
            */
            vulkan.Draw();
        }
    }
    return (int) msg.wParam;
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, Game & vulkan, const glm::mat4 & camera_matrix, const glm::mat4& projectionMatrix)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   vulkan.Initialize(hInstance, hWnd, 1904, 962, camera_matrix, projectionMatrix);

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
        /*
        switch (wParam)
        {
        case 'w':
        case 'W':
        {
            glm::vec3 direction = glm::normalize(cameraTarget - cameraPosition) * 0.1f;
            cameraPosition += direction;
            cameraTarget += direction;

            for (auto& linked_component : linked_components) {
                linked_component->UpdateWorldMatrix(glm::translate(linked_component->get_world_matrix(), direction));
            }

            for (auto it = potential_linked_components.begin(); it != potential_linked_components.end(); ) {
                if (linked_components.front()->Intersect(**it)) {
                    linked_components.push_back(*it);
                    it = potential_linked_components.erase(it);
                }
                else {
                    ++it;
                }
            }

            break;
        }
        case 's':
        case 'S':
        {
            glm::vec3 direction = glm::normalize(cameraTarget - cameraPosition) * 0.1f;
            cameraPosition -= direction;
            cameraTarget -= direction;

            for (auto& linked_component : linked_components) {
                linked_component->UpdateWorldMatrix(glm::translate(linked_component->get_world_matrix(), -direction));
            }

            for (auto it = potential_linked_components.begin(); it != potential_linked_components.end(); ) {
                if (linked_components.front()->Intersect(**it)) {
                    linked_components.push_back(*it);
                    it = potential_linked_components.erase(it);
                }
                else {
                    ++it;
                }
            }

            break;
        }
        case 'a':
        case 'A':
        {
            glm::vec3 forward_vec = glm::normalize(cameraTarget - cameraPosition);
            glm::vec3 direction = glm::cross(forward_vec, upVector) * 0.1f;
            cameraPosition -= direction;
            cameraTarget -= direction;

            for (auto& linked_component : linked_components) {
                linked_component->UpdateWorldMatrix(glm::translate(linked_component->get_world_matrix(), -direction));
            }

            for (auto it = potential_linked_components.begin(); it != potential_linked_components.end(); ) {
                if (linked_components.front()->Intersect(**it)) {
                    linked_components.push_back(*it);
                    it = potential_linked_components.erase(it);
                }
                else {
                    ++it;
                }
            }

            break;
        }
        case 'd':
        case 'D':
        {
            glm::vec3 forward_vec = glm::normalize(cameraTarget - cameraPosition);
            glm::vec3 direction = glm::cross(forward_vec, upVector) * 0.1f;
            cameraPosition += direction;
            cameraTarget += direction;

            for (auto& linked_component : linked_components) {
                linked_component->UpdateWorldMatrix(glm::translate(linked_component->get_world_matrix(), direction));
            }

            for (auto it = potential_linked_components.begin(); it != potential_linked_components.end(); ) {
                if (linked_components.front()->Intersect(**it)) {
                    linked_components.push_back(*it);
                    it = potential_linked_components.erase(it);
                }
                else {
                    ++it;
                }
            }

            break;
        }
        case VK_SPACE:
        {
            cameraPosition += glm::vec3(upVector * 0.1f);
            cameraTarget += glm::vec3(upVector * 0.1f);

            for (auto& linked_component : linked_components) {
                linked_component->UpdateWorldMatrix(glm::translate(linked_component->get_world_matrix(), upVector * 0.1f));
            }

            for (auto it = potential_linked_components.begin(); it != potential_linked_components.end(); ) {
                if (linked_components.front()->Intersect(**it)) {
                    linked_components.push_back(*it);
                    it = potential_linked_components.erase(it);
                }
                else {
                    ++it;
                }
            }
            break;
        }
        case VK_SHIFT:
        {
            cameraPosition -= glm::vec3(upVector * 0.1f);
            cameraTarget -= glm::vec3(upVector * 0.1f);

            for (auto& linked_component : linked_components) {
                linked_component->UpdateWorldMatrix(glm::translate(linked_component->get_world_matrix(), -upVector * 0.1f));
            }

            for (auto it = potential_linked_components.begin(); it != potential_linked_components.end(); ) {
                if (linked_components.front()->Intersect(**it)) {
                    linked_components.push_back(*it);
                    it = potential_linked_components.erase(it);
                }
                else {
                    ++it;
                }
            }
            break;
        }
        default:
            break;
        }
        g_camera_matrix = glm::lookAt(
            cameraPosition, // Позиция камеры в мировом пространстве
            cameraTarget,   // Указывает куда вы смотрите в мировом пространстве
            upVector        // Вектор, указывающий направление вверх. Обычно (0, 1, 0)
        );

        g_vulkan->update_camera_projection_matrixes(g_camera_matrix, g_projectionMatrix);
        */
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
