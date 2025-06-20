#include "player.h"

#include "core/cgm.h"
#include "core/log.h"
#include "core/config.h"
#include "platform/input.h"
#include "platform/time.h"
#include "render/camera.h"
#include "render/gfx.h"
#include "physics.h"

#define PLAYER_WIDTH            0.4
#define PLAYER_HEIGHT           1.7

#define PLAYER_SPEED            4.0
#define PLAYER_JUMP_FORCE       4.5

#define PLAYER_CAM_SENSIVITY    130.0


Player self;

void player_init(vec3 position, vec2 direction) {
    glm_vec3_copy(position, self.position);
    glm_vec2_copy(direction, self.direction);
    glm_vec3_zero(self.velocity);

    self.speed = PLAYER_SPEED;

    self.is_active = true;
    self.is_colliding = true;
    self.is_grounded = true;
    self.is_ceiled = false;

    self.camera_y_offset = PLAYER_HEIGHT;
    self.physics_y_offset = PLAYER_HEIGHT / 2 + 0.15;
    
    self.camera = camera_create(
        (vec3){position[0], position[1] + self.camera_y_offset, position[2]},
        self.direction
    );
    self.physics_id = physics_create_kinematic_object(
        PHYSICS_BODY_CAPSULE,
        (vec3){position[0], position[1] + self.physics_y_offset, position[2]},
        (vec3){0.0, 0.0, 0.0},
        (vec3){PLAYER_WIDTH / 2, PLAYER_HEIGHT, 0.0}
    );
    player_physics_set(self.physics_id);
}


void player_destroy() {
    camera_destroy(self.camera);
}

void player_print() {
    log_debug("--- Player ---");
    log_debug("Ground: %i ; Ceil: %i", self.is_grounded, self.is_ceiled);
    log_debug("Pos: (%.4f  %.4f  %.4f)", self.position[0], self.position[1], self.position[2]);
}

void player_set_active(bool value) {
    self.is_active = value;
}

void player_set_colliding(bool value) {
    self.is_colliding = value;
}

static inline
void _update_camera_rotation() {
    vec2 mouse_delta;
    input_get_mouse_delta(mouse_delta);

    self.direction[0] += mouse_delta[0] * PLAYER_CAM_SENSIVITY;
    self.direction[1] += -mouse_delta[1] * PLAYER_CAM_SENSIVITY;

    // Yaw limit
    if (self.direction[0] >  360.0)  self.direction[0] -= 360.0;
    if (self.direction[0] < -360.0)  self.direction[0] += 360.0;
    // Pitch limit
    if (self.direction[1] >  89.0)   self.direction[1] = 89.0;
    if (self.direction[1] < -89.0)   self.direction[1] = -89.0;

    camera_set_rotation(self.camera, self.direction);
}

static inline
void _calc_input_vec() {
}

static inline
void _calc_movement_vec() {
    glm_vec2_zero(self.v_input);
    glm_vec3_zero(self.v_movement);

    // Calc input vector
    if (input_is_keyrp(IN_KEY_W))  self.v_input[0] += 1.0;
    if (input_is_keyrp(IN_KEY_S))  self.v_input[0] -= 1.0;
    if (input_is_keyrp(IN_KEY_A))  self.v_input[1] -= 1.0;
    if (input_is_keyrp(IN_KEY_D))  self.v_input[1] += 1.0;

    if (glm_vec2_eq(self.v_input, 0.0))  return;

    vec3 v_move_fwd, v_move_side;

    // fwd movement
    glm_vec3_copy(self.camera->v_front, v_move_fwd);
    if (self.is_colliding)
        v_move_fwd[1] = 0.0;  // restrict fly

    // side movement
    glm_vec3_cross(v_move_fwd, V_UP_Y, v_move_side);

    // apply input vector
    glm_vec3_scale(v_move_fwd, self.v_input[0], v_move_fwd);
    glm_vec3_scale(v_move_side, self.v_input[1], v_move_side);

    glm_vec3_add(self.v_movement, v_move_fwd, self.v_movement);
    glm_vec3_add(self.v_movement, v_move_side, self.v_movement);

    if (!glm_vec3_eq(self.v_movement, 0.0))
        glm_vec3_normalize(self.v_movement);
}

static inline
void _calc_vertical_velocity(f32 dt) {
    bool is_jump = input_is_keyp(IN_KEY_SPACE);
    bool apply_gravity = true;

    self.is_grounded = player_physics_is_grounded();
    self.is_ceiled = player_physics_is_ceiled();

    if (self.is_grounded) {
        if (is_jump)
            self.velocity[1] = PLAYER_JUMP_FORCE;

        if (self.velocity[1] < 0.0)
            self.velocity[1] = 0.0;

        apply_gravity = false;
    }
    else if (self.is_ceiled) {
        if (self.velocity[1] > 0.0)
            self.velocity[1] = 0.0;
    }

    if (apply_gravity)
        self.velocity[1] += PHYSICS_GRAVITY * dt;
}


void player_update_physics() {
    f32 dt = time_get_dt();

    _update_camera_rotation();
    _calc_movement_vec();

    if (self.is_colliding)
        _calc_vertical_velocity(dt);

    // Apply speed
    glm_vec3_scale(self.v_movement, self.speed * dt, self.v_movement);

    // Apply velocity
    vec3 velocity_dt;
    glm_vec3_scale(self.velocity, dt, velocity_dt);
    glm_vec3_add(self.v_movement, velocity_dt, self.v_movement);

    // Calc final movement
    if (self.is_colliding)
        player_physics_transform(self.v_movement);
    
    // Apply final movement
    glm_vec3_add(self.position, self.v_movement, self.position);
    camera_set_position(
        self.camera,
        (vec3){self.position[0], self.position[1] + self.camera_y_offset, self.position[2]}
    );
    physics_set_kinematic_position(
        self.physics_id,
        (vec3){self.position[0], self.position[1] + self.physics_y_offset, self.position[2]}
    );
}


void player_update() {
    if (!self.is_active)  return;

    player_update_physics();
    gfx_update_camera(self.camera);
}

Player* player_get() {
    return &self;
}
