#include "config.h"
#include "my_Ardrone.h"
#include "my_event.h"
#include "my_leap.h"

using namespace std;
#include "ardrone/ardrone.h"





int main(int argc, char **argv)
{

	/*while (1)
	{
		if (GetAsyncKeyState(27))
			return 0;
		for (int i = 2; i < 500; i++)
		{
			if (GetAsyncKeyState(i))
				cout << i << endl;
		}
	}*/

	//NF = new frame (protection prise de nouvelle image leap motion
	//CC = calcule des commandes a appliquer en fonction des donnees leap motion
	//AC = applicaton des commandes au parrot
	static pthread_mutex_t	mutex_prot_NF_CC = PTHREAD_MUTEX_INITIALIZER; //protection echange position main
	static pthread_mutex_t	mutex_prot_CC_AC = PTHREAD_MUTEX_INITIALIZER; //protection echange commande
	static pthread_mutex_t	mutex_prot_AC_CC = PTHREAD_MUTEX_INITIALIZER; //protection echange level

	static pthread_mutex_t	mutex_mess_NF = PTHREAD_MUTEX_INITIALIZER; //message fin acquisition position main
	static pthread_mutex_t	mutex_mess_CC = PTHREAD_MUTEX_INITIALIZER; //message fin calcul commande
	static pthread_mutex_t	mutex_mess_AC = PTHREAD_MUTEX_INITIALIZER; //message fin utilisation donnees
#if !MULTI_THREAD
	static pthread_mutex_t	mutex_mess_main = PTHREAD_MUTEX_INITIALIZER; //message preparation terminee
#endif

	static pthread_cond_t	cond_mess_NF = PTHREAD_COND_INITIALIZER; //message fin acquisition position main
	static pthread_cond_t	cond_mess_CC = PTHREAD_COND_INITIALIZER; //message fin calcul commande
	static pthread_cond_t	cond_mess_AC = PTHREAD_COND_INITIALIZER; //message fin utilisation donnees
#if !MULTI_THREAD
	static pthread_cond_t	cond_mess_main = PTHREAD_COND_INITIALIZER; //message preparation terminee
#endif


#if MULTI_THREAD
	SampleListener listener(&mutex_prot_NF_CC, &mutex_mess_NF, &cond_mess_NF);
#else
	SampleListener listener(&mutex_prot_NF_CC, &mutex_mess_NF, &cond_mess_NF, &mutex_mess_AC, &cond_mess_AC, &cond_mess_main);
#endif

	EventTrad eventTrad(&listener, &mutex_prot_CC_AC, &mutex_prot_NF_CC, &mutex_mess_CC, &cond_mess_CC, &mutex_mess_NF, &cond_mess_NF,0);
	listener.setEventTrad(&eventTrad);


#if MULTI_THREAD
	MyARDrone ardrone(&eventTrad, &mutex_prot_CC_AC, &mutex_mess_CC, &cond_mess_CC);
#else
	MyARDrone ardrone(&eventTrad, &mutex_prot_CC_AC, &mutex_mess_CC, &cond_mess_CC, &mutex_mess_AC, &cond_mess_AC);
#endif

	if (ardrone.init() == -1)
		return -1;

	ardrone.print();
	Controller controller;

	pthread_t threadCC;
	pthread_t threadAC;
	pthread_create(&threadAC, NULL, ardrone.compute, (void*)&ardrone);
	pthread_create(&threadCC, NULL, eventTrad.compute, (void*)&eventTrad);
	controller.addListener(listener);
	
#if !MULTI_THREAD
	pthread_cond_wait(&cond_mess_main, &mutex_mess_main);
	pthread_cond_signal(&cond_mess_AC);
#endif

	pthread_join(threadCC, NULL);

#if MULTI_THREAD
	//eventTrad.launch();
#endif


	// See you
#if PARROT
   ardrone.close();
#endif
   controller.removeListener(listener);
   return 0;
}





