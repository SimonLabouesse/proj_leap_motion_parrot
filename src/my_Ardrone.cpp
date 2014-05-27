#include "my_Ardrone.h"


MyARDrone::MyARDrone()
{
	m_eventTrad = NULL;
	m_modeCamera = 0;
	m_mutex_prot_CC_AC = NULL;
	m_mutex_mess_CC = NULL;
	m_cond_mess_CC = NULL;
}

int MyARDrone::init()
{
#if PARROT
	if (!open())
	{
		printf("Failed to initialize.\n");
		return -1;
	}
#endif
	return 0;
}

MyARDrone::MyARDrone(	EventTrad* eventTrad,				pthread_mutex_t* mutex_prot_CC_AC, 
						pthread_mutex_t*	mutex_mess_CC ,	pthread_cond_t* cond_mess_CC
#if !MULTI_THREAD
						,pthread_mutex_t*	mutex_mess_AC , pthread_cond_t*	cond_mess_AC
#endif
)
{
	m_eventTrad = eventTrad; 
	m_modeCamera = 0;
	m_mutex_prot_CC_AC = mutex_prot_CC_AC;
	m_mutex_mess_CC = mutex_mess_CC;
	m_cond_mess_CC = cond_mess_CC;
#if !MULTI_THREAD
	m_mutex_mess_AC	= mutex_mess_AC;
	m_cond_mess_AC  = cond_mess_AC;
#endif
}




void* MyARDrone::compute(void* pvoid)
{
	MyARDrone* myARDrone = (MyARDrone*)pvoid;
#if OUTDOOR
	myARDrone->setOutdoorMode(true);
#endif
	int count = 0;

#if CHRONO 
	double timePred = 0;
#endif

	while (1)
	{
		//attendre calcul de la commande par event.
		pthread_cond_wait(myARDrone->m_cond_mess_CC, myARDrone->m_mutex_mess_CC);

		//cout<<"parrot"<<endl;

#if CHRONO
		double timeStart = clock();
#endif
#if PARROT
		if (!myARDrone->update()) break;
#if PARROT_CAM
		IplImage*image = myARDrone->getImage();
#endif
#endif

		static bool onGround = true;
		if( (onGround == true) && (static_cast<bool>(myARDrone->m_eventTrad->isTakeOff()) == true))
		{
			onGround = false;
			cout << "takeoff" << endl;
		}
		if ( (onGround == false) && (myARDrone->m_eventTrad->isTakeOff() == false))
		{
			onGround = true;
			cout << "landing" << endl;
		}


		//cout << "taratata" << endl;
		pthread_mutex_lock(myARDrone->m_mutex_prot_CC_AC);
#if PARROT
		if ((static_cast<bool>(myARDrone->onGround()) == true) && (static_cast<bool>(myARDrone->m_eventTrad->isTakeOff()) == true))
		{
			cout << "takeoff" << endl;
			myARDrone->takeoff();
		}
		if ((myARDrone->onGround() == false) && (myARDrone->m_eventTrad->isTakeOff() == false))
		{
			cout << "landing" << endl;
			myARDrone->landing();
		}
#endif
		Vector vitesse = myARDrone->m_eventTrad->getVitesse();
		double vr = -myARDrone->m_eventTrad->getRotation();
		int doLooping = myARDrone->m_eventTrad->doLooping();
		myARDrone->m_modeCamera = myARDrone->m_eventTrad->getCamMode();
		//myARDrone->keyEvent(vitesse, vr);

		pthread_mutex_unlock(myARDrone->m_mutex_prot_CC_AC);

		double vx = vitesse.x;
		double vy = -vitesse.y;
		double vz = vitesse.z;

#if PRINT_INFO
		count++;
		if ((vx != 0) || (vz != 0) || (vy != 0) || (vr != 0))
		{
			std::cout << "num  " << count << " :  (" << vx << " , " << vy << " , " << vz << " , " << vr << ")		" << clock() << std::endl;
		}
#endif

	

#if CHRONO
		double time = clock();
		//cout <<"Entier: "<< ((time - timePred) / CLOCKS_PER_SEC) << endl;
		//cout << "Drone : " << ((time - timeStart) / CLOCKS_PER_SEC) << endl;
		timePred = time;
#endif

		

#if PARROT
		myARDrone->move3D(vx, vy, vz, vr);
	if (doLooping != -1)
	{
		//ARDRONE_ANIM_FLIP_AHEAD
		myARDrone->setAnimation(doLooping);
	}
#if PARROT_CAM
		// Display the image
		cvShowImage("camera", image);
		cv::moveWindow("camera", 350, 400);
		cvWaitKey(25);
#endif
#endif

#if !MULTI_THREAD	
		pthread_cond_signal(myARDrone->m_cond_mess_AC);
#endif
	}

	return NULL;
}






//
//int MyARDrone::keyEvent(Vector& vitesse, double& rotation)
//{
//	int key = cvWaitKey(50);
//	if (key == 0x1b)
//		return -1;
//
//	if (key == 0x260000)
//		vitesse.x = 1.0;
//	if (key == 0x280000)
//		vitesse.x = -1.0;
//	if (key == 0x250000)
//		rotation = 1.0;
//	if (key == 0x270000)
//		rotation = -1.0;
//	if (key == 'a')
//		vitesse.z = 1.0;
//	if (key == 'q')
//		vitesse.z = -1.0;
//
//	if (key == ' ')
//	{
//		if (onGround())
//			takeoff();
//		else
//			landing();
//	}
//		
//
//
//	return 0;
//}


void MyARDrone::changeCameraMode()
{
	// Change camera
	m_modeCamera++;
	m_modeCamera %= 4;
	setCamera(m_modeCamera);
}

void MyARDrone::printBatteryLevel()
{
	printf("Battery = %d%%\n", getBatteryPercentage());
}

void MyARDrone::print()
{
	// Battery
	printf("Battery = %d%%\n", getBatteryPercentage());

	// Instructions
	printf("***************************************\n");
	printf("*       CV Drone sample program       *\n");
	printf("*           - How to Play -           *\n");
	printf("***************************************\n");
	printf("*                                     *\n");
	printf("* - Controls -                        *\n");
	printf("*    'Space' -- Takeoff/Landing       *\n");
	printf("*    'Up'    -- Move forward          *\n");
	printf("*    'Down'  -- Move backward         *\n");
	printf("*    'Left'  -- Turn left             *\n");
	printf("*    'Right' -- Turn right            *\n");
	printf("*    'Q'     -- Move upward           *\n");
	printf("*    'A'     -- Move downward         *\n");
	printf("*                                     *\n");
	printf("* - Others -                          *\n");
	printf("*    'C'     -- Change camera         *\n");
	printf("*    'Esc'   -- Exit                  *\n");
	printf("*                                     *\n");
	printf("***************************************\n\n");
	printf("*    '1'     -- Select level 1        *\n");
	printf("*    '2'     -- Select level 2        *\n");
	printf("*    '3'     -- Select level 3        *\n");
	printf("*    two finger for Takeoff/Landing   *\n");
	printf("***************************************\n\n");

}


