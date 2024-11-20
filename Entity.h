#ifndef ENTITY_H
#define ENTITY_H

#include "Map.h"
#include "glm/glm.hpp"
#include "ShaderProgram.h"
enum EntityType { PLAYER, ENEMY, HEARTS };
enum Animation { IDLE = 0, JUMP = 1, EMPTY = 1, WALK = 2 };
enum AIType { FROG};
enum AIState { IDLING, JUMPGUARDING};

class Entity
{
private:
    bool m_is_active = true;
    // ————— TEXTURES ————— //
    std::vector<GLuint> m_texture_ids;  // Vector of texture IDs for different animations

    // ————— TYPE ————— //
    EntityType m_entity_type;
    AIType m_ai_type;
    AIState m_ai_state;

    // ————— ANIMATIONS ————— //
    std::vector<std::vector<int>> m_animations;  // Indices for each animation type
    int m_animation_cols;
    int m_animation_frames, m_animation_index, m_animation_rows;
    Animation m_current_animation;  // Current animation state
    int* m_animation_indices = nullptr;
    float m_animation_time = 0.0f;

    // ————— TRANSFORMATIONS ————— //
    glm::vec3 m_movement;
    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_velocity;
    glm::vec3 m_acceleration;

    glm::mat4 m_model_matrix;
    float m_speed;
    float m_jumping_power;

    bool m_is_jumping;
    bool m_movingRight = true;

    float m_width = 1.0f;
    float m_height = 1.0f;

    // ————— COLLISIONS ————— //
    bool m_collided_top = false;
    bool m_collided_bottom = false;
    bool m_collided_left = false;
    bool m_collided_right = false;

    //----- Losing a Life Bool ---//
    bool m_lose_life = false;

public:
    static constexpr int SECONDS_PER_FRAME = 6;

    // ————— CONSTRUCTORS ————— //
    Entity();
    Entity(std::vector<GLuint> texture_ids, float speed, float jump_power,
        std::vector<std::vector<int>> animations, float animation_time,
        int animation_frames, int animation_index, int animation_cols,
        int animation_rows, Animation animation, EntityType EntityType, float width, float height);
    ~Entity();

    // ————— GENERAL METHODS ————— //
    void draw_sprite_from_texture_atlas(ShaderProgram* program);
    void update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map);
    void render(ShaderProgram* program);

    // Animation control
    void set_animation_state(Animation new_animation);

    // ————— GETTERS ————— //
    glm::vec3 const get_position() const;
    glm::vec3 const get_velocity()     const;
    glm::vec3 const get_acceleration() const;
    glm::vec3 const get_movement() const;
    glm::vec3 const get_scale() const;
    float     const get_speed()        const;
    bool      const get_collided_top() const;
    bool      const get_collided_bottom() const;
    bool      const get_collided_right() const;
    bool      const get_collided_left() const;
    bool get_lose_life() const;

    // ————— SETTERS ————— //
    void const set_position(glm::vec3 new_position);
    void const set_velocity(glm::vec3 new_velocity);
    void const set_acceleration(glm::vec3 new_acceleration);
    void const set_movement(glm::vec3 new_movement);
    void const set_scale(glm::vec3 new_scale);
    void const set_speed(float new_speed);
    void const set_jumping_power(float new_jumping_power);
    void const set_width(float new_width);
    void const set_height(float new_height);
    void const set_ai_type(AIType new_ai_type);
    void const set_ai_state(AIState new_state);

    // ————— MOVEMENT ————— //
    void move_left();
    void move_right();
    void reset_movement();
    void normalise_movement() { m_movement = glm::normalize(m_movement); }
    void const jump() { m_is_jumping = true; }

    // ————— COLLISIONS ————— //
    bool const check_collision(Entity* other) const;
    void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
    void const check_collision_x(Entity* collidable_entities, int collidable_entity_count);
    // Overloading our methods to check for only the map
    void const check_collision_y(Map* map);
    void const check_collision_x(Map* map);


    // ————— ACTIVATION ————— //
    bool isActive() { return m_is_active; }
    void activate() { m_is_active = true; };
    void deactivate() { m_is_active = false; };

    // ————— AI METHODS ————— //
    void ai_activate(Entity* player);
    void ai_jump(Entity* player);


};
#endif // ENTITY_H
