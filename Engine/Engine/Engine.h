// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the ENGINE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// ENGINE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

// forward declaration GameImpl to not export Vulkan in Game
struct GameImpl;


// This class is exported from the dll
class ENGINE_API Game {
public:
	Game();
	~Game();

#ifdef _WIN32
	void Initialize(HINSTANCE hinstance, HWND hwnd, int width, int height);
#endif // WIN_32

	void Exit();

	//void AddGameComponent(std::unique_ptr<IGameComponent> component);

	void Draw();
	// TODO: add your methods here.

private:
	GameImpl* m_game_impl;
};
