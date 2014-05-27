#include "my_event.h"

EventTrad::EventTrad()
{


}

EventTrad::EventTrad(SampleListener* sampleListener, pthread_mutex_t* mutex_prot_CC_AC, pthread_mutex_t* mutex_prot_NF_CC,
					pthread_mutex_t* mutex_mess_CC, pthread_cond_t* cond_mess_CC, pthread_mutex_t* mutex_mess_NF, 
					pthread_cond_t* cond_mess_NF, int level) : m_rotation(0.0), m_vitesse(0.0, 0.0, 0.0)
{
	levels[0] = &EventTrad::level1;
	levels[1] = &EventTrad::level2;
	levels[2] = &EventTrad::level3;
	//, &EventTrad::level1, &EventTrad::level0, &EventTrad::level0, &EventTrad::level0
	m_level = 0;
	m_paramLevel = ParamLevel::level1();
	m_sampleListener = sampleListener;
	m_isTakeOff = false;
	m_looping = -1;
	m_timeSinceLastLooping = 0;
	m_timeSinceLastKey = 0;
	m_camMode = 0;

	m_mutex_prot_CC_AC = mutex_prot_CC_AC;
	m_mutex_prot_NF_CC = mutex_prot_NF_CC;
	m_mutex_mess_CC	= mutex_mess_CC;
	m_cond_mess_CC = cond_mess_CC;
	m_mutex_mess_NF	= mutex_mess_NF;
	m_cond_mess_NF = cond_mess_NF;
}
	
//void EventTrad::launch()
//{
//	pthread_t thread;
//	void* (*pVoid)(void *);
//	pthread_create(&thread, NULL, EventTrad::compute, (void *) this);
//}

void* EventTrad::compute(void* pvoid)
{
	EventTrad* eventTrad = (EventTrad*) pvoid;

	eventTrad->stop();

	while (1)
	{		
		//attendre acquisition nouvelle frame par le leap motion.
		pthread_cond_wait(eventTrad->m_cond_mess_NF, eventTrad->m_mutex_mess_NF);

		//cout << "Calc commande" << endl;
		pthread_mutex_lock(eventTrad->m_mutex_prot_NF_CC);

		const LeapDataToExchange leapDataA = *eventTrad->m_sampleListener->getDataA();
		const LeapDataToExchange leapDataB = *eventTrad->m_sampleListener->getDataB();
		int nbrHand = eventTrad->m_sampleListener->getNbrHand();
		
		pthread_mutex_unlock(eventTrad->m_mutex_prot_NF_CC);

		if ((leapDataA.actionAct == 2) && (leapDataA.isFirstTimeNewAction) )
		{
			std::cout << "TakeOff/Landing leap" << std::endl;
			eventTrad->m_isTakeOff = !eventTrad->m_isTakeOff;
		}

		if ((leapDataA.actionAct > 2) && (nbrHand))
		{
			//eventTrad->m_isTakeOff = leapDataA.isTakeOff;
			((*eventTrad).*eventTrad->levels[eventTrad->m_level])(&leapDataA, &leapDataB);		//appel de la fonction level correspondant au niveau actuel
		}
		else
		{
			eventTrad->stop();
		}

		if (eventTrad->keyEvent() == -1)
			return NULL;


		pthread_cond_signal(eventTrad->m_cond_mess_CC);

	}
	return NULL;
}


void EventTrad::stop()
{

	pthread_mutex_lock(m_mutex_prot_CC_AC);

	m_vitesse.x = 0;
	m_vitesse.y = 0;
	m_vitesse.z = 0;
	m_rotation  = 0;

	pthread_mutex_unlock(m_mutex_prot_CC_AC);

}


void EventTrad::setLevel(int level)
{
#if PRINT_INFO
	std::cout << "level set to " << level << std::endl;
#endif
	m_level = level-1;
}

int EventTrad::getLevel()
{
	return m_level+1;
}

Vector& EventTrad::getVitesse()
{
	return m_vitesse;
}

double EventTrad::getRotation()
{
	return m_rotation;
}

bool EventTrad::isTakeOff()
{
	return m_isTakeOff;
}

int EventTrad::doLooping()
{
	return m_looping;
}

int	EventTrad::getCamMode()
{
	return m_camMode;
}

