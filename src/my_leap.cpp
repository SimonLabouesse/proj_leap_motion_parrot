/******************************************************************************\
* Copyright (C) 2012-2013 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include "my_leap.h"
#include "my_event.h"

#define TIME_DETECT 0.5

LeapData::LeapData() 
{
	reset();
}

void LeapData::reset()
{
	posMoy.x() = 0.0;
	posMoy.y() = 0.0;
	posMoy.z() = 0.0;

	normalMoy.x() = 0.0;
	normalMoy.y() = 0.0;
	normalMoy.z() = 0.0;

	directMoy.x() = 0.0;
	directMoy.y() = 0.0;
	directMoy.z() = 0.0;

	origine.x() = 0.0;
	origine.y() = 200.0;
	origine.z() = 0.0;

	origineNormal.x() = 0.0;
	origineNormal.y() = -1.0;
	origineNormal.z() = 0.0;

	origineDirect.x() = 0.0;
	origineDirect.y() = 0.0;
	origineDirect.z() = -1.0;

	diffPos.x() = 0.0;
	diffPos.y() = 0.0;
	diffPos.z() = 0.0;

	diffAngle.x() = 0.0;
	diffAngle.y() = 0.0;
	diffAngle.z() = 0.0;

#if ORIENTATION_LEAP
		base << 1, 0, 0,
			0, 1, 0,
			0, 0, 1;
#endif
		nbrFingersAct = -1;
		nbrFingersPred = -1;
		actionAct = 0;
		isFirstTimeNewAction = false;
		count = -1;

		timeOfOrigine = INT_MAX;

}

void LeapData::stop()
{
	diffPos.x() = 0.0;
	diffPos.y() = 0.0;
	diffPos.z() = 0.0;

	diffAngle.x() = 0.0;
	diffAngle.y() = 0.0;
	diffAngle.z() = 0.0;
}


LeapDataToExchange::LeapDataToExchange() 
{
	reset();
}

void LeapDataToExchange::reset()
{
	diffPos.x = 0.0;
	diffPos.x = 0.0;
	diffPos.x = 0.0;

	diffAngles.x = 0.0;
	diffAngles.x = 0.0;
	diffAngles.x = 0.0;

	isFirstTimeNewAction = false;
	actionAct = 0;
	isTakeOff = false;
}

void LeapDataToExchange::update(LeapData& data, bool tisTakeoff)
{
	isFirstTimeNewAction = data.isFirstTimeNewAction;
	actionAct = data.actionAct;
	diffPos.x = data.diffPos(0);
	diffPos.y = data.diffPos(1);
	diffPos.z = data.diffPos(2);
	diffAngles.x = data.diffAngle(0);
	diffAngles.y = data.diffAngle(1);
	diffAngles.z = data.diffAngle(2);
	isTakeOff = tisTakeoff;

}

//----------------------------------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------------------------------//


SampleListener::SampleListener()
{
}


#if MULTI_THREAD
SampleListener::SampleListener(pthread_mutex_t*	mutex_prot_NF_CC, pthread_mutex_t*	mutex_mess_NF, pthread_cond_t*	cond_mess_NF)
#else
SampleListener::SampleListener(pthread_mutex_t*	mutex_prot_NF_CC, pthread_mutex_t*	mutex_mess_NF, pthread_cond_t*	cond_mess_NF, pthread_mutex_t*	mutex_mess_AC, pthread_cond_t*	cond_mess_AC, pthread_cond_t* cond_mess_main)
#endif
{
	m_eventTrad = NULL;
	m_mutex_prot_NF_CC = mutex_prot_NF_CC;
	m_mutex_mess_NF = mutex_mess_NF;
	m_cond_mess_NF = cond_mess_NF;
#if !MULTI_THREAD
	m_mutex_mess_AC = mutex_mess_AC;
	m_cond_mess_AC = cond_mess_AC;
	m_cond_mess_main = cond_mess_main;
#endif
#if CHRONO
	timePred = 0;
#endif
#if AFFICHAGE_3D
	//cv::namedWindow("leapMotion", cv::WINDOW_AUTOSIZE);

#endif

}


void SampleListener::onInit(const Controller& controller) 
{
	std::cout << "Initialized" << std::endl;
	m_isTakeoff = false;
	/*m_angle = 0;
	m_position.x = 0;
	m_position.y = 0;
	m_position.z = 0;*/
