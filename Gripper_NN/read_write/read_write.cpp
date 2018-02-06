/*******************************************************************************
* Copyright 2017 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* Author: Ryu Woon Jung (Leon) */

//
// *********     Read and Write Example      *********
//
//
// Available DXL model on this example : All models using Protocol 1.0
// This example is tested with a DXL MX-28, and an USB2DYNAMIXEL
// Be sure that DXL MX properties are already set as %% ID : 1 / Baudnum : 34 (Baudrate : 57600)
//

#if defined(__linux__) || defined(__APPLE__)
#include <fcntl.h>
#include <termios.h>
#define STDIN_FILENO 0
#elif defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#endif

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "dynamixel_sdk.h"                                  // Uses Dynamixel SDK library

// Control table address
#define ADDR_MX_TORQUE_ENABLE           24                  // Control table address is different in Dynamixel model
#define ADDR_MX_GOAL_POSITION           30
#define ADDR_MX_PRESENT_POSITION        36
#define ADDR_MX_MOVING_SPEED            32

// Protocol version
#define PROTOCOL_VERSION                1.0                 // See which protocol version is used in the Dynamixel

// Default setting
#define DXL_ID_1                          1                   // Dynamixel ID: 1
#define DXL_ID_2                          2                   // Dynamixel ID: 2
#define DXL_ID_3                          3                   // Dynamixel ID: 2
#define DXL_ID_4                          4                   // Dynamixel ID: 2
#define BAUDRATE                        57142
#define DEVICENAME                      "/dev/ttyUSB0"      // Check which port is being used on your controller
                                                            // ex) Windows: "COM1"   Linux: "/dev/ttyUSB0" Mac: "/dev/tty.usbserial-*"

#define TORQUE_ENABLE                   1                   // Value for enabling the torque
#define TORQUE_DISABLE                  0                   // Value for disabling the torque
#define DXL_MINIMUM_POSITION_VALUE_2      250                // Dynamixel will rotate between this value
#define DXL_MAXIMUM_POSITION_VALUE_2      700                // and this value (note that the Dynamixel would not move when the position value is out of movable range. Check e-manual about the range of the Dynamixel you use.)
#define DXL_MINIMUM_POSITION_VALUE_1      498                // Dynamixel will rotate between this value
#define DXL_MAXIMUM_POSITION_VALUE_1      502
#define DXL_MOVING_STATUS_THRESHOLD     10                  // Dynamixel moving status threshold

#define ESC_ASCII_VALUE                 0x1b
#define NUMPAT 15
#define NUMIN  2
#define NUMHID 10
#define NUMOUT 4

int getch()
{
#if defined(__linux__) || defined(__APPLE__)
  struct termios oldt, newt;
  int ch;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  return ch;
#elif defined(_WIN32) || defined(_WIN64)
  return _getch();
#endif
}

int kbhit(void)
{
#if defined(__linux__) || defined(__APPLE__)
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
#elif defined(_WIN32) || defined(_WIN64)
  return _kbhit();
#endif
}

