#ifndef FI_CONFIG
#define FI_CONFIG

#include <iostream>
#include <fstream>
//#include  <time.h>
//#include <ctime>
#include <pthread.h>
#include <Windows.h>
#include <opencv2/opencv.hpp>	
#include <Eigen/Dense>
#include <math.h>
//#include <Eigen/Core>

using namespace Eigen;
using namespace std;




#define MULTI_THREAD 1 //Ne marche que pour 1 


#define PARROT 0
#define PARROT_CAM 0
#define OUTDOOR 0


//DEBUG

#define AFFICHAGE_3D      1
#define PRINT_INFO        0 
#define SAVE_DATA         0 //a refaire
#define CHRONO		      0 //il est conseillé d'envoyer une commande 30 fois par seconde dans le sdk du parrot


//Modif loop commande
#define MODIF_LOOP_COMMANDE 0


#endif
