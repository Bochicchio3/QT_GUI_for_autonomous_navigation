/*
 * Copyright (c) 2009, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "turtlesim/turtle.h"

#include <QColor>
#include <QRgb>
#include <cmath>


#define DEFAULT_PEN_R 0xb3
#define DEFAULT_PEN_G 0xb8
#define DEFAULT_PEN_B 0xff

namespace turtlesim
{

Turtle::Turtle(const ros::NodeHandle& nh, const QImage& turtle_image, const QPointF& pos, float orient)
: nh_(nh)
, turtle_image_(turtle_image)
, pos_(pos)
, orient_(orient)
, lin_vel_(0.0)
, ang_vel_(0.0)
, pen_on_(true)
, pen_(QColor(DEFAULT_PEN_R, DEFAULT_PEN_G, DEFAULT_PEN_B))
{
  pen_.setWidth(3);

  velocity_sub_ = nh_.subscribe("cmd_vel", 1, &Turtle::velocityCallback, this);
  pose_pub_ = nh_.advertise<Pose>("pose", 1);
  color_pub_ = nh_.advertise<Color>("color_sensor", 1);
  set_pen_srv_ = nh_.advertiseService("set_pen", &Turtle::setPenCallback, this);
  teleport_relative_srv_ = nh_.advertiseService("teleport_relative", &Turtle::teleportRelativeCallback, this);
  teleport_absolute_srv_ = nh_.advertiseService("teleport_absolute", &Turtle::teleportAbsoluteCallback, this);
  pose_sub=nh_.subscribe("tag_pose0", 1, &Turtle::poseCallback, this);
  waypoint_sub=nh_.subscribe("waypoint_publisher", 1, &Turtle::waypointCallback, this);
  orientation_sub=nh_.subscribe("orientation", 1, &Turtle::orientationCallback, this);

  meter_ =1;
  //rotateImage();
}

void Turtle::waypointCallback(const geometry_msgs::Point::ConstPtr& point2)
{
wayx=point2->x/10;
wayy=point2->y/10;



}

void Turtle::orientationCallback(const geometry_msgs::Point::ConstPtr& point2)
{

orientazione=-point2->z;
}

void Turtle::poseCallback(const geometry_msgs::Point::ConstPtr& point)
{
if (point->x ==0 &&  point->y==0){return;}
if (pow((point->x+point->y)/10,2)-pow((a+b)/10,2)<500){return;}
a=point->x/10;
b=point->y/10;
}


void Turtle::velocityCallback(const geometry_msgs::Twist::ConstPtr& vel)
{
  last_command_time_ = ros::WallTime::now();
  lin_vel_ = vel->linear.x;
  ang_vel_ = vel->angular.z;

}

bool Turtle::setPenCallback(turtlesim::SetPen::Request& req, turtlesim::SetPen::Response&)
{
  pen_on_ = !req.off;
  if (req.off)
  {
    return true;
  }

  QPen pen(QColor(req.r, req.g, req.b));
  if (req.width != 0)
  {
    pen.setWidth(req.width);
  }

  pen_ = pen;
  return true;
}

bool Turtle::teleportRelativeCallback(turtlesim::TeleportRelative::Request& req, turtlesim::TeleportRelative::Response&)
{
  teleport_requests_.push_back(TeleportRequest(0, 0, req.angular, req.linear, true));
  return true;
}

bool Turtle::teleportAbsoluteCallback(turtlesim::TeleportAbsolute::Request& req, turtlesim::TeleportAbsolute::Response&)
{
  teleport_requests_.push_back(TeleportRequest(req.x, req.y, req.theta, 0, false));
  return true;
}

void Turtle::rotateImage()
{
  QTransform transform;
  transform.rotate(orient_ * 180.0 / PI);
  turtle_rotated_image_ = turtle_image_.transformed(transform);
}

bool Turtle::update(double dt, QPainter& path_painter, const QImage& path_image, qreal canvas_width, qreal canvas_height)
{
  bool modified = false;
  qreal old_orient = orient_;

  // first process any teleportation requests, in order
  V_TeleportRequest::iterator it = teleport_requests_.begin();
  V_TeleportRequest::iterator end = teleport_requests_.end();
  for (; it != end; ++it)
  {
    const TeleportRequest& req = *it;

     QPointF old_pos = pos_;
    // if (req.relative)
    // {
    //
    //   orient_ += req.theta;
    //   pos_.rx() += std::cos(orient_) * req.linear;
    //   pos_.ry() += - std::sin(orient_) * req.linear;
    // }
    // else
    // {
    //
    //   pos_.setX(req.pos.x());
    //   pos_.setY(std::max(0.0, static_cast<double>(canvas_height - req.pos.y())));
    //   orient_ = req.theta;
    // }

    if (pen_on_)
    {
      path_painter.setPen(pen_);
    //  path_painter.drawLine(pos_ , old_pos );


    }
    modified = true;
  }

  teleport_requests_.clear();

  if (ros::WallTime::now() - last_command_time_ > ros::WallDuration(1.0))
  {
    lin_vel_ = 0.0;
    ang_vel_ = 0.0;
  }
 QPointF old_pos = pos_;
  pos_.rx() = a;
  pos_.ry() = b;
  //pos_.setX(std::min(std::max(static_cast<double>(pos_.x()), 0.0), static_cast<double>(canvas_width)));
  //pos_.setY(std::min(std::max(static_cast<double>(pos_.y()), 0.0), static_cast<double>(canvas_height)));

  // Publish pose of the turtle
  // Pose p;
  // p.x = pos_.x();
  // p.y = canvas_height - pos_.y();
  // p.theta = orient_;
  // p.linear_velocity = lin_vel_;
  // p.angular_velocity = ang_vel_;
  // pose_pub_.publish(p);

  // Figure out (and publish) the color underneath the turtle
  {
    Color color;
    QRgb pixel = path_image.pixel((pos_).toPoint());
    color.r = qRed(pixel);
    color.g = qGreen(pixel);
    color.b = qBlue(pixel);
    color_pub_.publish(color);
  }


    //rotateImage();
    modified = true;


    if (pen_on_)
    {

      path_painter.setPen(pen_);
      path_painter.drawLine(a+xrelativa,-b+yrelativa, a_vecio+xrelativa,-b_vecio+yrelativa );
      ROS_INFO("DISEGNOSTOaltroCAZZO");

    }
    modified = true;


  return modified;
}

void Turtle::paint(QPainter& painter)
{
  QPointF p(a,b);
  orient_ =orientazione -3*PI/2;
  // Keep orient_ between -pi and +pi
  orient_ -= 2*PI * std::floor((orient_ + PI)/(2*PI));
  a_vecio=pos_.rx();
  b_vecio=pos_.ry();
  pos_.rx() = a;
  pos_.ry() = b;
  painter.drawLine(a+xrelativa,-b+yrelativa, a_vecio+xrelativa,-b_vecio+yrelativa );
  rotateImage();

  // p.rx() -= 0.5 * turtle_rotated_image_.width();
  // p.ry() -= 0.5 * turtle_rotated_image_.height();

  painter.drawImage(a+xrelativa,-b+yrelativa, turtle_rotated_image_);
  painter.drawEllipse(int(wayx)+xrelativa,-int(wayy)+yrelativa, 100/2, 100/2);
  painter.setFont(QFont("Arial", 20/2));
  QRect rectangle1 = QRect(700/2, 1610/2, 400/2, 400/2);
  QString s = QString::number(wayx);
  QString s2 = QString::number(wayy);
  QString x = "Current waypoint: (";
  QString y = ",";
  QString s3 = ")";
  s=x.append(s.append(y.append(s2.append(s3))));
  QRect rectangle2 = QRect(100/2, 1610/2, 400/2, 400/2);
  QString as = QString::number(a);
  QString as2 = QString::number(b);
  QString ax = "Current position: (";
  QString ay = ",";
  QString as3 = ")";
  as=ax.append(as.append(ay.append(as2.append(as3))));
  // Qstring textfinale=  text.append(s.append(text3.append(s2.append(text2))));
  painter.drawText(rectangle1 ,s);
  painter.drawText(rectangle2 ,as);
  painter.drawLine(100/2,1600/2,1600/2,1600/2);
    painter.drawLine(1600/2,100/2,1600/2,1600/2);
      painter.drawLine(100/2,100/2,100/2,1600/2);
        painter.drawLine(1600/2,100/2,100/2,100/2);
  QRect rectangle3 = QRect(1300/2, 1610/2, 400/2, 400/2);
  QString as33 = "Width: 7m x 7m";
  painter.drawText(rectangle3 ,as33);

}

}
