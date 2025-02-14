//
// Created by pbustos on 27/10/22.
//

#include "abstract_graphic_viewer.h"
AbstractGraphicViewer::AbstractGraphicViewer(QWidget *parent, QRectF dim_, bool draw_axis)
{
    QVBoxLayout *vlayout = new QVBoxLayout(parent);
    vlayout->addWidget(this);
    scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    scene.setSceneRect(dim_);
    this->setScene(&scene);
    this->setCacheMode(QGraphicsView::CacheBackground);
    this->setViewport(new QOpenGLWidget());
    this->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    this->setRenderHint(QPainter::Antialiasing);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    //this->setMinimumSize(400, 400);
    this->scale(1, -1);
    this->setMouseTracking(true);
    this->fitInView(scene.sceneRect(), Qt::KeepAspectRatio);
    this->viewport()->setMouseTracking(true);

    // axis
    if(draw_axis)
    {
        auto r = sceneRect();
        QLineF x_axis(r.center(), r.center() + QPointF(300, 0));
        QLineF y_axis(r.center(), r.center() + QPointF(0, 300));
        scene.addLine(x_axis, QPen(QColor("Red"), 30));
        scene.addLine(y_axis, QPen(QColor("Green"), 30));
    }
    this->adjustSize();
}
std::tuple<QGraphicsPolygonItem*, QGraphicsEllipseItem*> AbstractGraphicViewer::add_robot(float robot_width, float robot_length,
                                                                   float laser_x_offset, float laser_y_offset, QColor color)
{
    float sl = robot_length / 2.f;
    float sw = robot_width / 2.f;
    QPolygonF poly2;
    poly2 << QPoint(-sw, -sl) << QPoint(-sw, sl) << QPoint(sw, sl) << QPoint(sw, -sl);
    QBrush brush(color, Qt::SolidPattern);
    robot_polygon = scene.addPolygon(poly2, QPen(color), brush);
    laser_in_robot_sr = new QGraphicsEllipseItem(-30, -30, 60, 60, robot_polygon);
    laser_in_robot_sr->setBrush(QBrush(QColor("White")));
    //scene.addItem(laser_in_robot_sr);
    laser_in_robot_sr->setPos(laser_x_offset, laser_y_offset);
    robot_polygon->setZValue(55);
    robot_polygon->setPos(0, 0);
    return std::make_tuple(robot_polygon, laser_in_robot_sr);
}
void AbstractGraphicViewer::draw_contour()
{
    auto r = sceneRect();
    auto sr = scene.addRect(r, QPen(QColor("Gray"), 100));
    sr->setZValue(15);
}
QGraphicsPolygonItem* AbstractGraphicViewer::robot_poly()
{
    return robot_polygon;
}
QGraphicsEllipseItem*  AbstractGraphicViewer::laser_in_robot()
{
    return laser_in_robot_sr;
}
void AbstractGraphicViewer::wheelEvent(QWheelEvent *event)
{
    qreal factor;
    if (event->angleDelta().y() > 0)
        factor = 1.1;
    else
        factor = 0.9;
    auto view_pos = event->position();
    auto scene_pos = this->mapToScene(view_pos.toPoint());
    this->centerOn(scene_pos);
    this->scale(factor, factor);
    auto delta = this->mapToScene(view_pos.toPoint()) - this->mapToScene(this->viewport()->rect().center());
    this->centerOn(scene_pos - delta);
}
void AbstractGraphicViewer::resizeEvent(QResizeEvent *e)
{
    QGraphicsView::resizeEvent(e);
}
void AbstractGraphicViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (_pan)
    {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - (event->position().x() - _panStartX));
        verticalScrollBar()->setValue(verticalScrollBar()->value() - (event->position().y() - _panStartY));
        _panStartX = event->position().x();
        _panStartY = event->position().y();
        event->accept();

    }
    QGraphicsView::mouseMoveEvent(event);
}
void AbstractGraphicViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        _pan = true;
        _panStartX = event->position().x();
        _panStartY = event->position().y();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        auto cursor_in_scene = this->mapToScene(QPoint(event->position().x(), event->position().y()));
        emit new_mouse_coordinates(cursor_in_scene);
        return;
    }
    if (event->button() == Qt::RightButton)
    {
        event->accept();
        auto cursor_in_scene = this->mapToScene(QPoint(event->position().x(), event->position().y()));
        emit right_click(cursor_in_scene);
        return;
    }
    QGraphicsView::mousePressEvent(event);
}
void AbstractGraphicViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        _pan = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    }
    QGraphicsView::mouseReleaseEvent(event);
}