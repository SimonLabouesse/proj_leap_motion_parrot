#ifndef FI_MYARDRONE
#define FI_MYARDRONE

#include "my_event.h"
#include "ardrone/ardrone.h"


class MyARDrone : public ARDrone
{
public:
	MyARDrone();
	MyARDrone(	EventTrad* eventTrad,
				pthread_mutex_t*	mutex_prot_CC_AC,
				pthread_mutex_t*	mutex_mess_CC,
				pthread_cond_t*		cond_mess_CC
#if !MULTI_THREAD
				,pthread_mutex_t*	mutex_mess_AC,
				pthread_cond_t*		cond_mess_AC
#endif
				);
		
	static void * compute(void *);

	void changeCameraMode();
	void printBatteryLevel();

	void print();
	int init();
private:

	EventTrad* m_eventTrad;
	int m_modeCamera;

	pthread_mutex_t*	m_mutex_prot_CC_AC;
	pthread_mutex_t*	m_mutex_mess_CC;
	pthread_cond_t*		m_cond_mess_CC;

#if !MULTI_THREAD
	pthread_mutex_t*	m_mutex_mess_AC;
	pthread_cond_t*		m_cond_mess_AC;
#endif

	int keyEvent(Vector& vitesse, double& rotation);


};








#endif
