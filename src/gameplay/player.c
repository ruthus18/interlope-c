#include "player.h"
#include "../core/cgm.h"
#include "../core/config.h"
#include "../platform/input.h"
#include "../platform/time.h"
#include "../render/camera.h"
#include "../render/gfx.h"
#include "../physics.h"


Player self;


void player_init(vec3 pos, vec2 rot) {
    glm_vec3_copy(pos, self.pos);
    glm_vec2_copy(rot, self.rot);

    self.velocity_y = 0.0f;

    self.is_grounded = true;  // Start grounded so player can move immediately
    self.is_active = true;
    self.gravity_enabled = true;
    
    self.camera = camera_create();
    vec3 camera_pos = {
        pos[0], pos[1] + PLAYER_HEIGHT, pos[2]
    };
    camera_set_position(self.camera, camera_pos);
    camera_set_rotation(self.camera, rot[0], rot[1]);

    self.physics_id = physics_create_kinematic_object(
        PHYSICS_BODY_CAPSULE,
        (vec3){pos[0], pos[1] + PLAYER_HEIGHT / 2 + 0.15, pos[2]},
        (vec3){0.0, 0.0, 0.0},
        (vec3){PLAYER_WIDTH / 2, PLAYER_HEIGHT, 0.0}
    );
}


void player_destroy() {
    camera_destroy(self.camera);
}

void player_set_is_active(bool value) {
    self.is_active = value;
}

void player_set_gravity_enabled(bool value) {
    self.gravity_enabled = value;
}


void player_handle_movement() {
    f32 dt = time_get_dt();
    
    // --- Rotation ---
    vec2 mouse_dt;
    input_get_mouse_delta(mouse_dt);
    camera_player_rotate(self.camera, mouse_dt);

    // --- Horizontal Movement ---
    bool w = input_is_keyrp(IN_KEY_W);
    bool s = input_is_keyrp(IN_KEY_S);
    bool a = input_is_keyrp(IN_KEY_A);
    bool d = input_is_keyrp(IN_KEY_D);
    bool space = input_is_keyrp(IN_KEY_SPACE);

    vec3 move_dt;
    cgm_wsad_vec(self.camera->v_front, w, s, a, d, move_dt);
    glm_vec3_scale(move_dt, dt * PLAYER_SPEED * 0.5, move_dt);
    move_dt[1] = 0.0f; // No Y movement from WASD

    // --- Ground Detection ---
    vec3 ground_check_pos = {
        self.pos[0], self.pos[1] + PLAYER_HEIGHT / 2, self.pos[2]
    };
    self.is_grounded = physics_check_ground_collision(self.physics_id, ground_check_pos);

    // --- Jump ---
    if (space && self.is_grounded) {
        self.velocity_y = PLAYER_JUMP_FORCE;
        self.is_grounded = false;
    }

    // --- Gravity ---
    if (self.is_grounded || !self.gravity_enabled) {
        self.velocity_y = 0.0f;
    }
    else {
        self.velocity_y += PHYSICS_GRAVITY * dt;
    }

    // --- Apply Movement with Per-Axis Collision Detection ---
    vec3 allowed_move = {move_dt[0], self.velocity_y * dt, move_dt[2]};
    
    // Try X movement separately
    vec3 x_test_pos;
    glm_vec3_copy(self.pos, x_test_pos);
    x_test_pos[0] += move_dt[0];
    vec3 x_physics_pos = {
        x_test_pos[0], x_test_pos[1] + PLAYER_HEIGHT / 2 + 0.15, x_test_pos[2]
    };
    
    if (physics_check_collision_at_position(self.physics_id, x_physics_pos)) {
        allowed_move[0] = 0.0f;  // Block X movement
    }
    
    // Try Z movement separately (with successful X movement included)
    vec3 z_test_pos;
    glm_vec3_copy(self.pos, z_test_pos);
    z_test_pos[0] += allowed_move[0];  // Include successful X movement
    z_test_pos[2] += move_dt[2];
    vec3 z_physics_pos = {
        z_test_pos[0], z_test_pos[1] + PLAYER_HEIGHT / 2 + 0.15, z_test_pos[2]
    };
    
    if (physics_check_collision_at_position(self.physics_id, z_physics_pos)) {
        allowed_move[2] = 0.0f;  // Block Z movement
    }
    
    vec3 final_move = {allowed_move[0], allowed_move[1], allowed_move[2]};
    
    // Apply final movement
    vec3 new_pos;
    glm_vec3_add(self.pos, final_move, new_pos);
    
    // Check for ground collision if falling
    if (self.velocity_y < 0) {
        vec3 new_physics_pos = {
            new_pos[0], new_pos[1] + PLAYER_HEIGHT / 2 + 0.15, new_pos[2]
        };
        if (physics_check_ground_collision(self.physics_id, new_physics_pos)) {
            final_move[1] = 0.0f;
            self.velocity_y = 0.0f;
            self.is_grounded = true;
            glm_vec3_add(self.pos, (vec3){final_move[0], 0, final_move[2]}, new_pos);
        }
    }

    // Apply final movement
    camera_transform(self.camera, final_move);
    glm_vec3_copy(new_pos, self.pos);
    vec3 final_physics_pos = {
        self.pos[0], self.pos[1] + PLAYER_HEIGHT / 2 + 0.15, self.pos[2]
    };
    physics_set_kinematic_position(self.physics_id, final_physics_pos);

    gfx_update_camera(self.camera);
}


void player_update() {
    if (self.is_active) {
        player_handle_movement();
    }
}
