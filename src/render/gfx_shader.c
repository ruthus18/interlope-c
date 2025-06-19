#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "gfx_shader.h"

#include "core/log.h"
#include "platform/file.h"
#include "core/types.h"


static inline
bool _check_gl_error() {
    bool is_err = false;
    int gl_err = glGetError();

    while (gl_err != GL_NO_ERROR) {
        log_error("OpenGL Error: %s", gl_err);
        is_err = true;
    }
    return is_err;
}


static inline
void _log_glshader(u32 shader) {
    i32 log_len = 0;
    i32 ch = 0;
    char *log;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        log = (char*) malloc(log_len);

        glGetShaderInfoLog(shader, log_len, &ch, log);
        log_info("Shader log: %s", log);
        free(log);
    }
}


static inline
void _log_glprogram(uint32_t program) {
    i32 log_len = 0;
    i32 ch = 0;
    char *log;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        log = (char*) malloc(log_len);

        glGetProgramInfoLog(program, log_len, &ch, log);
        log_info("Program log: %s", log);
        free(log);
    }
}


static inline
void _shader_compile(i32 program, const char* rel_path, i32 shader_type) {
    u32 shader_id = glCreateShader(shader_type);

    const char* path;
    with_path_to_shader(path, rel_path, {

        const char* content;
        with_file_read(path, content, {
            glShaderSource(shader_id, 1, &content, NULL);
        });
    });

    glCompileShader(shader_id);
    _check_gl_error();

    i32 compile_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_ok);

    if (compile_ok != 1) {
        _log_glshader(shader_id);
        log_exit("Shader compilation error; file=%s", rel_path);
    }

    glAttachShader(program, shader_id);
}


Shader* gfx_shader_create(const char* vert_path, const char* frag_path) {
    i32 program;
    Shader* shader;

    program = glCreateProgram();
    _shader_compile(program, vert_path, GL_VERTEX_SHADER);
    _shader_compile(program, frag_path, GL_FRAGMENT_SHADER);

    shader = malloc(sizeof(Shader));

    i32 link_ok;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (link_ok != 1) {
        _log_glprogram(program);
        log_exit("GL program linking error");
    }

    i32 validation_ok;
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &validation_ok);
    if (validation_ok != 1) {
        _log_glprogram(program);
        log_exit("GL program validation error");
    }

    log_success("Shader created: %s | %s", vert_path, frag_path);
    shader->program_id = program;
    return shader;
}


void gfx_shader_destroy(Shader* shader) {
    glDeleteProgram(shader->program_id);
    free(shader);
}


void gfx_shader_use(Shader* shader) {
    glUseProgram((shader != NULL) ? shader->program_id : 0);
}


#define __NO_UNIFORM -1


void gfx_uniform_set_vec3(Shader* shader, const char* name, vec3 data) {
    i32 uniform_id = glGetUniformLocation(shader->program_id, name);

    if (uniform_id == __NO_UNIFORM) {
        log_exit("Not found shader uniform: %s", name);
    }
    glUniform3f(uniform_id, data[0], data[1], data[2]);
}


void gfx_uniform_set_mat4(Shader* shader, const char* name, mat4 data) {
    i32 uniform_id = glGetUniformLocation(shader->program_id, name);

    if (uniform_id == __NO_UNIFORM) {
        log_exit("Not found shader uniform: %s", name);
    }
    glUniformMatrix4fv(uniform_id, 1, false, (f32*)data);
}


void gfx_uniform_set_float(Shader* shader, const char* name, float value) {
    i32 uniform_id = glGetUniformLocation(shader->program_id, name);

    if (uniform_id == __NO_UNIFORM) {
        log_exit("Not found shader uniform: %s", name);
    }
    glUniform1f(uniform_id, value);
}
