#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>


using namespace std;

struct termios cooked;

class Reader {

public:
	Reader();
	void enableRawMode();
	void disableRawMode();
};
void Reader::disableRawMode(){
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &cooked);
}
void Reader::enableRawMode() {
	tcgetattr(STDIN_FILENO, &cooked);
	struct termios raw = cooked;
	raw.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

Reader::Reader(){
	enableRawMode();
}



Reader input;

class KeyboardPublisher {

public:
	KeyboardPublisher();
	void keyLoop();
private:
	ros::NodeHandle nh;
  	double linear, angular;
  	ros::Publisher twist_pub;

};

KeyboardPublisher::KeyboardPublisher(): linear(0), angular(0) {
	twist_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);
}

void KeyboardPublisher::keyLoop() {
	char c;
	bool changed = false;

	cout << "Reading from keyboard" << endl;
	cout << "Use arrow keys to control robot" << endl;
	cout << "Press 'q' to quit" << endl;

	while(read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
		if(c == '\x1b'){
			char arr[2];
			read(STDIN_FILENO, &arr[0], 1);
			read(STDIN_FILENO, &arr[1], 1);
			c = arr[1];
		}

		angular=linear=0;

		switch(c) {
			case 'D':
				ROS_DEBUG("left");
				angular = 1.0;
				changed = true;
				break;
			case 'C':
				ROS_DEBUG("right");
				angular = -1.0;
				changed = true;
				break;
			case 'A':
				ROS_DEBUG("up");
				linear = 1.0;
				changed = true;
				break;
			case 'B':
				ROS_DEBUG("down");
				linear = -1.0;
				changed = true;
				break;	
		}
		geometry_msgs::Twist twist;
		twist.angular.z = angular;
		twist.linear.x = linear;
		if(changed){
			twist_pub.publish(twist);
			changed = false;
		}
	}
	return;
}



void quit(int sig) {
  	(void)sig;
  	input.disableRawMode();
 	ros::shutdown();
  	exit(0);
}

int main(int argc, char** argv) {
  	ros::init(argc, argv, "keyboard_publisher");
  	KeyboardPublisher keyboard_publisher;

  	signal(SIGINT,quit);

  	keyboard_publisher.keyLoop();
  	quit(0);
  
 	return(0);
}



