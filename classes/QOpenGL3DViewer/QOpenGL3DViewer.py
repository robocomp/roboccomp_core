"""
Author: Alejandro Torrejón Harto
Date: 24/10/2024
Description: This module implements a 3D viewer using OpenGL within a Qt widget. 
             It allows rendering cubes and points in a 3D space with camera controls 
             via mouse interaction (rotation, translation, and zoom).
             
Usage:
    In your QWidget, add this line to initialize the viewer:
        self.opengl_widget = QOpenGL3DViewer(FPS=60, parent=self)
        self.ui.viewer.addWidget(self.opengl_widget)

    You can use the following functions:
        - set_cube(self, cubes: np.array(Cube)): 
          Sets the positions, sizes, rotations, and colors of cubes to be rendered.
        
        - set_points(self, points: np.array(GroupPoints)):
          Sets the positions and colors of points to be rendered.
        
        - change_FPS(self, FPS: float):
          Changes the rendering frame rate.

Dependencies:
    - PySide6 for Qt Widgets and OpenGL support
    - OpenGL.GL and OpenGL.GLU for rendering

Classes:
    - Cube: Represents a cube with a position, size, rotation, and color.
    - GroupPoints: Represents a group of points with a color.

QOpenGL3DViewer:
    A Qt widget for 3D rendering using OpenGL. This viewer supports rendering 
    simple 3D objects such as cubes and points. The camera can be controlled 
    interactively using the mouse (rotation, translation, zoom).
"""

from PySide6.QtCore import QTimer, Qt
from PySide6.QtOpenGLWidgets import QOpenGLWidget
from PySide6.QtGui import QVector2D
from OpenGL.GL import (glClearColor, glShadeModel, glMatrixMode,
                       glLoadIdentity, glPushMatrix, glPopMatrix,
                       glEnable, glViewport, glTranslatef, glRotatef,
                       glClear, glBegin, glEnd, glColor3f, glVertexPointer,
                       glVertex3f, glPointSize, glDrawArrays,glEnableClientState, 
                       glDisableClientState, )
from OpenGL.GL import (GL_DEPTH_TEST, GL_SMOOTH, GL_PROJECTION, GL_MODELVIEW,
                       GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_LINES, GL_FLOAT,
                       GL_POINTS, GL_VERTEX_ARRAY)
from OpenGL.GLU import gluPerspective

from time import time
import numpy as np

from dataclasses import dataclass


@dataclass
class Cube:
    """
    Represents a 3D cube object.

    Attributes:
        position (np.array): The (x, y, z) coordinates of the cube's center.
        size (np.array): The dimensions (width, height) of the cube. 
                         Defaults to (1.0, 1.0).
        rotation (np.array): The rotation angles (in degrees) around the 
                             (x, y, z) axes. Defaults to (0.0, 0.0, 0.0).
        color (np.array): The RGB color of the cube, represented as a 
                          normalized float array. Defaults to (1.0, 1.0, 0.0).
    """

    position: np.array  # The position of the cube's center in 3D space.
    size: np.array = np.array((1.0, 1.0))  # The size of the cube (width, height).
    rotation: np.array = np.array((0.0, 0.0, 0.0))  # Rotation angles (x, y, z).
    color: np.array = np.array((1.0, 1.0, 0.0))  # Cube color (R, G, B).

@dataclass
class GroupPoints:
    """
    Represents a collection of points in 3D space.

    Attributes:
        points (np.array): A 2D array of shape (n, 3) where each row 
                           contains the (x, y, z) coordinates of a point.
        color (np.array): The RGB color of the points, represented as a 
                          normalized float array. Defaults to (0.0, 1.0, 1.0).
    """

    points: np.array  # Array of points in 3D space.
    color: np.array = np.array((0.0, 1.0, 1.0))  # Color of the points (R, G, B).