#if SAVE_DATA
	time_t now = time(NULL);
	struct tm * tm = localtime(&now);
	char date[64];
	strftime(date, sizeof date, "%A_%B_%d_%Y_%H-%M-%S", tm);
	string filename = "../../resultats/enregistrements/leap_motion_" + string(date) + ".txt";
	m_file.open(filename);
#endif

}

void SampleListener::onConnect(const Controller& controller) 
{
	std::cout << "Connected" << std::endl;
	controller.enableGesture(Gesture::TYPE_CIRCLE);
	controller.enableGesture(Gesture::TYPE_KEY_TAP);
	controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
	controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) 
{
	
}

void SampleListener::onExit(const Controller& controller) 
{
#if SAVE_DATA
	m_file.close();
#endif
}

void SampleListener::onFrame(const Controller& controller) 
{
#if !MULTI_THREAD
	pthread_cond_signal(m_cond_mess_main);
	pthread_cond_wait (m_cond_mess_AC, m_mutex_mess_AC);
#endif
	//cout << "Leap  "<<clock() << endl;

	int timeToStay = 15;
#if PARROT
	timeToStay = 20;
#endif
	int numMoy = 2;
#if PARROT
	numMoy = 3;
#endif
	int denMoy = numMoy + 1;
	int nbrHand = 0;
	Frame frame = controller.frame();
	stop();
	
	if (!frame.hands().isEmpty())
	{

		nbrHand = frame.hands().count();

		if (nbrHand == 1)
		{
			Hand hand = frame.hands()[0];
			if (hand.timeVisible()> TIME_DETECT)
				handTraitement(hand, m_data);
		}
		if (m_nbrHand == 2)
		{
			Hand handA = frame.hands()[0];
			Hand handB = frame.hands()[1];
			Vector3d posA(handA.palmPosition().x, handA.palmPosition().y, handA.palmPosition().z);
			Vector3d posB(handB.palmPosition().x, handB.palmPosition().y, handB.palmPosition().z);

			if ((handA.timeVisible() > TIME_DETECT) && (handB.timeVisible() > TIME_DETECT))
			{
				if (posA.x() < posB.x())
				{
					Hand temp = handA;
					handA = handB;
					handB = temp;
				}
			}

			if (handA.timeVisible()>TIME_DETECT)
				handTraitement(handA, m_data);
			if (handB.timeVisible()>TIME_DETECT)
				handTraitement(handB, m_dataS);
			
		}

	}
		


	pthread_mutex_lock(m_mutex_prot_NF_CC);

	m_nbrHand = nbrHand;
	m_dataToExchange.update(m_data, m_isTakeoff);
	m_dataToExchangeS.update(m_dataS, m_isTakeoff);

	pthread_mutex_unlock(m_mutex_prot_NF_CC);
	pthread_cond_signal(m_cond_mess_NF);


#if AFFICHAGE_3D
	affichage3d();
#endif

#if CHRONO
	double time = clock();
	cout <<"Leap : "<< ((time - timePred) / CLOCKS_PER_SEC) << endl;
	timePred = time;
#endif

}

void SampleListener::onFocusGained(const Controller& controller) {

}

void SampleListener::onFocusLost(const Controller& controller) 
{

}


