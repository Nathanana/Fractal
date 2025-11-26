import glfw
import moderngl
import numpy as np
import time
from pyrr import Vector3
from camera import Camera
from fractal import Mandelbulb
from shaders import vertex_shader, fragment_shader

# ===================== CONFIG =====================
WIDTH, HEIGHT = 1280, 720
FRACTAL_LOD = 1.0
SENSITIVITY = 0.002
MAX_ITER = 20
BAILOUT = 2.0
POWER = 8.0
ZOOM_FACTOR = 0.5
MIN_SPEED_FLOOR = 1e-8
MAX_SPEED_CAP = 0.5
# ==================================================

if not glfw.init():
    raise Exception("GLFW init failed")

glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3)
glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)

window = glfw.create_window(WIDTH, HEIGHT, "Fractal", None, None)
if not window:
    glfw.terminate()
    raise Exception("GLFW window creation failed")

glfw.make_context_current(window)
glfw.set_input_mode(window, glfw.CURSOR, glfw.CURSOR_DISABLED)

ctx = moderngl.create_context()
prog = ctx.program(vertex_shader=vertex_shader, fragment_shader=fragment_shader)

vertices = np.array([-1,-1, 1,-1, -1,1, -1,1, 1,-1, 1,1], dtype='f4')
vbo = ctx.buffer(vertices.tobytes())
vao = ctx.simple_vertex_array(prog, vbo, 'v_pos')

camera = Camera([0.0, 0.0, -1.5], SENSITIVITY)
fractal = Mandelbulb(power=POWER, bailout=BAILOUT)

def mouse_callback(window, xpos, ypos):
    camera.process_mouse(xpos, ypos)
glfw.set_cursor_pos_callback(window, mouse_callback)

prev_time = time.time()
while not glfw.window_should_close(window):
    current_time = time.time()
    delta = current_time - prev_time
    prev_time = current_time

    width, height = glfw.get_framebuffer_size(window)
    ctx.viewport = (0, 0, width, height)
    ctx.clear(0.0, 0.0, 0.0, 1.0)

    cam_rot, front, right = camera.get_rotation_matrix()

    dist_to_fractal = fractal.DE(camera.position, MAX_ITER)

    proportional_speed = dist_to_fractal * ZOOM_FACTOR
    
    speed = np.clip(proportional_speed, MIN_SPEED_FLOOR, MAX_SPEED_CAP)

    if glfw.get_key(window, glfw.KEY_W) == glfw.PRESS: camera.position += front * speed * delta
    if glfw.get_key(window, glfw.KEY_S) == glfw.PRESS: camera.position -= front * speed * delta
    if glfw.get_key(window, glfw.KEY_A) == glfw.PRESS: camera.position -= right * speed * delta
    if glfw.get_key(window, glfw.KEY_D) == glfw.PRESS: camera.position += right * speed * delta

    if glfw.get_key(window, glfw.KEY_Q) == glfw.PRESS: FRACTAL_LOD *= 1.01
    if glfw.get_key(window, glfw.KEY_E) == glfw.PRESS: FRACTAL_LOD /= 1.01

    FRACTAL_LOD = np.clip(FRACTAL_LOD, 0.1, 10.0)
    base_iter = 20
    MAX_ITER = int(np.clip(base_iter * FRACTAL_LOD, 10, 120))
    
    if glfw.get_key(window, glfw.KEY_ESCAPE) == glfw.PRESS: glfw.set_window_should_close(window, True)

    # Pass uniforms
    prog['iResolution'].value = (width, height)
    prog['camPos'].value = tuple(camera.position)
    prog['camRot'].write(cam_rot.tobytes())
    prog['fractalLod'].value = FRACTAL_LOD
    prog['MAX_ITER'].value = MAX_ITER
    prog['BAILOUT'].value = BAILOUT
    prog['POWER'].value = POWER

    vao.render()
    glfw.swap_buffers(window)
    glfw.poll_events()

glfw.terminate()