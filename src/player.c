#include "player.h"
#include "camera.h"
#include "cglm/io.h"
#include "physics.h"
#include "platform/input.h"
#include "input_keys.h"
#include <stdio.h>


Player self;

const f64 PLAYER_THICKNESS = 0.4;
const f64 PLAYER_HEIGHT = 1.7;


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


Camera* player_get_camera() {
    return self.camera;
}


void player_update() {
    bool w = input_is_keyrp(IN_KEY_W);
    bool s = input_is_keyrp(IN_KEY_S);
    bool a = input_is_keyrp(IN_KEY_A);
    bool d = input_is_keyrp(IN_KEY_D);

    camera_player_control(self.camera, w, s, a, d);
    camera_upload_to_gfx(self.camera);

    vec3 cam_pos;
    camera_get_position(self.camera, cam_pos);

    vec3 player_pos = {cam_pos[0], cam_pos[1] - PLAYER_HEIGHT, cam_pos[2]};

    
    vec3 physics_pos;
    physics_get_object_position(self.physics_id, physics_pos);
    physics_set_object_position(self.physics_id, (vec3){
        player_pos[0],
        physics_pos[1],
        player_pos[2]
    });

    glm_vec3_print(physics_pos, stdout);

    camera_set_position(self.camera, (vec3){physics_pos[0], physics_pos[1] + PLAYER_HEIGHT / 2, physics_pos[2]});
}