void SampleListener::handTraitement(Hand& hand, LeapData& data)
{

	int timeToStay = 15;
#if PARROT
	timeToStay = 20;
#endif
	int numMoy = 2;
#if PARROT
	numMoy = 3;
#endif
	int denMoy = numMoy + 1;

	// Check if the hand has any fingers
	const FingerList fingers = hand.fingers();
	data.nbrFingersAct = fingers.count();

	Vector3d posAct(hand.palmPosition().x, hand.palmPosition().y, hand.palmPosition().z);
	Vector3d normalAct(hand.palmNormal().x, hand.palmNormal().y, hand.palmNormal().z);
	Vector3d directAct(hand.direction().x, hand.direction().y, hand.direction().z);

	data.posMoy = (numMoy*data.posMoy + (denMoy - numMoy)*posAct) / denMoy;
	data.normalMoy = (numMoy*data.normalMoy + (denMoy - numMoy)*normalAct) / denMoy;
	data.directMoy = (numMoy*data.directMoy + (denMoy - numMoy)*directAct) / denMoy;

#if !PARROT
	//std::cout << posAct << "    "<< posMoy << std::endl;
	//std::cout << "nbrAction" << nbrAction << "		nbrDoigt : " << nbrFingersAct << std::endl;
#endif
	if (data.nbrFingersAct != data.nbrFingersPred)
	{
		data.count = 1;
		data.nbrFingersPred = data.nbrFingersAct;
	}
	else
	{
		data.count++;
	}

	if (data.count == timeToStay)
	{
		data.count = 0;
		if (data.actionAct != data.nbrFingersAct)
			data.isFirstTimeNewAction = true;
		data.actionAct = data.nbrFingersAct;
	}
	else
	{
		data.isFirstTimeNewAction = false;
	}




	if ((clock() - data.timeOfOrigine) < TIME_OF_ANGLE_MODIFICATION)
	{
		data.origineDirect = (numMoy*data.origineDirect + (denMoy - numMoy)*data.directMoy) / denMoy;
		data.origineNormal = (numMoy*data.origineNormal + (denMoy - numMoy)*data.normalMoy) / denMoy;
		data.origine = (numMoy*data.origine + (denMoy - numMoy)*data.posMoy) / denMoy;
	}


	data.diffPos = (data.origine - data.posMoy);
	data.diffAngle.x() = atan2(data.directMoy.y(), -data.directMoy.z()) - atan2(data.origineDirect.y(), -data.origineDirect.z());
	data.diffAngle.y() = atan2(data.normalMoy.x(), -data.normalMoy.y()) - atan2(data.origineNormal.x(), -data.origineNormal.y());
	data.diffAngle.z() = atan2(data.directMoy.x(), -data.directMoy.z()) - atan2(data.origineDirect.x(), -data.origineDirect.z());


	if ((data.nbrFingersAct <= 2))
	{
		data.stop();
		data.origine = (numMoy*data.origine + (denMoy - numMoy)*data.posMoy) / denMoy;
		data.origineDirect = (numMoy*data.origineDirect + (denMoy - numMoy)*data.directMoy) / denMoy;
		data.origineNormal = (numMoy*data.origineNormal + (denMoy - numMoy)*data.normalMoy) / denMoy;
		data.timeOfOrigine = clock();
		//cout << "truc  " << clock() << endl;
	}

#if SAVE_DATA
	saveData(posAct, normalAct, directAct);
#endif



}


void SampleListener::stop()
{
	m_data.stop();
	m_dataS.stop();
}


const LeapDataToExchange* SampleListener::getDataA()
{
	return &m_dataToExchange;
}

const LeapDataToExchange* SampleListener::getDataB()
{
	return &m_dataToExchangeS;
}

int SampleListener::getNbrHand()
{
	return m_nbrHand;
}


#if SAVE_DATA
void SampleListener::saveData(Vector3d& posAct, Vector3d& normalAct, Vector3d& directAct)
{
	m_memoirePos.push_back(posAct);
	m_memoireOri.push_back(m_data.origine);
	m_memoireDiff.push_back(m_data.diffPos);
	m_memoireNorm.push_back(normalAct);
	m_memoireOriNorm.push_back(m_data.origineNormal);
	m_memoireDirect.push_back(directAct);
	m_memoireOriDirect.push_back(m_data.origineDirect);
	m_memoireDiffAngle.push_back(m_data.diffAngle);

	string toPrintInFile;
	if (m_memoirePos.size() >= 10)
	{
		for (int i = 0; i < m_memoirePos.size(); i++)
		{
			//toPrintInFile += m_memoirePos.at(i)/*.toString().erase(0, 1) */+ "\n";
			//toPrintInFile += m_memoireOri.at(i).toString().erase(0,1)		+ "\n";
			//toPrintInFile += m_memoireDiff.at(i).toString().erase(0,1)		+ "\n";
			//toPrintInFile += m_memoireAngles.at(i).toString().erase(0,1)	+ "\n";
			//toPrintInFile += m_memoireOriAngles.at(i).toString().erase(0,1) + "\n";
			//toPrintInFile += m_memoireDiffAngles.at(i).toString().erase(0,1)+ "\n";
			//toPrintInFile += "\n";
		}
		m_file << toPrintInFile;
	}


}
#endif

#if AFFICHAGE_3D


