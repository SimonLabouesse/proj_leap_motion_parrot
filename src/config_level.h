#ifndef FI_CONFIG_LEVEL
#define FI_CONFIG_LEVEL


#define NBR_LEVEL			  3

//configuration des differents niveaux

#define DEP_MIN_MAG_L1		 55
#define DEP_MIN_DIR_L1		 40 // DEP_MIN_DIR*sqrt(2) = DEP_MIN_RAD
#define ANG_MIN_L1			 20 
#define FACT_VITESSE_L1		  0.6 // pourcent de 1m/s
#define FACT_ANGLE_L1		  0.5	 // pourcent de 1m/s

#define DEP_MIN_MAG_L2		 70
#define DEP_MIN_DIR_L2		 50  // DEP_MIN_DIR*sqrt(2) = DEP_MIN_RAD
#define ANG_MIN_L2			 20 
#define FACT_VITESSE_L2		  0.6 // pourcent de 1m/s
#define FACT_ANGLE_L2		  0.5	 // pourcent de 1m/s

#define DEP_MIN_MAG_L3		 55
#define DEP_MIN_DIR_L3		 40  // DEP_MIN_DIR*sqrt(2) = DEP_MIN_RAD
#define ANG_MIN_L3			 10 
#define FACT_VITESSE_L3		  0.0333  // pourcent de 1m/s
#define FACT_ANGLE_L3		  8.4	  // pourcent de 1m/s



#endif
