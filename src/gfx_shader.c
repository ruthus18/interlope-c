#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

#include "log.h"
#include "platform/file.h"
#include "gfx_shader.h"
#include "types.h"


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

    const char* path = path_to_shader(rel_path);
    const char* file_buffer = file_read_text(path);

    glShaderSource(shader_id, 1, &file_buffer, NULL);
    free((void*) path);
    free((void*) file_buffer);

    glCompileShader(shader_id);
    _check_gl_error();

    i32 compile_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_ok);

    if (compile_ok != 1) {
        log_error("Shader compilation error; file=%s", rel_path);
        _log_glshader(shader_id);
    }

    glAttachShader(program, shader_id);
}


Shader* shader_create(const char* vert_p, const char* frag_p) {
    Shader* shader = malloc(sizeof(Shader));

    i32 program = glCreateProgram();
    _shader_compile(program, vert_p, GL_VERTEX_SHADER);
    _shader_compile(program, frag_p, GL_FRAGMENT_SHADER);

    i32 link_ok;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (link_ok != 1) {
        log_error("GL program linking error");
        _log_glprogram(program);
    }

    i32 validation_ok;
    glValidateProgram(program);
    glGetProgramiv(program, GL_VALIDATE_STATUS, &validation_ok);
    if (validation_ok != 1) {
        log_error("GL program validation error");
        _log_glprogram(program);
    }

    log_success("Shader created: %s | %s", vert_p, frag_p);
    shader->program_id = program;
    return shader;
}


void shader_destroy(Shader* shader) {
    free(shader);
}


void shader_use(Shader* shader) {
    glUseProgram((shader != NULL) ? shader->program_id : 0);
}


#define __NO_UNIFORM -1


void uniform_set_vec3(Shader* shader, const char* name, vec3 data) {
    i32 uniform_id = glGetUniformLocation(shader->program_id, name);

    if (uniform_id == __NO_UNIFORM) {
        log_error("Not found shader uniform: %s", name);
        exit(EXIT_FAILURE);  // FIXME
    }
    glUniform3f(uniform_id, data[0], data[1], data[2]);
}


void uniform_set_mat4(Shader* shader, const char* name, mat4 data) {
    i32 uniform_id = glGetUniformLocation(shader->program_id, name);

    if (uniform_id == __NO_UNIFORM) {
        log_error("Not found shader uniform: %s", name);
        exit(EXIT_FAILURE);  // FIXME
    }
    glUniformMatrix4fv(uniform_id, 1, false, (f32*)data);
}
