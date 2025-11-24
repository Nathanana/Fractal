import numpy as np

class Mandelbulb:
    def __init__(self, power=8.0, bailout=2.0, base_iter=20):
        self.power = power
        self.bailout = bailout
        self.base_iter = base_iter

    def DE(self, pos, max_iter=None):
        z = np.array(pos, dtype=np.float32)
        dr = 1.0
        r = 0.0
        if max_iter is None:
            max_iter = self.base_iter
        for i in range(max_iter):
            r = np.linalg.norm(z)
            if r > self.bailout:
                break

            theta = np.arccos(np.clip(z[2]/r, -1.0, 1.0))
            phi = np.arctan2(z[1], z[0])

            dr = r**(self.power-1.0) * self.power * dr + 1.0
            zr = r**self.power
            theta *= self.power
            phi *= self.power

            z = zr * np.array([
                np.sin(theta)*np.cos(phi),
                np.sin(theta)*np.sin(phi),
                np.cos(theta)
            ]) + pos
        return 0.5 * np.log(r) * r / dr
