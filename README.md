# Neural-Hand-Control
Code set to control Robotic hand made up of Dynamixel motors

*******This code has been updated on 06-02-2018*********
*******It is the initial set of working codes********

The Robot hand made of dynamixels in the Centre for Robotics Research can be easily controlled. I have made use of the simple commands present in DynamixelSDK repository of ROBOTIS. The code is designed to use opencv-python to read an image. It captures a click made on the image. The coordinates of the click are sent to the neural network which has been trained to map the image coordinates to the required joint angles of the robot. The training has been done for 15 data points randomly chosen on a white board. The dataset containing the x and y coordinate inputs has been input to the neural network and the goal position of the 4 rx-64 dynamixel motors is the output. Once opencv-python and DynamixelSDK have been installed and built respectively, running *sh autohand.sh* in the terminal should suffice to run the code.
