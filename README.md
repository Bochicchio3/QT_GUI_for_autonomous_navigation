# Rover Graphical Interface

The purpose of this package is to provide a useful graphical representation of the rover, by representing current position and orientation and waypoint position.
With this minimalistic GUI you can understand how precise the localization is and draw the path of the Rover during the waypoint navigation.

Most of the code derives from the original turtlesim package, which had all the needed elements required for this project.
The original turtlesim code can be found here: http://wiki.ros.org/turtlesim

## 1. QT for ROS

For this project, we used QT_GUI C++ library.
For reference: https://doc.qt.io/qt-5/qtgui-index.html

We chose this library for its native compatibility with ROS and for its easeness of use.


## 2. Structure of the code

As previously stated, only minor modification have been made to the original turtlesim code.
The structure is the following:

Include files:
>. turtle.h
>. turtle_frame.h
C++ files:
>. turtle.cpp
>. turtle_frame.cpp
>. turtlesim.cpp

## turtle.h

This file declares all the required variables for the cpp code. 
Here you can define:
1. Subscribers and Publisher
```
  ros::Subscriber velocity_sub_;
  ros::Subscriber orientation_sub;
  ros::Publisher pose_pub_;
  ros::Subscriber waypoint_sub;
  ros::Subscriber pose_sub;
 ```
2. Callbacks
 ```
  void orientationCallback(const geometry_msgs::Point::ConstPtr& point2);
  void velocityCallback(const geometry_msgs::Twist::ConstPtr& vel);
  void waypointCallback(const geometry_msgs::Point::ConstPtr& point);
  void poseCallback(const geometry_msgs::Point::ConstPtr& point);
 ```

3. Global Variables

```
  double wayx;
  double wayy;
  double orientazione=0;
  int xrelativa=100/2;
  int yrelativa=1500/2;
 ```
 
 
 ## turtlesim.cpp
 
 This is where ros is initialized and the main is defined. Do not modify.
 
 ## turtle_frame.cpp
 
 ## turtle.cpp
 
 In turtle.cpp you can find all the code relative to the graphical interface, and the communication 
 of relevant data with the ros network
 
 1. Publisher and subscriber instantion (ONLY ADDED TOPICS)
 ```
  velocity_sub_ = nh_.subscribe("cmd_vel", 1, &Turtle::velocityCallback, this);
  pose_pub_ = nh_.advertise<Pose>("pose", 1);
  pose_sub=nh_.subscribe("tag_pose0", 1, &Turtle::poseCallback, this);
  waypoint_sub=nh_.subscribe("waypoint_publisher", 1, &Turtle::waypointCallback, this);
  orientation_sub=nh_.subscribe("orientation", 1, &Turtle::orientationCallback, this);
```
2. Callbacks
```
void Turtle::waypointCallback(const geometry_msgs::Point::ConstPtr& point2)
void Turtle::orientationCallback(const geometry_msgs::Point::ConstPtr& point2)
void Turtle::poseCallback(const geometry_msgs::Point::ConstPtr& point)
```
3. Draw line
This draw a line with relative coordinates, as I need the interface to start the car in the down left position.
More information about this can be found in POSZYX section, but essentially the interface right now is thought for a localization system which has the origin in the down-left, the X to down-right and the Y to up-left

```
path_painter.drawLine(a+xrelativa,-b+yrelativa, old_a+xrelativa,-old_b+yrelativa );
```
Where relative variables are defined in turtle.h file. Modify those if you want to change the starting point.

4. Orientation
Set initial angular offset with the variable orient_
In the code the initial offset is -3 PI /2
```
void Turtle::paint(QPainter& painter)
{
  QPointF p(a,b);
  orient_ =orientazione -3*PI/2;
  // Keep orient_ between -pi and +pi
  orient_ -= 2*PI * std::floor((orient_ + PI)/(2*PI));
 ...
 ...
 
 ``` 
 5. Rover Image
 ```
   painter.drawImage(a+xrelativa,-b+yrelativa, turtle_rotated_image_);
```

6. Waypoint
```
  painter.drawEllipse(int(wayx)+xrelativa,-int(wayy)+yrelativa, 100/2, 100/2);
```
 
 
 
 7. Static Interface and Text
 ```
 //DRAW THE INTERFACE INFORMATION: CURRENT WAYPOINT AND CURRENT POSITION

  painter.setFont(QFont("Arial", 20/2));
  QRect rectangle1 = QRect(700/2, 1610/2, 400/2, 400/2);
  QString s = QString::number(wayx);
  QString s2 = QString::number(wayy);
  QString x = "Current waypoint: (";
  QString y = ",";
  QString s3 = ")";

// THE TEXT IS DONE BY APPENDING ALL THE STRINGS

  s=x.append(s.append(y.append(s2.append(s3))));
  QRect rectangle2 = QRect(100/2, 1610/2, 400/2, 400/2);
  QString as = QString::number(a);
  QString as2 = QString::number(b);
  QString ax = "Current position: (";
  QString ay = ",";
  QString as3 = ")";
  as=ax.append(as.append(ay.append(as2.append(as3))));
 
 // HERE THE BOUNDING LINES ARE REALIZED

  painter.drawText(rectangle1 ,s);
  painter.drawText(rectangle2 ,as);
  painter.drawLine(100/2,1600/2,1600/2,1600/2);
  painter.drawLine(1600/2,100/2,1600/2,1600/2);
  painter.drawLine(100/2,100/2,100/2,1600/2);
  painter.drawLine(1600/2,100/2,100/2,100/2);
  QRect rectangle3 = QRect(1300/2, 1610/2, 400/2, 400/2);
  QString as33 = "Width: 7m x 7m";
  painter.drawText(rectangle3 ,as33);
 
```


 
 
 