class QOpenGL3DViewer(QOpenGLWidget):
    """
    A Qt widget for 3D rendering using OpenGL. This viewer supports rendering 
    simple 3D objects such as cubes and points. The camera can be controlled 
    interactively using the mouse for rotation, translation, and zoom.

    Attributes:
        angle_x (float): Rotation angle around the X-axis.
        angle_y (float): Rotation angle around the Y-axis.
        translate_x (float): Translation along the X-axis.
        translate_y (float): Translation along the Y-axis.
        zoom (float): Zoom level along the Z-axis.
        last_mouse_position (QVector2D): Last recorded mouse position.
        mouse_pressed (bool): Indicates if the left mouse button is pressed.
        right_mouse_pressed (bool): Indicates if the right mouse button is pressed.
        cubes (np.array): Array of Cube objects to be rendered.
        points (np.array): Array of GroupPoints objects to be rendered.
        render_timer (QTimer): Timer for controlling the rendering frame rate.
    """

    def __init__(self, FPS=60, parent=None):
        """
        Initializes the QOpenGL3DViewer widget.

        Args:
            FPS (int): The desired frames per second for rendering. Defaults to 60.
            parent (QWidget, optional): The parent widget. Defaults to None.
        """
        super(QOpenGL3DViewer, self).__init__(parent)

        # Initialize camera and interaction parameters
        self.angle_x = 0  
        self.angle_y = 0  
        self.translate_x = 0  
        self.translate_y = 0  
        self.zoom = -8.0  
        self.last_mouse_position = QVector2D()  
        self.mouse_pressed = False  
        self.right_mouse_pressed = False  

        # Object positions
        self.cubes = np.zeros((0), dtype=Cube) 
        self.points = np.zeros((0), dtype=GroupPoints)  

        # Render timer for continuous updating
        self.render_timer = QTimer(self)
        self.render_timer.timeout.connect(self.update)
        self.change_FPS(FPS)

    def initializeGL(self):
        """Initializes the OpenGL context and sets background color and depth test."""
        glClearColor(0.1, 0.1, 0.1, 1.0)  # Background color
        glEnable(GL_DEPTH_TEST)  # Enable depth testing
        glShadeModel(GL_SMOOTH)  # Set the shading model to smooth

    def resizeGL(self, w, h):
        """
        Handles resizing of the OpenGL viewport.

        Args:
            w (int): Width of the viewport.
            h (int): Height of the viewport.
        """
        if h == 0:
            h = 1
        glViewport(0, 0, w, h)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(45, w / h, 0.1, 100.0)
        glMatrixMode(GL_MODELVIEW)

    def paintGL(self):
        """Renders the scene in the OpenGL context."""
        t = time()
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glLoadIdentity()

        # Apply camera transformations
        glTranslatef(self.translate_x, self.translate_y, self.zoom)
        glRotatef(self.angle_x, 1.0, 0.0, 0.0)  
        glRotatef(self.angle_y, 0.0, 1.0, 0.0)  

        # Draw the coordinate axes
        self.draw_axes()

        # Render cubes
        for cube in self.cubes:
            glPushMatrix()
            glTranslatef(*cube.position)
            glColor3f(*cube.color)
            glRotatef(cube.rotation[0], 1, 0, 0)  
            glRotatef(cube.rotation[1], 0, 1, 0)  
            glRotatef(cube.rotation[2], 0, 0, 1)  
            self.draw_cube(cube.size)  
            glPopMatrix()

        # Render points
        for groupPoint in self.points:
            glColor3f(*groupPoint.color)
            self.draw_points(points=groupPoint.points)

        print(f"\rDraw time: {1/(time()-t):.4f} FPS", end="")

    def draw_axes(self):
        """Draws the coordinate axes (X, Y, Z) in different colors."""
        glBegin(GL_LINES)

        # X axis (red)
        glColor3f(1.0, 0.0, 0.0)
        glVertex3f(0, 0, 0)
        glVertex3f(10, 0, 0)

        # Y axis (green)
        glColor3f(0.0, 1.0, 0.0)
        glVertex3f(0, 0, 0)
        glVertex3f(0, 10, 0)

        # Z axis (blue)
        glColor3f(0.0, 0.0, 1.0)
        glVertex3f(0, 0, 0)
        glVertex3f(0, 0, 10)

        glEnd()

    def draw_cube(self, size=np.array((1, 1))):
        """Draws a cube with colored edges.

        Args:
            size (np.array): The dimensions of the cube.
        """
        # Create cube vertices based on size
        half_size = size / 2.0
        vertices = np.array([
            [-half_size[0], -half_size[1], -half_size[2]], 
            [half_size[0], -half_size[1], -half_size[2]],
            [half_size[0],  half_size[1], -half_size[2]], 
            [-half_size[0],  half_size[1], -half_size[2]],
            [-half_size[0], -half_size[1],  half_size[2]], 
            [half_size[0], -half_size[1],  half_size[2]],
            [half_size[0],  half_size[1],  half_size[2]], 
            [-half_size[0],  half_size[1],  half_size[2]]
        ], dtype=np.float32)

        edges = [
            [0, 1], [1, 2], [2, 3], [3, 0],
            [4, 5], [5, 6], [6, 7], [7, 4],
            [0, 4], [1, 5], [2, 6], [3, 7]
        ]

        # Enable vertex array mode
        glEnableClientState(GL_VERTEX_ARRAY)

        # Define vertices for lines
        line_vertices = vertices[edges].reshape(-1, 3)

        # Set vertex pointer to the line vertices
        glVertexPointer(3, GL_FLOAT, 0, line_vertices)

        # Draw the edges in one call
        glDrawArrays(GL_LINES, 0, line_vertices.shape[0])

        # Disable vertex array mode
        glDisableClientState(GL_VERTEX_ARRAY)

    def draw_points(self, points, size=2.0):
        """
        Draws points in 3D space.

        Args:
            points (np.array): The 3D coordinates of points to render.
            size (float): The size of the points to render. Defaults to 2.0.
        """
        if points.size == 0:
            return

        # Set point size
        glPointSize(size)

        # Enable vertex array mode
        glEnableClientState(GL_VERTEX_ARRAY)

        # Define the vertex array (3 coordinates per point)
        glVertexPointer(3, GL_FLOAT, 0, points)

        # Draw all points in one call
        glDrawArrays(GL_POINTS, 0, len(points))

        # Disable vertex array mode
        glDisableClientState(GL_VERTEX_ARRAY)

    def set_cube(self, cubes: np.array(Cube)):
        """Updates the positions, sizes, rotations, and colors of the cubes.

        Args:
            cubes (np.array): An array of Cube objects to be rendered.
        """
        self.cubes = cubes

    def set_points(self, points: np.array(GroupPoints)):
        """Sets the positions and colors of points to be rendered.

        Args:
            points (np.array): An array of GroupPoints objects.
        """
        self.points = points

    def change_FPS(self, FPS: float):
        """Adjusts the rendering frame rate.

        Args:
            FPS (float): The desired frame rate in frames per second.
        """
        if self.render_timer.isActive():
            self.render_timer.stop()
        self.render_timer.start(1000 / FPS)

    ################### MOUSE EVENTS ###################

    def mousePressEvent(self, event):
        """Handles mouse press events for rotation and translation.

        Args:
            event (QMouseEvent): The mouse event triggered.
        """
        if event.button() == Qt.MouseButton.LeftButton:
            self.mouse_pressed = True
            self.last_mouse_position = QVector2D(event.position())
        elif event.button() == Qt.MouseButton.RightButton:
            self.right_mouse_pressed = True
            self.last_mouse_position = QVector2D(event.position())
        elif event.button() == Qt.MouseButton.MiddleButton:
            self.add_cube_at_center()

    def mouseMoveEvent(self, event):
        """Handles mouse move events for camera manipulation.

        Args:
            event (QMouseEvent): The mouse event triggered.
        """
        if self.mouse_pressed:
            current_position = QVector2D(event.position())
            delta = current_position - self.last_mouse_position
            self.angle_x += delta.y()
            self.angle_y += delta.x()
            self.last_mouse_position = current_position
        elif self.right_mouse_pressed:
            current_position = QVector2D(event.position())
            delta = current_position - self.last_mouse_position
            self.translate_x += delta.x() * 0.1  
            self.translate_y -= delta.y() * 0.1  
            self.last_mouse_position = current_position

    def mouseReleaseEvent(self, event):
        """Handles mouse release events to stop camera manipulation.

        Args:
            event (QMouseEvent): The mouse event triggered.
        """
        if event.button() == Qt.MouseButton.LeftButton:
            self.mouse_pressed = False
        elif event.button() == Qt.MouseButton.RightButton:
            self.right_mouse_pressed = False

    def wheelEvent(self, event):
        """Handles mouse wheel events for zooming in and out.

        Args:
            event (QWheelEvent): The wheel event triggered.
        """
        delta = event.angleDelta().y()
        self.zoom += delta * 0.001  # Adjust zoom sensitivity