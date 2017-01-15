#include <yarp/os/Time.h>
#include <yarp/os/RateThread.h>
#include <yarp/sig/Vector.h>
#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/os/Network.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/os/BufferedPort.h>

#include <vector>
#include <iostream>
#include <string>

using namespace yarp::sig;
using namespace yarp::dev;
using namespace yarp::os;
using namespace std;

class MotorCortex: public yarp::os::RateThread
{
	PolyDriver *pd;
	IPositionControl *pos;
	IEncoders *encs;
	Vector currentPos;

	vector<Vector> buffer;
	Vector position;
	
	string robotName;
	string arm;

	bool press;
	
	//Buffered port
	BufferedPort<Bottle> fromNN;
	BufferedPort<Bottle> toNN;

public:

	MotorCortex(string _robotName, string _arm, int _period = 500);

protected:
	virtual bool threadInit();
	virtual void run();
	virtual void threadRelease();
};