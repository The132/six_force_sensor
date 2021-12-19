#include "ros/ros.h"
#include "std_msgs/Float32MultiArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <iostream>
#include <string.h>

#define    MYPORT     4008  
#define    BUF_SIZE   31

int result = 0;
using namespace std;

inline float byte2float( unsigned char *uc){
  float *f=(float *)uc;
  return *f;
}
inline int bg(int s){
  unsigned char command[]={'A','T','+','G','S','D',0X0D,0X0A};
  return send(s,command,sizeof(command),0);

}
inline int stop(int s){
  unsigned char command[]={'A','T','+','G','S','D','=','S','T','O','P',0X0D,0X0A};
  return send(s,command,sizeof(command),0);
}
inline int setRate(int s){
  unsigned char command[]={'A','T','+','S','M','P','R','=','1','0','0',0X0D,0X0A};
  return send(s,command,sizeof(command),0);
}

inline  bool checkSum(unsigned char *recvbuf){
  unsigned char mSum=0;
  for(int i=6;i<=29;++i) mSum+=recvbuf[i];
  if(mSum!=recvbuf[BUF_SIZE-1]){
    std::cout<<"mSum is: "<<mSum<<std::endl<<"but recvbuf[30] is: "<<(int)recvbuf[BUF_SIZE-1]<<std::endl;
    for(int i=6;i<=29;++i){
      std::cout<<(int)recvbuf[i]<<" ";
    }   
  }
  return mSum==recvbuf[BUF_SIZE-1];
}

void my_exit(int i){
  stop(i);
  close(i);
  std::cout<<"exit main ....."<<std::endl;
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "force_sensor_publish_node");
  ros::NodeHandle n;
  ros::Publisher chatter_pub = n.advertise<std_msgs::Float32MultiArray>("sixforce_sensor", 10);
  ros::Rate loop_rate(100);

  unsigned char recvbuf[BUF_SIZE-1];
  int socket_cli = socket(AF_INET, SOCK_STREAM, 0);
  if(socket_cli < 0)
  {
    ROS_INFO("socket() error");
    return -1;
  }
  
  sockaddr_in sev_addr;  
	memset(&sev_addr, 0, sizeof(sev_addr));
	sev_addr.sin_family      = AF_INET;
	sev_addr.sin_port        = htons(MYPORT);
	sev_addr.sin_addr.s_addr = inet_addr("172.16.0.108");
  ROS_INFO("connecting...");

	int connect_status=connect(socket_cli, (struct sockaddr*) &sev_addr, sizeof(sev_addr));
  if( connect_status< 0)
	{ 
    ROS_INFO("connect error ,%d",errno);
		return -1;
	}
	else
		ROS_INFO("connect successfully");

  stop(socket_cli);
	bg(socket_cli);
  vector<vector<float>> vdata;
  vector<float>zero_force{0.0,0.0,0.0,0.0,0.0,0.0};
  while (ros::ok())
  {
    std_msgs::Float32MultiArray msg;
    memset(recvbuf,0,sizeof(recvbuf));
    int flag=recv(socket_cli, (char*)recvbuf, BUF_SIZE, 0);

    if(flag != BUF_SIZE){
      ROS_INFO("flag != BUF_SIZE exit......%d",flag);
      my_exit(socket_cli);
      return -1;

    }
    if(!checkSum(recvbuf)){
      ROS_INFO("checksum error: flag != sizeof(recvbuf) continue next......");
      continue;
    }


    if(vdata.size()<100){

      vector<float> temp;
      for(int i=0;i<6;++i){
        temp.push_back( byte2float(&(recvbuf[6+i*4])) );
      }
      vdata.push_back(temp);

      if(vdata.size() == 100){
        for(auto v:vdata){
          for(int i=0;i<6;++i){
            zero_force[i]+=v[i];
          }
        }
        for(int i=0;i<6;++i){
          zero_force[i]/=100.0f;
        }
      ROS_INFO("100 times average force ......%f ,%f ,%f ,%f ,%f ,%f ",zero_force[0],zero_force[1],zero_force[2],zero_force[3],zero_force[4],zero_force[5]);
      }

      continue;
    }

    for(int i=0;i<6;++i){
      msg.data.push_back( (byte2float(&(recvbuf[6+i*4])))-zero_force[i] );
    }
    chatter_pub.publish(msg);
    ros::spinOnce();

  }

  return 0;
}
