/**
* Author: Jemima Datus
* Assignment: Platformer
* Date due: 2023-11-23, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL_RIGHT_EDGE 20.0f
#define LEVEL_BOTTOM_EDGE -6.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "Menu.h"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.h"

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH  = 640 * 1.5f,
          WINDOW_HEIGHT = 480 * 1.5f;

// Background color components
constexpr float BG_RED = 0.875f,
BG_BLUE = 0.965f,
BG_GREEN = 0.961f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

enum AppStatus { RUNNING, TERMINATED };

// ————— GLOBAL VARIABLES ————— //
Scene *g_current_scene;
Menu* g_menu;
LevelA* g_level_a;
LevelB* g_level_b;
LevelC* g_level_c;

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

void switch_to_scene(Scene *scene)
{
    //trade value for number of lives
    if (g_current_scene != nullptr) {
        scene->set_num_lives(g_current_scene->get_num_lives());
    }
    g_current_scene = scene;
    g_current_scene->initialise();
}

void initialise();
void process_input();
void update();
void render();
void shutdown();


void initialise()
{
    // ————— VIDEO ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Platformer!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        shutdown();
    }
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    // ————— GENERAL ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    //menu
    g_menu = new Menu();
    //level A
    g_level_a = new LevelA();
    //level B
    g_level_b = new LevelB();
    //level C
    g_level_c = new LevelC();

    //set current scene to menu
    switch_to_scene(g_menu);
    //set initial number of lives to 3
    g_current_scene->set_num_lives(3);
    
    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{    
 
    SDL_Event event;
    while (SDL_PollEvent(&event)){
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE){
                g_app_status = TERMINATED;
        }
    }
    
    //get keyboard state
    const Uint8* keyState = SDL_GetKeyboardState(NULL);
    //only check for movement if we are not on the menu and the game is still being played
    if (g_current_scene != g_menu && g_current_scene->get_game_status() == PLAYING) {
        //reset movement
        g_current_scene->get_state().player->reset_movement();
        if (keyState[SDL_SCANCODE_LEFT]) { //player is moving left
            g_current_scene->get_state().player->move_left();
            g_current_scene->get_state().player->set_animation_state(WALK);
        }
        else if (keyState[SDL_SCANCODE_RIGHT]) {//player is moving right
            g_current_scene->get_state().player->move_right();
            g_current_scene->get_state().player->set_animation_state(WALK);
        }
        else if (keyState[SDL_SCANCODE_UP] && g_current_scene->get_state().player->get_collided_bottom()) {//player is jumping, check for bottom collision to prevent moon jump
            //play jump sound effect
            Mix_PlayChannel(-1, g_current_scene->get_state().jump_sfx, 0);
            g_current_scene->get_state().player->jump();
            g_current_scene->get_state().player->set_animation_state(JUMP);
        }
        else if (!g_current_scene->get_state().player->get_collided_bottom()) { //if the player is in the air use jump animation
            g_current_scene->get_state().player->set_animation_state(JUMP);
        }
        else {
            g_current_scene->get_state().player->set_animation_state(IDLE);

        }
        //normalize player movement
        if (glm::length(g_current_scene->get_state().player->get_movement()) > 1.0f)
            g_current_scene->get_state().player->normalise_movement();
    }
    //if our current scene is the menu then check if the user pressed enter
    else if(g_current_scene == g_menu && g_current_scene->get_game_status() == PLAYING){ 
        if (keyState[SDL_SCANCODE_RETURN]) {//if the press return switch to level a
            switch_to_scene(g_level_a);
        }
    }
    

    
 
}

void update()
{
    // ————— DELTA TIME / FIXED TIME STEP CALCULATION ————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        // ————— UPDATING THE SCENE (i.e. map, character, enemies...) ————— //
        g_current_scene->update(FIXED_TIMESTEP);
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    //only move camera if the current scene is not the menu
    if (g_current_scene != g_menu) {
        // ————— PLAYER CAMERA ————— //
        g_view_matrix = glm::mat4(1.0f);
        //check veritcal bound
        if (g_current_scene->get_state().player->get_position().y < LEVEL_BOTTOM_EDGE) {
            //stop following the player with the camera
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, -LEVEL_BOTTOM_EDGE, 0.0f));
            //end the game
            g_current_scene->lostGame();
        }else {
            //make camera follow player
            g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->get_state().player->get_position().x, -g_current_scene->get_state().player->get_position().y, 0));
        }
    }
   
    //check bound for level a
    if (g_current_scene == g_level_a && g_current_scene->get_state().player->get_position().x > LEVEL_RIGHT_EDGE) {
        //if they have reached the end of level a switch to level b
        switch_to_scene(g_level_b);
    }
    //check bound for level b
    if (g_current_scene == g_level_b && g_current_scene->get_state().player->get_position().x > LEVEL_RIGHT_EDGE) {
        //if they have reached the end of level b switch to level c
        switch_to_scene(g_level_c);
    }
    //check bound for level c
    if (g_current_scene == g_level_c && g_current_scene->get_state().player->get_position().x > LEVEL_RIGHT_EDGE) {
        //if the player has reached the end of level c they have won the game
        g_current_scene->wonGame();
    }
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    // ————— RENDERING THE SCENE (i.e. map, character, enemies...) ————— //
    g_current_scene->render(&g_shader_program);

    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{    
    SDL_Quit();
    
    //delete all leves
    delete g_menu;
    delete g_level_a;
    delete g_level_b;
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
