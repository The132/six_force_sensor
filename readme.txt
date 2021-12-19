<<<<<<<<<<<<<<============2021/10/22 23:44 tianhai 台式机完成测试===================>>>>>>>>>>>>>>>>
环境：ubuntu18.04 ros:melodic
使用方法：
直接catkin_make编译，之后记得source
1、设置静态ip:192.168.2.12 掩码：255.255.255.0 
2、完成硬件连接（网线、电源、、、、）
3、启动master节点： roscore 
4、启动本节点 rosrun forcesensor publish
5、可视化工具：rqt_plot /sixforce_sensor/data[0]:data[1]:data[2]:data[3]:data[4]:data[5]
	     分别是Fx,Fy,Fz,Mx,My,Mz



发布话题：/sixforce_sensor 消息类型 std_msgs/Float32MultiArray
目前的频率：10hz 最大频率可达2k 

待实现：
1、其他ros控制接口：如修改频率,设置零点，开始启动（当前自动启动），暂停数据，单次获取数据等，这些可以通过windows上位机实现修改。
2、包头校验，和校验等（tcp封装了，应该也可以不用做）；



