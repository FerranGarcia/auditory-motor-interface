#include <vector>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

class ParseFile{

public:
	ParseFile();
	vector<vector<double> > LoadData(string fileName);
	vector<vector<double> > normalize(vector<vector<double> > data);
	vector<double> unNormalize(vector<double> data);
	
	vector<double> min;
	vector<double> max;
	double minus;
	double maxus;

private:
	vector<string> &split(const string &s, char delim, vector<string> &elems);
	vector<string> split(const string &s, char delim);

};