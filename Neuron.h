#include <vector>
#include <cstdlib>
#include <math.h>

using namespace std;

struct Connection
{
	double weight;
	double deltaWeight;
}; 

class Neuron;

typedef vector<Neuron> Layer;

class Neuron
{

public:
	Neuron(){};
	Neuron(unsigned numOutputs, unsigned myIndex);
	double getOutputVal();
	void setOutputVal(double val);
	int getIndex();
	vector<Connection> m_outputWeights;
	double randomValue();

private:
	double m_myIndex;
	double randomWeight();
	double m_outputVal;
};