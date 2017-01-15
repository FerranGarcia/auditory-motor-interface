#include <time.h>

#include "AuditoryCortex.h"
#include "MotorCortex.h"
#include "NeuralNetwork.h"

int main(int argc, char *argv[]) 
{
	srand(time(NULL));
	
	Network yarp;
	int globalPeriod = 500;

	AuditoryCortex ac(globalPeriod);
	ac.start();
	
	//necessary for compiling in the real robot
	MotorCortex mcR("icub", "right_arm", globalPeriod);
	mcR.start();
	
	MotorCortex mcL("icub", "left_arm", globalPeriod);
	mcL.start();

	int rows = 10;
	int cols = 10;
	vector<unsigned> topology;
	
	topology.push_back(32);						//number of output units (16 joints + arm_id)		
	topology.push_back(rows*cols);
	topology.push_back(1);						//number of input states (pitch)
	topology.push_back(rows);					//rows
	topology.push_back(cols);					//columns

	NeuralNetwork nn(topology, globalPeriod);	//It creates the network and initialize the weights	
	nn.start();
	cout<<"The NN has been created!"<<endl;
	cout<<endl;
	
	while(
	//AC to NN
		!Network::connect("/auditoryCortex/pitch:o","/dorsalPathway/pitch:i") ||
	//MCL to NN
		!Network::connect("/motorCortex/left_arm/encs:o","/dorsalPathwayLeft/encs:i") ||
	//NN to MCL
		!Network::connect("/dorsalPathwayLeft/encs:o","/motorCortex/left_arm/encs:i") ||
	//MCR to NN
		!Network::connect("/motorCortex/right_arm/encs:o","/dorsalPathwayRight/encs:i") ||
	//NN to MCR
		!Network::connect("/dorsalPathwayRight/encs:o","/motorCortex/right_arm/encs:i"))
	{ cout<<"Trying to connect..."<<endl; Time::delay(1.0);}

	string quit = "";
	while (quit != "quit")
	{
		cin>>quit;
	}

	mcR.stop();
	mcL.stop();
	ac.stop();
	nn.stop();
    
    return 0;
}