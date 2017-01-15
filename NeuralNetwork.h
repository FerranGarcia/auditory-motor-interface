#include "Neuron.h"
#include "ParseFile.h"
#include <yarp/sig/all.h>
#include <yarp/sig/Vector.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/RateThread.h>

#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"

#include <iostream>
#include <fstream>
#include <list>
#include <map>

using namespace yarp::sig;
using namespace yarp::os;

class NeuralNetwork: public yarp::os::RateThread
{
	BufferedPort<Bottle> rpc;
	BufferedPort<Bottle> fromMCR;
	BufferedPort<Bottle> toMCR;
	BufferedPort<Bottle> fromMCL;
	BufferedPort<Bottle> toMCL;
	BufferedPort<Bottle> fromAC;

	bool isLearning;
	bool hands;
	Vector movementR;
	Vector movementL;

	vector<vector<double>> song;

public:
	NeuralNetwork(vector<unsigned> &topology, int _period = 500);
	void propagateToMap();
	void propagateToOutput();
	void propagateNet();

	void propagateNetReverse();
	void propagateToMapReverse();
	void propagateToInput();

	void setInput(vector<double> &input);
	void setOutput(vector<double> &output);
	vector<double> getOutput();
	vector<double> getInput();

	double getNeighbours(Neuron &neuron);
	void trainMap();
	void trainOutput();

	float sigma;
	float lambda;
	BufferedPort<ImageOf<PixelRgb>> portVisu;
	yarp::sig::ImageOf<yarp::sig::PixelRgb> mapActivityImage;

	Neuron winner;

	ofstream logIn;
	ofstream logOut;
	ofstream logError;
	
	ParseFile inputFile;
	ParseFile outputFile;

protected:
	virtual bool threadInit();
	virtual void run();
	virtual void threadRelease();

private:
	int input;  //number of inputs states
	int kohonen;
	int output;

	int rows;
	int cols;
	
	vector<Layer> m_layers; //m_layers[layerNum][neuronNum]
};