int EventTrad::keyEvent()
{

	if (GetAsyncKeyState(27))//echap
		return -1;

	pthread_mutex_lock(m_mutex_prot_CC_AC);

	if (GetAsyncKeyState(38))//fleche haut
		m_vitesse.x = 1.0;
	if (GetAsyncKeyState(40))//fleche bas
		m_vitesse.x = -1.0;
	if (GetAsyncKeyState(39))//fleche droite
		m_rotation = 1.0;
	if (GetAsyncKeyState(37))//fleche gauche
		m_rotation = -1.0;
	if (GetAsyncKeyState(65))//a
		m_vitesse.z = 1.0;
	if (GetAsyncKeyState(81))//q
		m_vitesse.z = -1.0;

	if ((clock() - m_timeSinceLastKey) > TIME_MIN_UNDER_TWO_KEYPRESS)
	{
		if (GetAsyncKeyState(49) || GetAsyncKeyState(97))//1
		{
			m_paramLevel = ParamLevel::level1();
			setLevel(1);
			m_timeSinceLastKey = clock();
		}
		if (GetAsyncKeyState(50) || GetAsyncKeyState(98))//2
		{
			m_paramLevel = ParamLevel::level2();
			setLevel(2);
			m_timeSinceLastKey = clock();
		}
		if (GetAsyncKeyState(51) || GetAsyncKeyState(99))//3
		{
			m_paramLevel = ParamLevel::level3();
			setLevel(3);
			m_timeSinceLastKey = clock();
		}
		if (GetAsyncKeyState(32))//espace
		{
			std::cout << "TakeOff/Landing" << std::endl;
			m_isTakeOff = !m_isTakeOff;
			m_timeSinceLastKey = clock();
		}

		if (GetAsyncKeyState(67))//c
		{
			m_camMode++;
			m_camMode %= 4;
			m_timeSinceLastKey = clock();
		}
	}
	m_looping = -1;
	if (m_paramLevel.isLoopingOk && ((clock() - m_timeSinceLastLooping) > TIME_MIN_UNDER_TWO_LOOPING) )
	{
		if (	GetAsyncKeyState(13) )//entrer
		{
			m_looping = 16;
			m_timeSinceLastLooping = clock();
		}
		if (	GetAsyncKeyState(68) )//d
		{
			m_looping = 9;
			m_timeSinceLastLooping = clock();
		}
		if (	GetAsyncKeyState(70) )//f
		{
			m_looping = 9;
			m_timeSinceLastLooping = clock();
		}
		//cout << "looping :  " << m_looping <<"   "<< clock()<< endl;
	}
	pthread_mutex_unlock(m_mutex_prot_CC_AC);

	return 0;
}

void EventTrad::level1(const LeapDataToExchange* leapDataA, const LeapDataToExchange* leapDataB)
{
	Vector vitesseA(0.0, 0.0, 0.0);
	Vector vitesseB(0.0, 0.0, 0.0);
	double rotation = 0;
	bool isMoveA, isMoveB, isMoveAng = false;

	translation(leapDataA, isMoveA, vitesseA);
	translation(leapDataB, isMoveB, vitesseB);
	//angle(leapDataA, isMoveAng, rotation);

	//cout <<"vitesse : "<< vitesseB.y <<"    "<<clock()<< endl;

	if (isMoveA || isMoveAng || vitesseB.x )
	{
		pthread_mutex_lock(m_mutex_prot_CC_AC);
		m_vitesse.x =  vitesseA.z;
		m_vitesse.y = -vitesseA.x;
		m_vitesse.z = -vitesseA.y;
		m_rotation =  -vitesseB.x;
		pthread_mutex_unlock(m_mutex_prot_CC_AC);

#if PRINT_INFO
		static int count = 0;
		count++;
		//std::cout << "num  " <<count<< " :  (" << m_vitesse.x << " , " << m_vitesse.y << " , " << m_vitesse.z << ")" << std::endl;
#endif

	}
	else
	{
		stop();
	}

}


void EventTrad::level2(const LeapDataToExchange* leapDataA, const LeapDataToExchange* leapDataB)
{
	double magnitude = leapDataA->diffPos.magnitude();
	Vector diffPos = leapDataA->diffPos;
	Vector vitesse(0.0, 0.0, 0.0);
	bool isMove, isMoveAng;
	double rotation = 0;
	
	translation(leapDataA, isMove, vitesse);
	angle(leapDataA, isMoveAng, rotation);

	if (isMove || isMoveAng)
	{
		pthread_mutex_lock(m_mutex_prot_CC_AC);
		m_vitesse.x = vitesse.z;
		m_vitesse.y = -vitesse.x;
		m_vitesse.z = -vitesse.y;
		m_rotation = rotation;
		pthread_mutex_unlock(m_mutex_prot_CC_AC);

#if PRINT_INFO
		static int count = 0;
		count++;
		//std::cout << "num  " <<count<< " :  (" << m_vitesse.x << " , " << m_vitesse.y << " , " << m_vitesse.z << ")" << std::endl;
#endif
	}
	else
	{
		stop();
	}
}



