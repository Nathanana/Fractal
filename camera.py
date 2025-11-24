import numpy as np
from pyrr import Vector3

class Camera:
    def __init__(self, position, sensitivity=0.002):
        self.position = Vector3(position)
        self.yaw = 0.0
        self.pitch = 0.0
        self.sensitivity = sensitivity
        self.first_mouse = True
        self.lastX = 0
        self.lastY = 0

    def process_mouse(self, xpos, ypos):
        if self.first_mouse:
            self.lastX, self.lastY = xpos, ypos
            self.first_mouse = False
            return

        xoffset = xpos - self.lastX
        yoffset = self.lastY - ypos
        self.lastX, self.lastY = xpos, ypos

        self.yaw -= xoffset * self.sensitivity
        self.pitch += yoffset * self.sensitivity

        max_pitch = np.pi / 2.0 - 0.01
        self.pitch = np.clip(self.pitch, -max_pitch, max_pitch)

    def get_rotation_matrix(self):
        front = Vector3([
            np.cos(self.pitch) * np.sin(self.yaw),
            np.sin(self.pitch),
            np.cos(self.pitch) * np.cos(self.yaw)
        ]).normalized
        world_up = Vector3([0.0, 1.0, 0.0])
        right = front.cross(world_up).normalized
        up = right.cross(front).normalized
        return np.array([list(right), list(up), list(front)], dtype='f4'), front, right