int main()
{
    	int    i, j, k, p, np, op, epoch;/*NUMIN=0,NUMHID=0,NUMOUT=0;*/
    		    int    NumInput = NUMIN, NumHidden = NUMHID, NumOutput = NUMOUT;
	double Input[NUMIN+1];
	char floatstr[8];
	double SumH[NUMHID+1], WeightIH[NUMIN+1][NUMHID+1],Hidden[NUMHID+1];
	double SumO[NUMOUT+1], WeightHO[NUMHID+1][NUMOUT+1], Output[NUMOUT+1];
    FILE *wfile1=fopen("WeightIH.csv","r");
    FILE *wfile2=fopen("WeightHO.csv","r");
    FILE *cfile=fopen("InputCoord.csv","r");
    //Read WeightIH
	for(j=0;j<=NUMHID;j++)
	{
		for(i=0;i<=NUMIN;i++)
		{
			fscanf(wfile1,"%[^,],",floatstr);
			WeightIH[i][j]=atof(floatstr);
		}
	}
    
	//Read WeightHO
	for(k=0;k<=NUMOUT;k++)
	{
		for(j=0;j<=NUMHID;j++)
		{
			fscanf(wfile2,"%[^,],",floatstr);
			WeightHO[j][k]=atof(floatstr);
		}
	}
//Read Input
	for(j=1;j<=NUMIN;j++)
	{
		fscanf(cfile,"%[^,],",floatstr);
        if(j==1)
		Input[j]=atof(floatstr)/650;
        else
        Input[j]=atof(floatstr)/500;
	}
	/* compute hidden unit activations */
	for( j = 1 ; j <= NumHidden ; j++ ) 
	{    
		SumH[j] =WeightIH[0][j] ;
                for( i = 1 ; i <= NumInput ; i++ ) 
		{
			SumH[j] += Input[i] * WeightIH[i][j] ;
                }
                Hidden[j] = 1.0/(1.0 + exp(-SumH[j])) ;
	}

	/* compute output unit activations and errors */
	for( k = 1 ; k <= NumOutput ; k++ ) 
	{    
		SumO[k] = WeightHO[0][k] ;
                for( j = 1 ; j <= NumHidden ; j++ ) 
		{
			SumO[k] += Hidden[j] * WeightHO[j][k] ;
                }
                Output[k] = 1.0/(1.0 + exp(-SumO[k])) ;
	}
int dxl_goal_position1=Output[1]*1000,dxl_goal_position2=Output[2]*1000,dxl_goal_position3=Output[3]*1000,dxl_goal_position4=Output[4]*1000;
if (dxl_goal_position2<280)
    dxl_goal_position2=280;
printf("%03d,%03d,%03d,%03d\n\n",dxl_goal_position1,dxl_goal_position2,dxl_goal_position3,dxl_goal_position4);
  // Initialize PortHandler instance
  // Set the port path
  // Get methods and members of PortHandlerLinux or PortHandlerWindows
  dynamixel::PortHandler *portHandler = dynamixel::PortHandler::getPortHandler(DEVICENAME);
  // Initialize PacketHandler instance
  // Set the protocol version
  // Get methods and members of Protocol1PacketHandler or Protocol2PacketHandler
  dynamixel::PacketHandler *packetHandler = dynamixel::PacketHandler::getPacketHandler(PROTOCOL_VERSION);
  uint16_t dxl_present_position1 = 0, dxl_present_position2=0, dxl_present_position3=0, dxl_present_position4=0;              // Present position
int dxl_comm_result = COMM_TX_FAIL;
  uint8_t dxl_error = 0;         
  // Open port
  if (portHandler->openPort())
  {
    printf("Succeeded to open the port!\n");
  }
  else
  {
    printf("Failed to open the port!\n");
    printf("Press any key to terminate...\n");
    getch();
    return 0;
  }
  // Set port baudrate
  if (portHandler->setBaudRate(BAUDRATE))
  {
    printf("Succeeded to change the baudrate!\n");
  }
  else
  {
    printf("Failed to change the baudrate!\n");
    printf("Press any key to terminate...\n");
    getch();
    return 0;
  }// Enable Dynamixel Torque
  
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID_1, ADDR_MX_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);
    dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID_2, ADDR_MX_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);
dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID_3, ADDR_MX_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);
dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID_4, ADDR_MX_TORQUE_ENABLE, TORQUE_DISABLE, &dxl_error);
dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_1, ADDR_MX_MOVING_SPEED, 200, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_2, ADDR_MX_MOVING_SPEED, 200, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_3, ADDR_MX_MOVING_SPEED, 200, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_4, ADDR_MX_MOVING_SPEED, 800, &dxl_error);
  dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID_1, ADDR_MX_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);
    dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID_2, ADDR_MX_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);
dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID_3, ADDR_MX_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);
dxl_comm_result = packetHandler->write1ByteTxRx(portHandler, DXL_ID_4, ADDR_MX_TORQUE_ENABLE, TORQUE_ENABLE, &dxl_error);
printf("Torque Enabled\n\n");
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_1, ADDR_MX_GOAL_POSITION, 500, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_2, ADDR_MX_GOAL_POSITION, 500, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_3, ADDR_MX_GOAL_POSITION, 500, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_4, ADDR_MX_GOAL_POSITION, 500, &dxl_error);
    printf("Goal Written\n\n");
    do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_1, ADDR_MX_PRESENT_POSITION, &dxl_present_position1, &dxl_error);
    }while((abs(500 - dxl_present_position1) > DXL_MOVING_STATUS_THRESHOLD));
    printf("1 Reached\n\n");
    do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_2, ADDR_MX_PRESENT_POSITION, &dxl_present_position2, &dxl_error);
    }while((abs(500 - dxl_present_position2) > DXL_MOVING_STATUS_THRESHOLD));
    printf("2 Reached\n\n");
        do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_3, ADDR_MX_PRESENT_POSITION, &dxl_present_position3, &dxl_error);
    }while((abs(500 - dxl_present_position3) > DXL_MOVING_STATUS_THRESHOLD));
    printf("3 Reached\n\n");
        do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_4, ADDR_MX_PRESENT_POSITION, &dxl_present_position4, &dxl_error);
    }while((abs(500 - dxl_present_position4) > DXL_MOVING_STATUS_THRESHOLD));
    printf("4 Reached\n\n");
    
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_1, ADDR_MX_MOVING_SPEED, 70, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_2, ADDR_MX_MOVING_SPEED, 70, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_3, ADDR_MX_MOVING_SPEED, 70, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_4, ADDR_MX_MOVING_SPEED, 800, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_1, ADDR_MX_GOAL_POSITION, dxl_goal_position1, &dxl_error);

    do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_1, ADDR_MX_PRESENT_POSITION, &dxl_present_position1, &dxl_error);
    }while((abs(dxl_goal_position1 - dxl_present_position1) > DXL_MOVING_STATUS_THRESHOLD));
    printf("1 Reached\n\n");    
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_4, ADDR_MX_GOAL_POSITION, dxl_goal_position4, &dxl_error);
    
        printf("Goal Written\n\n");
        do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_4, ADDR_MX_PRESENT_POSITION, &dxl_present_position4, &dxl_error);
    }while((abs(dxl_goal_position4 - dxl_present_position4) > DXL_MOVING_STATUS_THRESHOLD));
    printf("4 Reached\n\n");

    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_3, ADDR_MX_GOAL_POSITION, dxl_goal_position3, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_2, ADDR_MX_GOAL_POSITION, dxl_goal_position2, &dxl_error);
            do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_3, ADDR_MX_PRESENT_POSITION, &dxl_present_position3, &dxl_error);
    }while((abs(dxl_goal_position3 - dxl_present_position3) > DXL_MOVING_STATUS_THRESHOLD));
    printf("3 Reached\n\n");

    do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_2, ADDR_MX_PRESENT_POSITION, &dxl_present_position2, &dxl_error);
    }while((abs(dxl_goal_position2 - dxl_present_position2) > DXL_MOVING_STATUS_THRESHOLD));
    printf("2 Reached\n\n");
    sleep(2);
    /*dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_1, ADDR_MX_MOVING_SPEED, 200, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_2, ADDR_MX_MOVING_SPEED, 200, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_3, ADDR_MX_MOVING_SPEED, 200, &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_4, ADDR_MX_MOVING_SPEED, 200, &dxl_error);*/

    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_2, ADDR_MX_GOAL_POSITION, 500, &dxl_error);
        dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_3, ADDR_MX_GOAL_POSITION, 500, &dxl_error);
    printf("Goal Written\n\n");
    do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_2, ADDR_MX_PRESENT_POSITION, &dxl_present_position2, &dxl_error);
    }while((abs(500 - dxl_present_position2) > DXL_MOVING_STATUS_THRESHOLD));
    printf("2 Reached\n\n");
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_4, ADDR_MX_GOAL_POSITION, 500, &dxl_error);
        do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_3, ADDR_MX_PRESENT_POSITION, &dxl_present_position3, &dxl_error);
    }while((abs(500 - dxl_present_position3) > DXL_MOVING_STATUS_THRESHOLD));
    printf("3 Reached\n\n");
        do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_4, ADDR_MX_PRESENT_POSITION, &dxl_present_position4, &dxl_error);
    }while((abs(500 - dxl_present_position4) > DXL_MOVING_STATUS_THRESHOLD));
    printf("4 Reached\n\n");
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID_1, ADDR_MX_GOAL_POSITION, 500, &dxl_error);
    do
    {
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID_1, ADDR_MX_PRESENT_POSITION, &dxl_present_position1, &dxl_error);
    }while((abs(500 - dxl_present_position1) > DXL_MOVING_STATUS_THRESHOLD));
    printf("1 Reached\n\n");
    

  portHandler->closePort();
fclose(wfile1);
fclose(wfile2);
fclose(cfile);
  return 0;
}
