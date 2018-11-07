#include <iostream>
#include <string>
#include <fstream>
#include <math.h>

using namespace std;


class Point
{
	public:
		int Xcoord;
		int Ycoord;
		int Label;
		int Distance;

		Point();
		Point(int, int);
};

Point :: Point()
{
	Xcoord = -1;
	Ycoord = -1;
	Label = -1;
	Distance = 99999;
}

Point :: Point(int x, int y)
{
	Xcoord = x;
	Ycoord = y;
	Label = -1;
	Distance = 99999;
}


// Used for central points of labels
class xyCoord
{
	public:
		int Xcoord;
		int Ycoord;
		int Label;

		xyCoord();
		xyCoord(int, int);
		xyCoord(int, int, int);
};

xyCoord :: xyCoord()
{
	Xcoord = -1;
	Ycoord = -1;
	Label = -1;
}

xyCoord :: xyCoord(int x, int y)
{
	Xcoord = x;
	Ycoord = y;
	Label = -1;
}

xyCoord :: xyCoord(int x, int y, int l)
{
	Xcoord = x;
	Ycoord = y;
	Label = l;
}


void print_append(string content, string input_file_name)
{
	ofstream outFile(input_file_name, fstream::app);
	outFile << content;
	outFile.close();
	return;
}


void file_cleaner(string file_name)
{
	ofstream outFile(file_name, ifstream::trunc);
	outFile.close();
}

class Kmean
{
	public:
		// Source Data Relative 
		int numRows;
		int numCols;
		int minVal;
		int maxVal;
		int numPts;
		// K-mean Relative
		int K;
		int change;
		int** imgAry;
		Point* pointSet;
		xyCoord* Kcentroids;

		// Constructor
		Kmean(int);

		/**** Methods ****/
		// Reading points from txt file
		// Finding out the number of points
		void extractPts(string, string);
		// Loading points
		void loadPointSet(string);
		// Computing distance between two points(different classes)
		int computeDist(Point, xyCoord);
		// Renewing the central points
		void computeCentroids();
		// Finding the shortest distance and renewing label
		// Variable "change" will be modified in this method
		void DistanceMinLabel(int);
		// Used once, to assign the label
		void assignLabel();
		// Main method to clustering
		void kMeansClustering(string);

		void writePtSet(string);
		void Point2Image();
		void printImage(string, int);
};

Kmean :: Kmean(int in_K)
{
	numRows = -1;
	numCols = -1;
	minVal = -1;
	maxVal = -1;
	numPts = 0;
	change = 999;
	K = in_K;
	Kcentroids = new xyCoord[K+1];
}

void Kmean :: extractPts(string input_file_name, string output_file_name)
{
	int pointCounter = 0;
	int number;
	string stringNumber = "";
	string line = "";

	ifstream inFile;
	inFile.open(input_file_name);

	// Reading the header information
	inFile >> stringNumber;
	this->numRows = atoi(stringNumber.c_str());
	inFile >> stringNumber;
	this->numCols = atoi(stringNumber.c_str());
	inFile >> stringNumber;
	this->minVal = atoi(stringNumber.c_str());
	inFile >> stringNumber;
	this->maxVal = atoi(stringNumber.c_str());

	for(int i = 0; i < this->numRows; i++)
		for(int j = 0; j < this->numCols; j++)
		{
			inFile >> stringNumber;
			number = atoi(stringNumber.c_str());
			if(number == 1)
			{
				pointCounter++;
				line += std::to_string(i);
				line += " ";
				line += std::to_string(j);
				line += "\r\n";
				print_append(line, output_file_name);
				line = "";
			}
			else
				continue;
		}
	// Renewing numPts and pointSet array since the 
	// number of points is known.
	// Renewing imgAry since the rows and cols are known
	this->numPts = pointCounter;
	this->pointSet = new Point[pointCounter];
	this->imgAry = new int*[this->numRows];
	for(int r = 0; r < this->numRows; r++)
		this->imgAry[r] = new int[this->numCols];
		
	// Initialzing imgAry
	for(int r = 0; r < this->numRows; r++)
		for(int c = 0; c < this->numCols; c++)
			this->imgAry[r][c] = 0;
	inFile.close();

	return;
}

void Kmean :: assignLabel() 
{
	int front = 0;
	int back = this->numPts - 1;
	int label = 1;

	while(front <= back)
	{
		if(label > this->K)
			label = 1;
		this->pointSet[front].Label = label;
		front++;
		label++;
		this->pointSet[back].Label = label;
		back--;
		label++;
	}

	return;
}

void Kmean :: loadPointSet(string input_file_name)
{
	ifstream inFile;
	inFile.open(input_file_name);

	for(int i = 0; i < this->numPts; i++)
	{
		inFile >> this->pointSet[i].Xcoord;
		inFile >> this->pointSet[i].Ycoord;
	}
	inFile.close();

	return;
}

