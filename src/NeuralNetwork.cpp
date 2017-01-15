#include "NeuralNetwork.h"

using namespace std;

NeuralNetwork::NeuralNetwork(vector<unsigned> &topology, int _period):RateThread(_period)
{
	input	= topology[0];
	kohonen = topology[1];
	output  = topology[2];
	rows    = topology[3];
	cols    = topology[4];

	lambda = 0.5;
	sigma = 0.85;

	for (unsigned layerNum = 0; layerNum < 3; layerNum++){
		m_layers.push_back(Layer());

		unsigned numOutputs;
		if (layerNum == 2) numOutputs = 0;
		else numOutputs = topology[layerNum + 1];

		for (unsigned neuronNum = 0; neuronNum < topology[layerNum]; ++neuronNum){
			m_layers.back().push_back(Neuron(numOutputs, neuronNum));		
		}
	}
	this->mapActivityImage.resize(cols,rows);
}

bool NeuralNetwork::threadInit(){

	fromMCR.open("/dorsalPathwayRight/encs:i");
	toMCR.open("/dorsalPathwayRight/encs:o");
	fromMCL.open("/dorsalPathwayLeft/encs:i");
	toMCL.open("/dorsalPathwayLeft/encs:o");
	fromAC.open("/dorsalPathway/pitch:i");

	portVisu.open("/neuralNetwork/img:o");
	Network::connect("/neuralNetwork/img:o", "/v");

	rpc.open("/neuralNetwork/rpc");
	movementL.resize(16);
	movementR.resize(16);
	isLearning = true;
	hands = false;

	logIn.open("logIn.txt",ios::app);
	logOut.open("logOut.txt",ios::app);
	logError.open("logError.txt");

	return true;
}

