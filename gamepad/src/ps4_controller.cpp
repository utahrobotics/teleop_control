#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <sensor_msgs/Joy.h>
#include <std_msgs/Empty.h>
#include <std_msgs/Float32.h>

/**
 --Ver 1.1
 <Change Notes from 1.0>:
    - Changed right_horiz_axis(4) to right_horiz_axis(3) so that the 
      variable maps to the correct axis.
 */

class PS4Controller{
public:
  PS4Controller();

private:
  void joyCallback(const sensor_msgs::Joy::ConstPtr& joy);

  ros::NodeHandle nh_;

  int left_vert_axis_, right_horiz_axis_, left_trigger_, right_trigger_;
  int start_button_;
  double linear_scale_, angular_scale_;
  
  
  ros::Publisher vel_pub_; // velocity publisher node
  ros::Publisher autonomy_cmd_pub_; // autonomy publisher node
  ros::Publisher linear_rc_; //digger linear acuator velocity
  ros::Publisher digger_rc_; //digger drum speed
  ros::Subscriber joy_sub_; // joy subscriber node
};


PS4Controller::PS4Controller(): // This is an initialization list of the indexes into the axes and buttons arrays
  linear_scale_(1),
  angular_scale_(1),
  left_vert_axis_(1),
  right_horiz_axis_(2),
  right_trigger_(4),
  left_trigger_(3),
  start_button_(12)
{

  // get parameters from the parameter server
  // try to get a parameter named arg1, save it in arg2, if that fails use value from arg3
  nh_.param("scale_angular", angular_scale_, angular_scale_);
  nh_.param("scale_linear", linear_scale_, linear_scale_);
  nh_.param("left_vert_axis", left_vert_axis_, left_vert_axis_);
  nh_.param("right_horiz_axis", right_horiz_axis_, right_horiz_axis_);
  nh_.param("start_button", start_button_, start_button_);
  nh_.param("left_trigger", left_trigger_, left_trigger_);
  nh_.param("right_trigger", right_trigger_, right_trigger_);


  // advertise this node to ros
  vel_pub_ = nh_.advertise<geometry_msgs::Twist>("/cmd_vel", 1);
  autonomy_cmd_pub_ = nh_.advertise<std_msgs::Empty>("/click_select_button", 10);
  linear_rc_ = nh_.advertise<std_msgs::Float32>("/linear_rc",5);
  digger_rc_ = nh_.advertise<std_msgs::Float32>("/digger_rc",5);

  // subscribe to the incoming joystick input
  // argument description: (name of topic, number of messages to queue, callback pointer, what object to call that callback on)
  joy_sub_ = nh_.subscribe<sensor_msgs::Joy>("joy", 10, &PS4Controller::joyCallback, this);

}

// Callback method called when this node gets a joy messge
void PS4Controller::joyCallback(const sensor_msgs::Joy::ConstPtr& joy){
  geometry_msgs::Twist twist;
  
  std_msgs::Empty empty;

  std_msgs::Float32 linear_rc_msg;

  std_msgs::Float32 digger_rc_msg;

  // The linear-x component of the twist is the product of the linear scale factor and the left vertical axis input on joy
  twist.linear.x = linear_scale_ * joy->axes[left_vert_axis_]; // has a maximum value of 1

  // The angular-z component of the twist is the product of the angular scale factor and the right horizontal axis input on joy
  twist.angular.z = angular_scale_ * joy->axes[right_horiz_axis_]; // has a maximum value of 1


  // The velocity of the linear acuator that controls the digger arm: value between 1 and -1
  linear_rc_msg.data = -1.0f * joy->axes[right_trigger_];

  // The velocity of the digger drum: value between 1 and -1
  digger_rc_msg.data = -1.0f * joy->axes[left_trigger_];

  if(joy->buttons[start_button_]){
    autonomy_cmd_pub_.publish(empty);
  }
  
  vel_pub_.publish(twist);
  digger_rc_.publish(digger_rc_msg);
  linear_rc_.publish(linear_rc_msg);
}

int main(int argc, char** argv){
  ros::init(argc, argv, "ps4_controller");
  PS4Controller ps4_controller;

  //wait for and incoming joy message to interpret
  ros::spin();
}
