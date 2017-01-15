#include "OscThread.h"

	OscThread::OscThread(int _port)
	{
		port = _port;
	}

	int OscThread::getLastPitch()
	{
		return lastPitch;
	}
	
	int OscThread::getCurrentPitch(double interval)
	{
		//std::cout<<"Now is : "<<t1<< " last pitch is "<<lastTime<<std::endl;
		if (yarp::os::Time::now() - lastTime> interval)
		{
			return -1;
		}
		return lastPitch;
	}

	bool OscThread::threadInit()
	{
		s = new UdpListeningReceiveSocket(
			IpEndpointName( IpEndpointName::ANY_ADDRESS, port ),
            this );
		lastPitch = 0;
		return true;
	}

	void OscThread::run()
	{
		while(!this->isStopping())
		{
			s->RunUntilSigInt();
		}
	}

	void OscThread::threadRelease()
	{
		s->AsynchronousBreak();
		delete s;
	}

	void OscThread::ProcessMessage( const osc::ReceivedMessage& m, 
				const IpEndpointName& remoteEndpoint )
    {
        (void) remoteEndpoint; // suppress unused parameter warning

        try{
            
            if( std::strcmp( m.AddressPattern(), "/pitch" ) == 0 ){
       
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
                osc::int32 a2;
				args >> a2 >> osc::EndMessage;
                
                lastPitch = a2;
				lastTime = yarp::os::Time::now();

            }else if( std::strcmp( m.AddressPattern(), "/test2" ) == 0 ){

                osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
                bool a1 = (arg++)->AsBool();
                int a2 = (arg++)->AsInt32();
                float a3 = (arg++)->AsFloat();
                const char *a4 = (arg++)->AsString();
                if( arg != m.ArgumentsEnd() )
                    throw osc::ExcessArgumentException();
            }
        }catch( osc::Exception& e ){

            std::cout << "error while parsing message: "
                << m.AddressPattern() << ": " << e.what() << "\n";
        }
    }