void NeuralNetwork::run(){

	//Process inputs
	Bottle *BotAC = fromAC.read(false);
	Bottle *BotMCR = fromMCR.read(false);
	Bottle& outBotR = toMCR.prepare();
	Bottle *BotMCL = fromMCL.read(false);
	Bottle& outBotL = toMCL.prepare();

	Bottle *MCLrpc;
	Bottle *MCRrpc;
	
	Bottle *botRPC = rpc.read(false);
	string rpccmd = "nada";

	if (botRPC!=NULL)
	{
		rpccmd = botRPC->get(0).asString().c_str();
		cout<<"NN Got a RPC command: "<<rpccmd<<endl;
		if(rpccmd == "start")
		{
			isLearning = false;

			//Parsing all data from input and output files
			
			vector<vector<double>> dataInput = inputFile.LoadData("logIn.txt");
			dataInput = inputFile.normalize(dataInput);

			vector<vector<double>> dataOutput = outputFile.LoadData("logOut.txt");
			dataOutput = outputFile.normalize(dataOutput);

			cout<<"All the dataset has been readed!"<<endl;
			cout<<endl;

			for (unsigned int epoch = 0; epoch < 150; epoch++){
		
				double epochError = 0;
				double maxError = 0;
				int neuron;
		
				for (unsigned int j = 0; j < dataInput.size(); j++){
		
					//cout<<"Epoch: "<<epoch<<" Step: "<<j+1<<endl;

					this->setInput(dataInput[j]);		//It inicializes the values of the neurons
					this->propagateNet();				//Propagate signals in both Layers: Kohonen Map and output
					this->trainMap();					//Train the Kohonen Map Layer
			
					//Write through yarp
					ImageOf<PixelRgb> &img = portVisu.prepare();
					img.copy(this->mapActivityImage);
					portVisu.write();

					//Write the prediction			
					for(unsigned i = 0; i < this->getOutput().size(); i++){
						//cout<<"Prediction: "<<this->getOutput()[i]<<" Desired value: "<<dataOutput[j][0]<<endl;
						double stepError = abs(dataOutput[j][0] - this->getOutput()[i]);
						//cout<<"Step error: "<<stepError<<endl;
						epochError += stepError;
				
						if (maxError < stepError){
							maxError = stepError;
							neuron = this->winner.getIndex();
						}
					}
					//cout<<endl;
			
					this->setOutput(dataOutput[j]);		//Learn desired value
					this->trainOutput();				//Train the output Layer
				}

				//cout<<"Maximum error: "<<maxError<<" Neuron: "<<neuron<<endl;
				epochError = epochError / dataInput.size();
				//cout<<"Epoch: "<<epoch<<" Average error: "<<epochError<<endl;
				logError<<epochError<<endl;
				//cout<<endl;
			}
		}
	}

	//Listen to the auditory cortex
	if (BotAC != NULL){
		int currentPitch = BotAC->get(0).asInt();
	
		if(isLearning){

			for (int i = 0; i<BotMCR->size(); i++){
				double result = BotMCR->get(i).asDouble();
				movementR[i] = result;
			}

			for (int i = 0; i<BotMCL->size(); i++){
				double result = BotMCL->get(i).asDouble();
				movementL[i] = result;
			}
			
			//put the pitch into the output file
			cout<<"Recorded pitch is: "<<currentPitch<<endl;
			logOut<<currentPitch<<endl;
			//put the joint into the input file
			cout<<"Recorded values for RIGHT arm are: "<<movementR.toString(3,3)<<endl;
			cout<<"Recorded values for LEFT arm are: "<<movementL.toString(3,3)<<endl;
			logIn<<movementR.toString(3,3).c_str()<<" "<<movementL.toString(3,3).c_str()<<endl;
	
		}

		//Press the note, get the arm position and store it
		if (!isLearning){	
			
			vector<vector<double>> pitch;
			vector<double> current;
			current.push_back(currentPitch);
			pitch.push_back(current);

			pitch = outputFile.normalize(pitch);

			vector<double> position;

			//Present the pitch
			this->setOutput(pitch[0]);
			this->propagateNetReverse();
			
			//Get the value of the winner
			for(unsigned i = 0; i < this->getInput().size(); i++){
				position.push_back(this->getInput()[i]);
			}
			
			position = inputFile.unNormalize(position);

			vector<Neuron> neurons;
			neurons.push_back(this->winner);

			song.push_back(position);
		}
	}

	if(rpccmd == "play"){

		for(unsigned int i = 0; i<song.size(); i++){
			vector<double> position;
			position = song[i];
			string mcl = "wait";
			string mcr = "wait";
			
			//Sent the movement to the MCR
			Bottle& motorCmdR = toMCR.prepare();
			motorCmdR.clear();		

			for (unsigned int i = 0; i<16; i++){				
				motorCmdR.addDouble(position[i]);
				cout<<"Sending to MCR: "<<position[i]<<endl;
			}

			//Sent the movement to the MCL
			Bottle& motorCmdL = toMCL.prepare();
			motorCmdL.clear();		

			for (unsigned int i = 16; i<position.size(); i++){				
				motorCmdL.addDouble(position[i]);
				cout<<"Sending to MCL: "<<position[i]<<endl;
			}
			toMCR.write();
			toMCL.write();
			
			cout<<"NN: Waiting confirmation..."<<endl;
			while((mcl != "play") || (mcr != "play")){
				//internal commands of motion done comming from both cortex
				MCLrpc = fromMCL.read(false);
				MCRrpc = fromMCR.read(false);
				if (MCLrpc != NULL) mcl = MCLrpc->get(0).asString().c_str();
				if (MCRrpc != NULL) mcr = MCRrpc->get(0).asString().c_str();
			}
			cout<<"NN: Next Movement"<<endl;
		}
	}

	if(rpccmd == "artist") hands = true;
	if(rpccmd == "stop") hands = false;

	if (hands){
		vector<vector<double>> position;
		vector<double> data;
		
		for (int i = 0; i<BotMCR->size(); i++){
			double result = BotMCR->get(i).asDouble();
			data.push_back(result);
			cout<<"Right encoders: "<<data[i]<<endl;
		}

		for (int i = 0; i<BotMCL->size(); i++){
			double result = BotMCL->get(i).asDouble();
			data.push_back(result);
			cout<<"Left encoders: "<<data[i+16]<<endl;
		}

		position.push_back(data);		
		
		position = inputFile.normalize(position);

		this->setInput(position[0]);		
		this->propagateNet();

		vector<double> getPitch = this->getOutput();
		getPitch = outputFile.unNormalize(getPitch);
		cout<<"Pitch: "<<getPitch[0]<<endl;
		
		//sending pitch to PD
		UdpTransmitSocket transmitSocket( IpEndpointName( "127.0.0.1", 7001 ) );
		char buffer[1024];
		osc::OutboundPacketStream p( buffer, 1024 );
    
		p << osc::BeginBundleImmediate
        << osc::BeginMessage( "/pitch" ) 
            << (float)getPitch[0] << osc::EndMessage
        << osc::EndBundle;
    
		transmitSocket.Send( p.Data(), p.Size() );
	}

	if(rpccmd == "clear"){
		cout<<"Reseting buffer"<<endl;	
		song.clear();
	}

	if(rpccmd == "calibrate"){	
		//cout<<"NN: Sending calibrating order"<<endl;
		//outBotL.clear();
		//outBotL.addString("calibrate");
		//outBotR.clear();
		//outBotR.addString("calibrate");
		//toMCL.write();
		//toMCR.write();

		vector<vector<double>> pitch;
		vector<double> tmp;
		tmp.push_back(84);
		pitch.push_back(tmp);

		pitch = outputFile.normalize(pitch);
		vector<double> position;

		//Present the pitch
		this->setOutput(pitch[0]);
		this->propagateNetReverse();
			
		//Get the value of the winner
		for(unsigned i = 0; i < this->getInput().size(); i++){
			position.push_back(this->getInput()[i]);
		}
			
		position = inputFile.unNormalize(position);

		//Sent the movement to the MCR
		Bottle& motorCmdR = toMCR.prepare();
		motorCmdR.clear();		

		for (unsigned int i = 0; i<16; i++){				
			motorCmdR.addDouble(position[i]);
			cout<<"Sending to MCR: "<<position[i]<<endl;
		}
		toMCR.write();
		
		position.clear();
		tmp.clear();
		pitch.clear();
		tmp.push_back(53);
		pitch.push_back(tmp);

		pitch = outputFile.normalize(pitch);

		//Present the pitch
		this->setOutput(pitch[0]);
		this->propagateNetReverse();
			
		//Get the value of the winner
		for(unsigned i = 0; i < this->getInput().size(); i++){
			position.push_back(this->getInput()[i]);
		}
			
		position = inputFile.unNormalize(position);

		//Sent the movement to the MCL
		Bottle& motorCmdL = toMCL.prepare();
		motorCmdL.clear();		

		for (unsigned int i = 16; i<position.size(); i++){				
			motorCmdL.addDouble(position[i]);
			cout<<"Sending to MCL: "<<position[i]<<endl;
		}
		toMCL.write();
	}
}

