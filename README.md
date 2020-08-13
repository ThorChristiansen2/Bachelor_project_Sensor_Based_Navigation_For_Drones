# Bachelor_project_Sensor_Based_Navigation_For_Drones

The bachelor project "Sensor Based Navigation for Drones" was prepared at the department of Electrical Engineering (Automation and Control) at the Technical University of Denmark (DTU) in fulfillment of the requirements for acquiring a Bachelor of Science degree in Electrical Engineering.

The project is written by August Leander Høeg and Thor Vestergaard Christiansen, students at the Technical University of Denmark.

########################################################

Source-code for the VO-pipeline

The source-code for the VO-pipeline used in the Bachelor project "Sensor Based Navigation for Drones" will soon be uploaded. This code is based upon exercises and the mini project from the course "Vision Algorithms for Mobile Robotics" offered at ETH Zürich in the fall semester of 2019 and taught by Prof. Dr. Davide Scaramuzza and Ph.D. students Mathias Gehrig and Daniel Gehrig. A link to the website of the course is here: http://rpg.ifi.uzh.ch/teaching.html 

Moreover a file called Matrix.h (c) Bjarne Stroustrup, Texas A&M University is also used as a part of the VO-pipeline. This file was made by Prof. Bjarne Stroustrup, the designer and original implementer of the programming language C++. A link to Prof. Bjarne Stroustrup's website is: https://www.stroustrup.com/index.html 

To use the code for the VO-pipeline on a Raspberry Pi, the following software packages are needed:

  1) OpenCV C++ Library. For this project version 4.3 was used. The official website for the OpenCV C++ Library can be found here: https://opencv.org/.
     A great guide for installing the OpenCV C++ library on a Raspberry Pi can be found on this website from Q-engineering: 
     https://qengineering.eu/install-opencv-4.3-on-raspberry-pi-4.html 
     
  2) A C++ API for using Raspberry Pi camera with OpenCV. This C++ API is made by Rafael Muñoz Salinas from Aplicaciones de la Visión Artifical at Universidad de Córdoba and can         be found on the following website: https://www.uco.es/investiga/grupos/ava/node/40
  
  3) To calibrate the camera and find the matrix K containing the intrinsic parameters, the OpenCV C++ library can be used. To learn how to use the OpenCV C++ library to              calibrate the camera, George D. Lecakes Jr. has provided some great videoes on his youtube-channel George Lecakes. The first video in a series of five videos can be found        here: https://www.youtube.com/watch?v=HNfPbw-1e_w&t=53s  

########################################################

Source-code for the Controller

The source-code for the controller builds upon code from DJI-osdk, which can be downloaded from DJI's official website: https://developer.dji.com/onboard-sdk/ 

Moreover, the source-code for the controller builds upon internal libraries developed by the Group of Automation and Control at the Department of Electrical Engineering at the Technical University of Denmark, DTU.

########################################################

This was updated on the 13th of August 2020. 
