#include "Menu.h"
#include "Utility.h"

constexpr char g_fontSheet[] = "font1.png";
GLuint g_fontSheetTextureID;

void Menu::initialise(){
	//load texture id for font
	g_fontSheetTextureID = Utility::load_texture(g_fontSheet);

	//music and sound effects
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	m_game_state.bgm = Mix_LoadMUS("Canon.mp3");
	Mix_PlayMusic(m_game_state.bgm, -1);
}
//destructor
Menu::~Menu() {
	//free up the background music
	Mix_FreeMusic(m_game_state.bgm);
}
//update doesn't need to do anything
void Menu::update(float delta_time){}

//print out message
void Menu::render(ShaderProgram* program) {
	Utility::draw_text(program, g_fontSheetTextureID, "Press enter to start!", 0.45f, 0.001f, glm::vec3(-4.5f, 0.0f, 0.0f));
}