/*******************************PROPAGATING SIGNALS*********************************/

void NeuralNetwork::propagateToMap()
{
	Layer &inputLayer = m_layers[0];
	Layer &koLayer = m_layers[1];

	double out, weight, sum, min;

	for (unsigned numCon = 0; numCon < koLayer.size(); numCon++){
		sum = 0;
		for (unsigned numNeuron = 0; numNeuron < inputLayer.size(); numNeuron++){
			out = inputLayer[numNeuron].getOutputVal();
			weight = inputLayer[numNeuron].m_outputWeights[numCon].weight;
			sum += pow((out - weight),2);
		}
		koLayer[numCon].setOutputVal(sqrt(sum));

		int position = koLayer[numCon].getIndex();
		int x = position / cols;
		int y = position % cols;

		this->mapActivityImage.pixel(x,y).g = 0;
		this->mapActivityImage.pixel(x,y).b = 0;
		this->mapActivityImage.pixel(x,y).r = 255*koLayer[numCon].getOutputVal();
	}

	unsigned int i;
	this->winner = koLayer[0];
	for (i = 1; i < koLayer.size(); i++) {
		if (koLayer[i].getOutputVal() < this->winner.getOutputVal())
			this->winner = koLayer[i];
	}
	//cout<<"Winner neuron is "<<this->winner.getIndex()<<" with activity "<<this->winner.getOutputVal()<<endl;
}

void NeuralNetwork::propagateToOutput()
{  
	Layer &outputLayer = m_layers[2];
	for (unsigned i = 0; i < outputLayer.size(); i++) {
		outputLayer[i].setOutputVal(this->winner.m_outputWeights[i].weight);
  }
}

void NeuralNetwork::propagateNet()
{
	propagateToMap();
	propagateToOutput();
}