void SampleListener::affichage3d()
{
	int sizeWindows = 300;
	int facteur = 200;
	CvSize size;
	size.height = sizeWindows << 1;
	size.width = sizeWindows << 1;
	m_image = cv::Mat::zeros(size, CV_8UC3);


	if (m_nbrHand == 1)
	{
		affichage3dOneHand(m_data, sizeWindows, facteur);
	}
	else if (m_nbrHand == 2)
	{
		affichage3dOneHand(m_data , sizeWindows, facteur);
		affichage3dOneHand(m_dataS, sizeWindows, facteur);
	}

	cv::Point cvZeru;
	projPoint((Vector3d::Zero()), sizeWindows, cvZeru);
	cv::Point cvAxeX;
	projPoint((facteur*Vector3d(1, 0, 0)), sizeWindows, cvAxeX);
	cv::Point cvAxeY;
	projPoint((facteur*Vector3d(0, 1, 0)), sizeWindows, cvAxeY);
	cv::Point cvAxeZ;
	projPoint((facteur*Vector3d(0, 0, 1)), sizeWindows, cvAxeZ);

	cv::line(m_image, cvZeru, cvAxeX, cv::Scalar(255, 0, 0));
	cv::line(m_image, cvZeru, cvAxeY, cv::Scalar(0, 255, 0));
	cv::line(m_image, cvZeru, cvAxeZ, cv::Scalar(0, 0, 255));

	double fontScale = 0.5;
	int fonceface = cv::FONT_HERSHEY_DUPLEX;
	stringstream s1, s2, s3, s4;
	s1 << "Level : "<<m_eventTrad->getLevel();
	s2 << "Nombre de mains : " << m_nbrHand;
	s3 << "Nombre de doigt main 1 : " << m_data.nbrFingersAct;
	s4 << "Nombre de doigt main 2 : " << m_dataS.nbrFingersAct;

	string text = s1.str();
	cv::putText(m_image, text, cv::Point(10, 15), fonceface, fontScale, cv::Scalar::all(255), 1, 8);
	text = s2.str();
	cv::putText(m_image, text, cv::Point(10, 30), fonceface, fontScale, cv::Scalar::all(255), 1, 8);
	text = s3.str();
	cv::putText(m_image, text, cv::Point(10, 45), fonceface, fontScale, cv::Scalar::all(255), 1, 8);
	text = s4.str();
	cv::putText(m_image, text, cv::Point(10, 60), fonceface, fontScale, cv::Scalar::all(255), 1, 8);

	cv::imshow("leapMotion", m_image);
	cv::moveWindow("leapMotion", 700, 50);
	cvWaitKey(50);
}

void SampleListener::affichage3dOneHand(LeapData& data, int sizeWindows, int facteur)
{
	pthread_mutex_lock(m_mutex_prot_NF_CC);

	cv::Point cvOrg;
	projPoint(data.origine, sizeWindows, cvOrg);
	cv::Point cvOrgDir;
	projPoint((data.origine + (facteur*data.origineDirect)), sizeWindows, cvOrgDir);
	cv::Point cvOrgNor;
	projPoint((data.origine + (facteur*data.origineNormal)), sizeWindows, cvOrgNor);
	cv::Point cvPos;
	projPoint((data.posMoy), sizeWindows, cvPos);
	cv::Point cvPosDir;
	projPoint((data.posMoy + (facteur*data.directMoy)), sizeWindows, cvPosDir);
	cv::Point cvPosNor;
	projPoint((data.posMoy + (facteur*data.normalMoy)), sizeWindows, cvPosNor);

	pthread_mutex_unlock(m_mutex_prot_NF_CC);

	cv::line(m_image, cvOrg, cvOrgDir, cv::Scalar(255, 0, 0));
	cv::line(m_image, cvOrg, cvOrgNor, cv::Scalar(0, 255, 0));
	cv::line(m_image, cvPos, cvPosDir, cv::Scalar(0, 0, 255));
	cv::line(m_image, cvPos, cvPosNor, cv::Scalar(255, 0, 255));
}


void SampleListener::projPoint(const Vector3d& point, int sizeWindows, cv::Point& pointProj)
{

	int facteur = 300;

	Matrix3d base = Matrix3d::Zero();
	base(0, 0) = 1;
	base(1, 1) = -1 / sqrt(2);
	base(2, 1) = -1 / sqrt(2);
	base(1, 2) = 1 / sqrt(2);
	base(2, 2) = -1 / sqrt(2);

	Vector3d offset(0, -150, 0);
	offset += Vector3d(0, -200, -200);

	Vector3d pt = base*(point + offset);
	if (pt.z() != 0)
	{
		pointProj.x = sizeWindows + (int)(facteur*pt.x() / pt.z());
		pointProj.y = 3 * sizeWindows / 2 + (int)(facteur*pt.y() / pt.z());
	}
	else
	{
		pointProj.x = INT_MAX;
		pointProj.y = INT_MAX;
	}

}


#endif