void Kmean :: writePtSet(string output_file_name)
{
	string line = "";
	// Number of points
	line += std::to_string(this->numPts);
	line += "\r\n";
	// Row x Column
	line += std::to_string(this->numRows);
	line += " ";
	line += std::to_string(this->numCols);
	line += "\r\n";

	for(int i = 0; i < this->numPts; i++)
	{
		line += std::to_string(this->pointSet[i].Xcoord);
		line += " ";
		line += std::to_string(this->pointSet[i].Ycoord);
		line += " ";
		line += std::to_string(this->pointSet[i].Label);
		line += "\r\n";
	}
	print_append(line, output_file_name);

	return;
}

void Kmean :: computeCentroids()
{
	cout << endl;
	int sumX[this->K + 1];
	int sumY[this->K + 1];
	int totalPt[this->K + 1];
	for(int i = 1; i <= K; i++)
	{
		sumX[i] = 0;
		sumY[i] = 0;
		totalPt[i] = 0;
	}
	for(int i = 0; i < this->numPts; i++)
	{
		int label = this->pointSet[i].Label;
		sumX[label] += this->pointSet[i].Xcoord;
		sumY[label] += this->pointSet[i].Ycoord;
		totalPt[label]++;
	}
	for(int i = 1; i <= this->K; i++)
	{
		float x = (float)sumX[i];
		float y = (float)sumY[i];
		float p = (float)totalPt[i];
		float mid_x;
		float mid_y;
		if(p == 0)
		{
			mid_x = 0;
			mid_y = 0;
		}
		else
		{
			mid_x = x / p;
			mid_y = y / p;
		}
		this->Kcentroids[i].Xcoord = (int)mid_x;
		this->Kcentroids[i].Ycoord = (int)mid_y;
	}
	
	return;
}

void Kmean :: Point2Image()
{
	int r = 0;
	int c = 0;

	for(int i = 0; i < this->numPts; i++)
	{
		r = this->pointSet[i].Xcoord;
		c = this->pointSet[i].Ycoord;
		this->imgAry[r][c] = this->pointSet[i].Label;
	}

	return;
}

void Kmean :: printImage(string output_file_name, int iteration)
{
	string line = "";

	line += "**** Result of iteration ";
	line += std::to_string(iteration);
	line += " ****\r\n";

	for(int r = 0; r < this->numRows; r++)
	{
		for(int c = 0; c < this->numCols; c++)
		{
			if(this->imgAry[r][c] == 0)
				line += " ";
			else
				line += std::to_string(this->imgAry[r][c]);
		}
		line += "\r\n";
	}
	print_append(line, output_file_name);

	return;
}

int Kmean :: computeDist(Point p, xyCoord c)
{
	int x1 = p.Xcoord;
	int y1 = p.Ycoord;
	int x2 = c.Xcoord;
	int y2 = c.Ycoord;
	int result = 0;

	result = (y1 - y2)*(y1 - y2);
	result += (x1 - x2)*(x1 - x2);
	result = sqrt(result);

	return result;
}

void Kmean :: DistanceMinLabel(int index)
{
	int minDist = 99999;
	int minLabel = 0;
	int dist;
	for(int l = 1; l <= this->K; l++)
	{
		dist = this->computeDist(this->pointSet[index],
			this->Kcentroids[l]);
		if(dist < minDist)
		{
			minDist = dist;
			minLabel = l;
		}
	}

	if(minDist < this->pointSet[index].Distance)
	{
		this->pointSet[index].Distance = minDist;
		this->pointSet[index].Label = minLabel;
		this->change++;
	}

	return;
}

void Kmean :: kMeansClustering(string output_file_name)
{
	int iteration = 1;
	this->assignLabel();
	this->change = 99999;

	while(this->change != 0)
	{
		this->Point2Image();
		this->printImage(output_file_name, iteration);
		this->computeCentroids();
		this->change = 0;
		
		for(int i = 0; i < this->numPts; i++)
		{
			this->DistanceMinLabel(i);
		}
		iteration++;
	}

	return;
}



int main(int argc, char ** argv)
{
	// Getting K from user
	int input_K;
	cout << "Please input K: " << endl;
	cin >> input_K;
	Kmean* kc = new Kmean(input_K);

	// Cleaning up the files
	file_cleaner(argv[2]);
	file_cleaner(argv[3]);
	file_cleaner(argv[4]);

	// Extracting points and output to file
	kc->extractPts(argv[1], argv[2]);
	// Reading <x, y> pair from file and putting into pointSet
	kc->loadPointSet(argv[2]);
	// Assigning labels
	kc->kMeansClustering(argv[3]);
	// Printing out pointSet to file
	kc->writePtSet(argv[4]);

	delete[] kc->pointSet;
	delete[] kc->Kcentroids;
	delete[] kc->imgAry;
	delete kc;

	return 0;
}
