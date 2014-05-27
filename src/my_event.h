#ifndef FI_MY_EVENT
#define FI_MY_EVENT

#include "config.h"
#include "my_leap.h"

#include "config_level.h"

#define TIME_MIN_UNDER_TWO_LOOPING 5000
#define TIME_MIN_UNDER_TWO_KEYPRESS 800

struct ParamLevel
{
	double 	depMinMag;
	double 	depMinDir;
	double 	angMin;
	double 	factVitesse;
	double 	factAngle;
	bool	isLoopingOk;
	ParamLevel(){ };

	ParamLevel(double a, double b, double c, double d, double e, bool f = false){
		depMinMag = a;
		depMinDir = b;
		angMin = c;
		factVitesse = d;
		factAngle = e;
		isLoopingOk = f;
	};


	static const ParamLevel& level1() {
		static ParamLevel paramLevel(DEP_MIN_MAG_L1, DEP_MIN_MAG_L1, ANG_MIN_L1, FACT_VITESSE_L1, FACT_ANGLE_L1,true);
		return paramLevel;
	};
	static const ParamLevel& level2() {
		static ParamLevel paramLevel(DEP_MIN_MAG_L2, DEP_MIN_MAG_L2, ANG_MIN_L2, FACT_VITESSE_L2, FACT_ANGLE_L2);
		return paramLevel;
	};
	static const ParamLevel& level3() {
		static ParamLevel paramLevel(DEP_MIN_MAG_L3, DEP_MIN_MAG_L3, ANG_MIN_L3, FACT_VITESSE_L3, FACT_ANGLE_L3,true);
		return paramLevel;
	};


};


class EventTrad
{
public:
	EventTrad();
	EventTrad(SampleListener* sampleListener, pthread_mutex_t* mutex_prot_CC_AC,
	pthread_mutex_t* mutex_prot_NF_CC,	pthread_mutex_t* mutex_mess_CC,	pthread_cond_t* cond_mess_CC,
	pthread_mutex_t* mutex_mess_NF, pthread_cond_t* cond_mess_NF, int level = 0);
	
	static void * compute(void *);
	
	void setLevel(int level);
	int getLevel();
	Vector& getVitesse();
	double	getRotation();
	bool	isTakeOff();
	int		doLooping();
	int		getCamMode();



private:

	SampleListener* m_sampleListener;

	int			m_level;
	ParamLevel	m_paramLevel;
	Vector		m_vitesse;
	double		m_rotation;
	bool		m_isTakeOff;
	int			m_looping;
	int			m_timeSinceLastLooping;
	int			m_timeSinceLastKey;
	int			m_camMode;

	void stop();

private:
	typedef void (EventTrad::* Level)(const LeapDataToExchange* leapDataA, const LeapDataToExchange* leapDataB);
	Level  levels[NBR_LEVEL];
	void level1(const LeapDataToExchange* leapDataA, const LeapDataToExchange* leapDataB);
	void level2(const LeapDataToExchange* leapDataA, const LeapDataToExchange* leapDataB);
	void level3(const LeapDataToExchange* leapDataA, const LeapDataToExchange* leapDataB);

	void translation(		const LeapDataToExchange* leapData, bool& isMove, Vector& vitesse);
	void translationProp(	const LeapDataToExchange* leapData, bool& isMove, Vector& vitesse);
	void angle(				const LeapDataToExchange* leapData, bool& isMove, double& vitesseAng);
	void angleProp(			const LeapDataToExchange* leapData, bool& isMove, double& vitesseAng);
	
	int keyEvent();


	pthread_mutex_t*	m_mutex_prot_CC_AC;
	pthread_mutex_t*	m_mutex_prot_NF_CC;
	pthread_mutex_t*	m_mutex_mess_CC;
	pthread_cond_t*		m_cond_mess_CC;
	pthread_mutex_t*	m_mutex_mess_NF;
	pthread_cond_t*		m_cond_mess_NF;


};




#endif



