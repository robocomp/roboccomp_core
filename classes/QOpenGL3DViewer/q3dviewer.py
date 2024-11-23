# This Python file uses the following encoding: utf-8
import sys

from PySide6.QtWidgets import QApplication, QWidget

# Important:
# You need to run the following command to generate the ui_form.py file
#     pyside6-uic form.ui -o ui_form.py, or
#     pyside2-uic form.ui -o ui_form.py
from ui_form import Ui_Q3DViewer

from PySide6.QtCore import QTimer

import random
from QOpenGL3DViewer import QOpenGL3DViewer, Cube, GroupPoints
import numpy as np


class Q3DViewer(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.ui = Ui_Q3DViewer()
        self.ui.setupUi(self)

        ###ADD In de componet
        self.opengl_widget = QOpenGL3DViewer(FPS=60, parent=self)
        self.ui.viewer.addWidget(self.opengl_widget)
        ###

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.generate_things)
        self.timer.start(100)
    def __del__(self):
         self.timer.stop()


    def generate_things(self):
        """Generar puntos aleatorios para el renderizado."""
        distance = 20
        num_points = self.ui.slider_points.value()//3
        num_cubes = self.ui.slider_cube.value()

        positions = np.random.uniform(-0, 10, 3*num_cubes).reshape((num_cubes,3))
        rotations = np.random.uniform(0, 360, 3*num_cubes).reshape((num_cubes,3))
        sizes = np.random.uniform(-0, 3, 3*num_cubes).reshape((num_cubes,3))
        colors = np.random.uniform(0, 1, 3*num_cubes).reshape((num_cubes,3))
        cubes = np.array(())
        for i in range(num_cubes):
            cube = Cube(
                position=positions[i],
                size=sizes[i],
                rotation=rotations[i],
                color=colors[i]
            )
            cubes = np.append(cubes, cube)

        points_red = GroupPoints(points=np.random.uniform(-distance, distance, num_points*3).reshape((num_points,3)),
                                 color=np.array((1,0,0)))
        points_green = GroupPoints(points=np.random.uniform(-distance, distance, num_points*3).reshape((num_points,3)),
                                 color=np.array((0,1,0)))
        points_blue = GroupPoints(points=np.random.uniform(-distance, distance, num_points*3).reshape((num_points,3)),
                                 color=np.array((0,0,1)))

        self.opengl_widget.set_cube(cubes)
        self.opengl_widget.set_points(np.array((points_red, points_green, points_blue)))

        self.ui.label_cube.setText(f"{num_cubes} CUBES")
        self.ui.label_points.setText(f"{num_points*3} POINTS")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    widget = Q3DViewer()
    widget.show()
    sys.exit(app.exec())
