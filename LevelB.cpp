#include "LevelB.h"
#include "Utility.h"

int LEVEL_WIDTH_B = 20;
int  LEVEL_HEIGHT_B = 7;

//variables for sprite textures
//player
constexpr char g_alienWalk[] = "alienGreen_walk.png";
constexpr char g_alienStand[] = "alienGreen_stand.png";
constexpr char g_alienJump[] = "alienGreen_jump.png";
//frog enemy
constexpr char g_frog[] = "frog.png";
constexpr char g_frogLeap[] = "frog_leap.png";
//hearts
constexpr char g_fullHeart[] = "heart_full.png";
constexpr char g_emptyHeart[] = "heart_empty.png";
//font stuff
constexpr char g_fontSheetB[] = "font1.png";
GLuint g_fontSheetTextureIDB;

unsigned int LEVELB_DATA[] =
{
    12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 41, 43, 0, 0, 0, 0, 41, 43, 0,
    52, 0, 0, 0, 0, 0, 0, 41, 42, 43, 0, 0, 0, 0, 41, 43, 0, 0, 0, 0,
    72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    61, 62, 62, 62, 62, 62, 62, 62, 0, 0, 62, 62, 62, 62, 62, 62, 0, 0, 62, 63,
    121, 122, 122, 122, 122, 122, 122, 122, 0, 0, 122, 122, 122, 122, 122, 122, 0, 0, 122, 123,
    121, 122, 122, 122, 122, 122, 122, 122, 0, 0, 122, 122, 122, 122, 122, 122, 0, 0, 122, 123

};

LevelB::~LevelB()
{
    delete[] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelB::initialise()
{
    //create level map
    GLuint map_texture_id = Utility::load_texture("tilemap.png");
    m_game_state.map = new Map(LEVEL_WIDTH_B, LEVEL_HEIGHT_B, LEVELB_DATA, map_texture_id, 1.0f, 20, 9);
    //load texture id for font
    g_fontSheetTextureIDB = Utility::load_texture(g_fontSheetB);

    //create our player on the heap
    std::vector<std::vector<int>> animationsPlayer = {
        {0}, //standing
        {0}, //jumping
        {0, 1} //walking
    };
    std::vector<GLuint> textureIdsPlayer = { Utility::load_texture(g_alienStand), Utility::load_texture(g_alienJump), Utility::load_texture(g_alienWalk) };
    m_game_state.player = new Entity(
        textureIdsPlayer, //textureID
        1.0f, //speed
        3.0f, //jump power
        animationsPlayer, //animations
        0.0f, //animation time
        1, //frames
        0, //animation index
        1, //cols
        1, //rows
        IDLE, //animation
        PLAYER, //entity type
        1.0f, //width
        1.0f //height
    );
    m_game_state.player->set_position(glm::vec3(1.0f, -1.75f, 0.0f));

    m_game_state.enemies = new Entity[ENEMY_COUNT];
    for (size_t ind = 0; ind < ENEMY_COUNT; ind++) {
        //creating frog enemey
        std::vector<GLuint> textureIdsFrog = { Utility::load_texture(g_frog), Utility::load_texture(g_frogLeap) };
        std::vector<std::vector<int>> animationsFrog = {
            {0}, //idle
            {0}, //jumping
        };
        m_game_state.enemies[ind] = Entity(textureIdsFrog, 1.0f, 1.8f, animationsFrog, 0.0f, 1, 0, 1, 1, IDLE, ENEMY, 0.81f, 0.81f);
        m_game_state.enemies[ind].set_ai_type(FROG);
        m_game_state.enemies[ind].set_ai_state(IDLING);
        m_game_state.enemies[ind].set_animation_state(IDLE);
        //space out enemies
        m_game_state.enemies[ind].set_position(glm::vec3(4.0f + (ind * 5.0f), 0.0f, 0.0f));
    }
    //3 hearts
    m_game_state.hearts = new Entity[3];
    for (size_t ind = 0; ind < 3; ind++) {
        //creating each heart
        std::vector<GLuint> textureIdsHeart = { Utility::load_texture(g_fullHeart), Utility::load_texture(g_emptyHeart) };
        std::vector<std::vector<int>> animationsHeart = {
            {0}, //full
            {0}, //empty
        };
        m_game_state.hearts[ind] = Entity(textureIdsHeart, 1.0f, 1.8f, animationsHeart, 0.0f, 1, 0, 1, 1, IDLE, HEARTS, 1.0f, 1.0f);
        m_game_state.hearts[ind].set_animation_state(IDLE);
        //space out enemies
        m_game_state.hearts[ind].set_position(glm::vec3(ind, 0.0f, 0.0f));
    }
    //update any hearts that need to be empty
    for (int curr = 3; curr > m_game_state.num_lives; curr--) {
        m_game_state.hearts[curr-1].set_animation_state(EMPTY);
    }
    //music and sound effects
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    m_game_state.bgm = Mix_LoadMUS("Canon.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    m_game_state.jump_sfx = Mix_LoadWAV("jump.wav");
}

void LevelB::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);
    //if the player is active and they need to lose a life subtract 1 from number of lives
    if (m_game_state.player->isActive() && m_game_state.player->get_lose_life()) {
        m_game_state.num_lives -= 1;
        m_game_state.hearts[m_game_state.num_lives].set_animation_state(EMPTY);
    }
    //if number of lives equals 0 deactivate the player and lose game
    if (m_game_state.num_lives == 0) {
        m_game_state.player->deactivate();
        lostGame();
    }
    for (int i = 0; i < ENEMY_COUNT; i++)
    {
        m_game_state.enemies[i].update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);
    }
    for (int i = 0; i < 3; i++) {
        m_game_state.hearts[i].update(delta_time, m_game_state.player, NULL, NULL, m_game_state.map);
    }
}


void LevelB::render(ShaderProgram* g_shader_program)
{
    m_game_state.map->render(g_shader_program);
    m_game_state.player->render(g_shader_program);
    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i].render(g_shader_program);
    }
    for (int i = 0; i < 3; i++) {
        m_game_state.hearts[i].render(g_shader_program);
    }
    //if player has lost the game render you lost message
    if (get_game_status() == LOST) {
        if (m_game_state.player->get_position().y <= -5.0f) {
            Utility::draw_text(g_shader_program, g_fontSheetTextureIDB, "You Lost!", 0.5f, 0.001f, glm::vec3(m_game_state.player->get_position().x - 1.0f, -5.0f, 0.0f));
        }
        else {
            Utility::draw_text(g_shader_program, g_fontSheetTextureIDB, "You Lost!", 0.5f, 0.001f, glm::vec3(m_game_state.player->get_position().x - 1.0f, m_game_state.player->get_position().y, 0.0f));
        }
    }
}