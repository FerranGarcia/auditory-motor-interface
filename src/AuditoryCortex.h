#include <OscThread.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/BufferedPort.h>

class AuditoryCortex: public yarp::os::RateThread
{
	OscThread * osc;

public:
	
	AuditoryCortex(int _period = 500);
	
	//Buffered port
	BufferedPort<Bottle> outPitch;

protected:

	virtual bool threadInit();
	virtual void run();
	virtual void threadRelease();

};