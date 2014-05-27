#ifndef FI_MY_LEAP
#define FI_MY_LEAP

#include "Leap.h"
#include "config.h"


#define TIME_OF_ANGLE_MODIFICATION 500

using namespace Leap;

class EventTrad;


struct LeapData
{
	int nbrFingersAct = -1;
	int nbrFingersPred = -1;
	int actionAct = 0;
	
	//int actionPred = 0;
	int count = -1;
	bool isFirstTimeNewAction;
	
	Vector3d posMoy;
	Vector3d normalMoy;
	Vector3d directMoy;
#if ORIENTATION_LEAP
	Matrix3d base;
#endif

	int timeOfOrigine;
	Vector3d origine;
	Vector3d origineNormal;
	Vector3d origineDirect;
		  
	Vector3d diffPos;		//position par rapport à l'origine
	Vector3d diffAngle;		//angles   par rapport à l'origine

	LeapData();
	void reset();
	void stop();

};

struct LeapDataToExchange
{
	bool isFirstTimeNewAction;
	int actionAct;
	Vector diffPos;		//position par rapport à l'origine
	Vector diffAngles;	//angles   par rapport à l'origine
	int isTakeOff;

	LeapDataToExchange();
	void reset();
	void update(LeapData& data, bool tisTakeoff);
};




//----------------------------------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------------------------------//



class SampleListener : public Listener
{

public:
	SampleListener();
#if MULTI_THREAD
	SampleListener(pthread_mutex_t*	mutex_prot_NF_CC, pthread_mutex_t*	mutex_mess_NF, pthread_cond_t*	cond_mess_NF);
#else
	SampleListener(pthread_mutex_t*	mutex_prot_NF_CC, pthread_mutex_t*	mutex_mess_NF, pthread_cond_t*	cond_mess_NF, pthread_mutex_t*	mutex_mess_AC, pthread_cond_t*	cond_mess_AC, pthread_cond_t* cond_mess_main);
#endif

	virtual void onInit(const Controller&);
	virtual void onConnect(const Controller&);
	virtual void onDisconnect(const Controller&);
	virtual void onExit(const Controller&);
	virtual void onFrame(const Controller&);
	virtual void onFocusGained(const Controller&);
	virtual void onFocusLost(const Controller&);
	
	int							getNbrHand();
	const LeapDataToExchange*	getDataA();
	const LeapDataToExchange*	getDataB();

	void setEventTrad(EventTrad* eventTrad) { m_eventTrad = eventTrad; };

private : 

	int		m_nbrHand;
	bool	m_isTakeoff;

	LeapData m_data;
	LeapDataToExchange m_dataToExchange;
	LeapData m_dataS;
	LeapDataToExchange m_dataToExchangeS;

	void handTraitement(Hand& hand, LeapData& data);
	void stop();
	
private:

	EventTrad* m_eventTrad;

	pthread_mutex_t*	m_mutex_prot_NF_CC;
	pthread_mutex_t*	m_mutex_mess_NF;
	pthread_cond_t*		m_cond_mess_NF;
#if!MULTI_THREAD
	pthread_mutex_t*	m_mutex_mess_AC;
	pthread_cond_t*		m_cond_mess_AC;
	pthread_cond_t*		m_cond_mess_main;
#endif
#if CHRONO
	double timePred;
#endif
#if SAVE_DATA
	ofstream		m_file;
	vector<Vector3d>	m_memoirePos;
	vector<Vector3d>	m_memoireOri;
	vector<Vector3d>	m_memoireDiff;
	vector<Vector3d>	m_memoireNorm;
	vector<Vector3d>	m_memoireOriNorm;
	vector<Vector3d>	m_memoireDirect;
	vector<Vector3d>	m_memoireOriDirect;
	vector<Vector3d>	m_memoireDiffAngle;

	void saveData(Vector3d& posAct, Vector3d& normalAct, Vector3d& directAct);

#endif

#if AFFICHAGE_3D
	cv::Mat m_image;


	void affichage3d();
	void affichage3dOneHand(LeapData& data, int sizeWindows, int facteur);
	void projPoint(const Vector3d& point, int sizeWindows, cv::Point& pointProj);
#endif

};







#endif

