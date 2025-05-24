#include "player.h"
#include "../render/camera.h"
#include "../render/gfx.h"
#include "../platform/input.h"
#include "../physics.h"
#include "../core/cgm.h"
#include "../platform/time.h"


Player self;

const f64 PLAYER_THICKNESS = 0.4;
const f64 PLAYER_HEIGHT = 1.7;
#define PLAYER_SPEED 6


void player_init(vec3 pos, f64 pitch, f64 yaw) {
    glm_vec3_copy(pos, self.pos);
    glm_vec2_copy((vec2){pitch, yaw}, self.rot);
    
    self.camera = camera_create();
    vec3 camera_pos = {
        pos[0], pos[1] + PLAYER_HEIGHT, pos[2]
    };
    camera_set_position(self.camera, camera_pos);
    camera_set_rotation(self.camera, pitch, yaw);

    self.physics_id = physics_create_static_object(
        PHYSICS_BODY_CAPSULE,
        (vec3){pos[0], pos[1] + PLAYER_HEIGHT / 2, pos[2]},
        (vec3){0.0, 0.0, 0.0},
        (vec3){PLAYER_THICKNESS / 2, PLAYER_HEIGHT, 0.0}
        // 60.0
    );
}


void player_destroy() {
    camera_destroy(self.camera);
}


void player_handle_movement() {
    // --- Rotation ---
    vec2 mouse_dt;
    input_get_mouse_delta(mouse_dt);
    
    camera_player_rotate(self.camera, mouse_dt);

    // --- Movement ---
    bool w = input_is_keyrp(IN_KEY_W);
    bool s = input_is_keyrp(IN_KEY_S);
    bool a = input_is_keyrp(IN_KEY_A);
    bool d = input_is_keyrp(IN_KEY_D);
    // camera_player_transform(self.camera, w, s, a, d);

    vec3 move_dt;
    cgm_wsad_vec(self.camera->v_front, w, s, a, d, move_dt);
    glm_vec3_scale(move_dt, time_get_dt() * PLAYER_SPEED * 0.5, move_dt);

    camera_transform(self.camera, move_dt);

    gfx_update_camera(self.camera);
}


void player_update() {
    player_handle_movement();
}