void NeuralNetwork::propagateToMapReverse()
{
	Layer &outputLayer = m_layers[2];
	Layer &koLayer = m_layers[1];

	double out, weight, sum, min;

	for (unsigned numCon = 0; numCon < koLayer.size(); numCon++){
		sum = 0;
		for (unsigned numNeuron = 0; numNeuron < outputLayer.size(); numNeuron++){
			out = outputLayer[numNeuron].getOutputVal();
			weight = koLayer[numCon].m_outputWeights[numNeuron].weight;
			sum += pow((out - weight),2);
		}
		koLayer[numCon].setOutputVal(sqrt(sum));

		int position = koLayer[numCon].getIndex();
		int x = position / cols;
		int y = position % cols;

		this->mapActivityImage.pixel(x,y).g = 0;
		this->mapActivityImage.pixel(x,y).b = 0;
		this->mapActivityImage.pixel(x,y).r = 255*koLayer[numCon].getOutputVal();
	}

	unsigned int i;
	this->winner = koLayer[0];
	for (i = 1; i < koLayer.size(); i++) {
		if (koLayer[i].getOutputVal() < this->winner.getOutputVal())
			this->winner = koLayer[i];
	}
	//cout<<"Winner neuron is "<<this->winner.getIndex()<<" with activity "<<this->winner.getOutputVal()<<endl;
}

void NeuralNetwork::propagateToInput()
{  
	Layer &inputLayer = m_layers[0];
	for (unsigned i = 0; i < inputLayer.size(); i++) {
		inputLayer[i].setOutputVal(inputLayer[i].m_outputWeights[this->winner.getIndex()].weight);
  }
}

void NeuralNetwork::propagateNetReverse()
{
	propagateToMapReverse();
	propagateToInput();
}

/*******************************TRAINING THE NET*********************************/

double NeuralNetwork::getNeighbours(Neuron &neuron)
{
	float  iRow, iCol, jRow, jCol;
	int position = neuron.getIndex();

	iRow = position / cols;
	iRow = (int)iRow;
	jRow = this->winner.getIndex() / cols;
	jRow = (int)jRow;
	iCol = position % cols; 
	jCol = this->winner.getIndex() % cols;

	double distance = sqrt(pow((iRow - jRow),2) + pow((iCol - jCol),2));
	return exp(-pow(distance,2) / (2*pow(this->sigma,2)));
}

void NeuralNetwork::trainMap()
{
	double out, weight, alpha;
	Layer &inputLayer = m_layers[0];
	Layer &koLayer = m_layers[1];

	for (unsigned i = 0; i < koLayer.size(); i++){
		for (unsigned j = 0; j < inputLayer.size(); j++){
			out = inputLayer[j].getOutputVal();
			weight = inputLayer[j].m_outputWeights[i].weight;
			alpha = getNeighbours(koLayer[i]);
			//take all the connections of the inputLayer to the same neuron
			inputLayer[j].m_outputWeights[i].weight += this->lambda * alpha * (out - weight); 
		}
	}
}

void NeuralNetwork::trainOutput()
{
	double out, weight, alpha;
	Layer &koLayer = m_layers[1];
	Layer &outputLayer = m_layers[2];

	for (unsigned i = 0; i < outputLayer.size(); i++){
		for (unsigned j = 0; j < koLayer.size(); j++){
			out = outputLayer[i].getOutputVal();
			weight = koLayer[j].m_outputWeights[i].weight;
			alpha = getNeighbours(koLayer[j]);
			koLayer[j].m_outputWeights[i].weight += this->lambda * alpha * (out - weight); 
		}
	}
}

/**************************GETTERS AND SETTERS*****************************/

void NeuralNetwork::setInput(vector<double> &input)
{
	Layer &inputLayer = m_layers[0];
	for (unsigned i = 0; i < inputLayer.size(); i++) {
		inputLayer[i].setOutputVal(input[i]);
	}
}

void NeuralNetwork::setOutput(vector<double> &output)
{
	Layer &outputLayer = m_layers[2];
	for (unsigned i = 0; i < outputLayer.size(); i++) {
		outputLayer[i].setOutputVal(output[i]);
	}
}

vector<double> NeuralNetwork::getOutput()
{
	vector<double> output;
	Layer &outputLayer = m_layers[2];
	for (unsigned i = 0; i < outputLayer.size(); i++) {
		output.push_back(outputLayer[i].getOutputVal());
	} 
	return output;
}

vector<double> NeuralNetwork::getInput()
{
	vector<double> input;
	Layer &inputLayer = m_layers[0];
	for (unsigned i = 0; i < inputLayer.size(); i++) {
		input.push_back(inputLayer[i].getOutputVal());
	} 
	return input;
}		

void NeuralNetwork::threadRelease(){

	toMCL.close();
	fromMCL.close();
	toMCR.close();
	fromMCR.close();
	fromAC.close();
	cout<<"NN: Closing..."<<endl;
}