void EventTrad::level3(const LeapDataToExchange* leapDataA, const LeapDataToExchange* leapDataB)
{
	double magnitude = leapDataA->diffPos.magnitude();
	Vector diffPos = leapDataA->diffPos;
	Vector vitesseA(0.0, 0.0, 0.0);
	Vector vitesseB(0.0, 0.0, 0.0);
	double rotation = 0;
	bool isMoveA, isMoveB, isMoveAng;

	translationProp(leapDataA, isMoveA, vitesseA);
	translationProp(leapDataB, isMoveB, vitesseB);
	angleProp(leapDataA, isMoveAng, rotation);

	if (isMoveA || isMoveB || vitesseB.x)
	{
		pthread_mutex_lock(m_mutex_prot_CC_AC);
		m_vitesse.x = vitesseA.z;
		m_vitesse.y = -vitesseA.x;
		m_vitesse.z = -vitesseA.y;
		m_rotation = rotation;// -vitesseB.x;
		pthread_mutex_unlock(m_mutex_prot_CC_AC);
		
#if PRINT_INFO
		static int count = 0;
		count++;
		//std::cout << "num  " <<count<< " :  (" << m_vitesse.x << " , " << m_vitesse.y << " , " << m_vitesse.z << ")" << std::endl;
#endif

	}
	else
	{
		stop();
	}
}



void EventTrad::translation(const LeapDataToExchange* leapData, bool& isMove, Vector& vitesse)
{
	double magnitude = leapData->diffPos.magnitude();
	Vector diffPos = leapData->diffPos;
	isMove = false;
	if (magnitude > m_paramLevel.depMinMag)
	{
		isMove = true;
		int sizedepMin = m_paramLevel.depMinDir;
		if (diffPos.x > sizedepMin)
			vitesse.x = 1.0;
		if (diffPos.x < -sizedepMin)
			vitesse.x = -1.0;
		if (diffPos.y >  sizedepMin)
			vitesse.y = 1.0;
		if (diffPos.y < -sizedepMin)
			vitesse.y = -1.0;
		if (diffPos.z >  sizedepMin)
			vitesse.z = 1.0;
		if (diffPos.z < -sizedepMin)
			vitesse.z = -1.0;
	}
	vitesse = vitesse*m_paramLevel.factVitesse;
}


void EventTrad::translationProp(const LeapDataToExchange* leapData, bool& isMove, Vector& vitesse)
{
	double magnitude = leapData->diffPos.magnitude();
	Vector diffPos = leapData->diffPos;
	isMove = false;
	if (magnitude > m_paramLevel.depMinMag)
	{
		isMove = true;
		int sizedepMin = m_paramLevel.depMinDir;
		if (abs(diffPos.x) > sizedepMin)
			vitesse.x = diffPos.x;
		if (abs(diffPos.y) > sizedepMin)
			vitesse.y = diffPos.y;
		if (abs(diffPos.z) > sizedepMin)
			vitesse.z = diffPos.z;		

		vitesse = vitesse*m_paramLevel.factVitesse;
	}

}

void EventTrad::angle(const LeapDataToExchange* leapData, bool& isMove, double& vitesseAng)
{
	//x = pitch  
	//y = roll 
	//z = yaw   
	double diffAngles = leapData->diffAngles.z;
	isMove = false;
	
	if (abs(diffAngles)*360/PI > m_paramLevel.angMin)
	{
		isMove = true;
		if (diffAngles>0)
			vitesseAng = 1;
		if (diffAngles<0)
			vitesseAng = -1;
		//std::cout << "angles -  " << vitesseAng <<"            "<<clock()<< std::endl;
	}
	vitesseAng *= m_paramLevel.factAngle;
}

void EventTrad::angleProp(const LeapDataToExchange* leapData, bool& isMove, double& vitesseAng)
{
	//x = pitch  
	//y = roll 
	//z = yaw   
	double diffAngles = leapData->diffAngles.z;
	isMove = false;

	if (abs(diffAngles) * 360 / PI > m_paramLevel.angMin)
	{
		isMove = true;
		vitesseAng = diffAngles * m_paramLevel.factAngle;
		//std::cout << "angles -  " << vitesseAng << std::endl;
	}
}









