#include "AuditoryCortex.h"

AuditoryCortex::AuditoryCortex(int _period):RateThread(_period)
{
}

bool AuditoryCortex::threadInit()
{
	cout<<"Opening auditory cortex"<<endl;
	osc = new OscThread();
	osc->start();
	outPitch.open("/auditoryCortex/pitch:o");
	cout<<"AC: Listening..."<<endl;
	return true;
}

void AuditoryCortex::run()
{
	{
		int currentPitch = osc->getCurrentPitch(0.5);
		if (currentPitch != -1)
		{
			Bottle& outKey = outPitch.prepare();
			outKey.clear();
			outKey.addInt(currentPitch);
			outPitch.write();

			cout<<"AC: Sending "<<currentPitch<<endl;
		}
		//else
		//	cout<<"...Waiting for sound..."<<endl;
	}
}

void AuditoryCortex::threadRelease()
{
	osc->stop();
	cout<<"AC: Closing..."<<endl;
	delete osc;
}