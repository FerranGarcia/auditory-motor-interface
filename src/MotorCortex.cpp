#include "MotorCortex.h"

MotorCortex::MotorCortex(string _robotName, string _arm, int _period):RateThread(_period)
{
	robotName = _robotName;
	arm = _arm;

	string remotePorts="/";
	remotePorts+=robotName;
	remotePorts+="/";
	remotePorts+=arm;
 
	string localPorts="/motorCortex/icub/";
	localPorts += arm;				  
 
	Property options;
	options.put("device", "remote_controlboard");
	options.put("local", localPorts.c_str());			//local port names
	options.put("remote", remotePorts.c_str());         //where we connect to
 
	pd = new PolyDriver(options);

	if (!pd->isValid()) {
        printf("Device not available\n");
    }
}

bool MotorCortex::threadInit()
{
	currentPos.resize(16);
	currentPos = 0;
	position.resize(16);

	press = false;

	//Name of the ports
	string rootName = "/motorCortex/";
	rootName += arm;
	rootName += "/encs:i";
	fromNN.open(rootName.c_str());
	
	rootName = "/motorCortex/";
	rootName += arm;
	rootName += "/encs:o";
	toNN.open(rootName.c_str());
		
	// create a device
	if (!pd->isValid()) {
		printf("Device not available. Here are the known devices:\n");
		printf("%s", Drivers::factory().toString().c_str());
		return false;
	}

	bool ok;
	ok = pd->view(pos);
	ok &= pd->view(encs);
 
	if (!ok) {
		printf("Problems acquiring interfaces\n");
		return false;
	}
	for(int i=0; i<7;i++)
	{
		pos->setRefAcceleration(i,50.0);
		pos->setRefSpeed(i,40.0);
	}
	for(int i=7; i<16;i++)
	{
		pos->setRefAcceleration(i,100.0);
		pos->setRefSpeed(i,90.0);
	}
	return true;		
}

void MotorCortex::run()
{	
	////Send the encoders
	//encs->getEncoders(currentPos.data());
	//Bottle& outBot = toNN.prepare();   // Get the object
	//outBot.clear();
	//			
	//for (unsigned int i = 0; i<currentPos.size(); i++){				  
	//	outBot.addDouble(currentPos[i]);
	//}

	////cout<<"MC: Sending "<<outBot.toString()<<endl;
	//toNN.write();

	//Read from the NN
	Bottle *BotNNCmd = fromNN.read(false); 
	if (BotNNCmd != NULL)
	{
		cout<<"MC "<<arm<<": Catched command from NN"<<endl;

		for (int i = 0; i<BotNNCmd->size(); i++){
			double result = BotNNCmd->get(i).asDouble();
			position[i] = result;
			cout<<"Moving "<<arm<<": "<<position[i]<<endl;
		}

		if ((position[4] < 0.1) && (position[4] > -0.1)){
			press = false;
		}else{
				press = true;
		}
							
		pos->positionMove(position.data());
						
		bool done=false;
		while(!done)
		{
			pos->checkMotionDone(&done);
			Time::delay(0.1);
		}
		
		if(press){
			//cout<<"I am in!"<<endl;
			Vector tmp = position;
			tmp[5] = 0.0;
			pos->positionMove(tmp.data());

			bool move = false;
			while(!move)
			{
				pos->checkMotionDone(&move);
				Time::delay(0.1);
			}

			tmp[5] = -30.0;
			pos->positionMove(tmp.data());
			bool movex = false;

			while(!movex)
			{
				pos->checkMotionDone(&movex);
				Time::delay(0.1);
			}
			press = false;
		}

		Bottle& NNcmd = toNN.prepare();
		NNcmd.clear();
		NNcmd.addString("play");
		toNN.write();
		cout<<"MC "<<arm<<": Confirmation sended"<<endl;
	}
}


void MotorCortex::threadRelease()
{
	pd->close();
	delete pd;
	fromNN.close();
	toNN.close();
	cout<<"MC: Closing..."<<endl;
}