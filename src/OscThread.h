#include <osc/OscReceivedElements.h>
#include <osc/OscPacketListener.h>
#include <ip/UdpSocket.h>
#include <yarp/os/Time.h>
#include <yarp/os/Thread.h>

#include <iostream>

using namespace std;
using namespace yarp::os;

class OscThread: public yarp::os::Thread, public osc::OscPacketListener
{
	int port;
	UdpListeningReceiveSocket *s;
	int lastPitch;
	double lastTime;

public:

	OscThread(int _port = 7000);
	int getLastPitch();	
	int getCurrentPitch(double interval = 1.0);

protected:
	virtual bool threadInit();
	virtual void run();
	virtual void threadRelease();
	virtual void ProcessMessage(const osc::ReceivedMessage& m, 
				const IpEndpointName& remoteEndpoint);

};