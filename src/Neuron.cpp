#include "Neuron.h"
#include <iostream>
using namespace std;

Neuron::Neuron(unsigned numOutputs, unsigned myIndex)
{
	for(unsigned c = 0; c < numOutputs; ++c){
		m_outputWeights.push_back(Connection());
		m_outputWeights.back().weight = randomWeight();
	}

	m_myIndex = myIndex;
}

double Neuron::randomWeight()
{
	return rand() / double(RAND_MAX);
}

double Neuron::getOutputVal()
{
	return m_outputVal;
}

void Neuron::setOutputVal(double val)
{
	m_outputVal = val;
}

int Neuron::getIndex()
{
	return m_myIndex;
}

double Neuron::randomValue()
{
	return rand() / double(RAND_MAX);
}