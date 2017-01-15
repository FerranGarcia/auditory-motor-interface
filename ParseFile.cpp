#include "ParseFile.h"

ParseFile::ParseFile(){

	minus = 500;
	maxus = -80;
}

vector<string> &ParseFile::split(const string &s, char delim, vector<string> &elems)
{
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> ParseFile::split(const string &s, char delim)
{
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

vector<vector<double>> ParseFile::normalize(vector<vector<double>> data)
{
	for(unsigned int i = 0; i < data[0].size(); i++){
		for(unsigned int j = 0; j < data.size(); j++){
			if (this->minus > data[j][i]){
				this->minus = data[j][i];
			}
			if (this->maxus < data[j][i]){
				this->maxus = data[j][i];
			}
		}
		this->min.push_back(this->minus);
		this->max.push_back(this->maxus);
	}

	for(unsigned int i = 0; i < data[0].size(); i++){
		for(unsigned int j = 0; j < data.size(); j++){
			data[j][i] = ((data[j][i] - this->min[i]) / (abs(this->max[i] - this->min[i])));
		}
	}
	return data;
}

vector<double> ParseFile::unNormalize(vector<double> data)
{
	for(unsigned int i = 0; i < data.size(); i++){
			data[i] = ((abs(this->max[i] - this->min[i])) * data[i]) + this->min[i];
		}
	
	return data;
}

vector<vector<double>> ParseFile::LoadData(string fileName)
{
	vector<vector<double>> data;

	ifstream myfile;
	myfile.open(fileName);
	
	while (!myfile.eof())
	{
		string s;
		if (!getline(myfile, s)) break;

		vector <double> record;
		vector <string> recordStr = split(s,' ');
		for(int i=0; i< recordStr.size(); i++)
		{
			if(recordStr[i]!="")
			{
				double val = atof(recordStr[i].c_str());
				record.push_back(val);
			}
		}		
		data.push_back(record);
	}

	myfile.close();
	return data;
}
