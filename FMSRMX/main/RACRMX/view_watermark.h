#ifdef VIEWWATERAMRK_EXPORTS
#define VIEWWATERAMRK_API __declspec(dllexport)
#else
#define VIEWWATERAMRK_API __declspec(dllimport)
#endif

#include <string>
#include <tuple>


/*
	must call this function at main_ui_thread, 
	to let all dependent component get initailized.
*/
VIEWWATERAMRK_API bool RPMInitViewOverlayInstance();

/* 
	for calling thread is main ui thread
*/
VIEWWATERAMRK_API bool RPMSetViewOverlay(void* target_window,
	const std::wstring& overlay_text,
	const std::tuple<unsigned char, unsigned char, unsigned char, unsigned char>& font_color,
	const std::wstring& font_name,
	int font_size, int font_rotation, int font_sytle, int text_alignment,
	const std::tuple<int, int, int, int>& display_offset);



VIEWWATERAMRK_API bool RPMSetViewOverlay_Background(void* target_window,
	const std::wstring& overlay_text,
	const std::tuple<unsigned char, unsigned char, unsigned char, unsigned char>& font_color,
	const std::wstring& font_name,
	int font_size, int font_rotation, int font_sytle, int text_alignment,
	const std::tuple<int, int, int, int>& display_offset);


VIEWWATERAMRK_API void  RPMClearViewOverlay(void* target_window);







