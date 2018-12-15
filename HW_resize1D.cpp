//Homework 4 Problem 1
/* 
Command Line Function HW_resize1D that takes 5 parameters [input_file.txt] [output_array_length] [kernelID] [param] [output_file.txt]
Example: input.txt 256 0 1 output.txt
This example will use the array stored in input.txt, and resize it to have 256 elements using box filter method. The output is then stored in output.txt
*/

#define _CRT_SECURE_NO_WARNINGS
//defined this here due to visual studio error when file opening fopen();

#include <iostream>
using namespace std;


#define PI		3.1415926535897931160E0
#define SGN(A)		((A) > 0 ? 1 : ((A) < 0 ? -1 : 0 ))
#define FLOOR(A)	((int) (A))
#define CEILING(A)	((A)==FLOOR(A) ? FLOOR(A) : SGN(A)+FLOOR(A))
#define CLAMP(A,L,H)	((A)<=(L) ? (L) : (A)<=(H) ? (A) : (H))

double boxFilter(double t, double param);
double triFilter(double t, double param);
double cubicConv(double t, double param);
double lanczos(double t, double param);
double hann(double t, double param);
double sinc(double t);
double hamming(double t, double param);

void HW_resize1D(float *IN, float *OUT, int INlen, int OUTlen, int kernel_type, double param);
void print_array(float* array, int len);

double boxFilter(double t, double param) {
	if ((t > -.5) && (t <= .5)) return (1.0);
	return (0.0);
}

double triFilter(double t, double param) {
	if (t < 0) t = -t;
	if (t < 1.0) return (1.0 - t);
	return (0.0);
}

double cubicConv(double t, double param = -1) {
	double A, t2, t3;
	if (t < 0) t = -t;
	t2 = t * t;
	t3 = t2 * t;

	A = param;
	if (t < 1.0) return ((A + 2) * t3 - (A + 3) * t2 + 1);
	if (t < 2.0) return (A*(t3 - 5 * t2 + 8 * t - 4));
	return (0.0);
}

double sinc(double t) {
	t *= PI;
	if (t != 0) return (sin(t) / t);
	return (1.0);
}

double lanczos(double t, double param = 3) {
	if (t < 0) t = -t;
	if (t < param) return (sinc(t) * sinc(t / param));
	return (0.0);
}

double hann(double t, double param = 3) {
	int N = param;
	if (t < 0) t = -t;
	if (t < N)
		return(sinc(t) * (.5 + .5*cos(PI*t / N)));
	return (0.0);
}

double hamming(double t, double param = 3) {
	int	N;
	N = (int)param;
	if (t < 0) t = -t;
	if (t < N) return(sinc(t) * (.54 + .46*cos(PI*t / N)));
	return(0.0);
}


void print_array(float* array, int len) {
	for (int i = 0; i < len; i++) {
		cout << "array[" << i << "] = " << array[i] << endl;
	}
	cout << endl;
}

int main(int argc, char* argv[]) {

	if (argc != 6) {
		cout << argc << endl;
		cout << "Invalid number of arguments. 5 Arguments Required. Enter in the form (HW_resize inputFileName, outputLength, filterType, param, outputFileName)" << endl;
		std::cin.get();
	}

	else {
		cout << "Program Finished. Output in file output.txt" << endl;

		char* inputFileName = argv[1];
		char* outputFileName = argv[5];

		int outputLength = atoi(argv[2]);
		int filterType = atoi(argv[3]);
		double  param = double(atoi(argv[4]));

		FILE *inputFile;
		FILE *outputFile;

		inputFile = fopen(inputFileName, "r");
		outputFile = fopen(outputFileName, "w+");

		int INlen = 0;
		int OUTlen = outputLength;
		float number;

		//fscanf_s(inputFile, "%d", &number);
		//cout << number << endl;
		//std::cin.get();


		while (fscanf(inputFile, "%f", &number) != EOF) {
			INlen++;
			//cout << number << endl;
		}

		fclose(inputFile);
		inputFile = fopen(inputFileName, "r");

		//cout << INlen << endl;
		float* IN = new float[INlen];
		float* OUT = new float[OUTlen];

		for (int j = 0; j < INlen; j++) {
			fscanf(inputFile, "%f", &number);
			IN[j] = number;
		}

		HW_resize1D(IN, OUT, INlen, OUTlen, filterType, param);
		for (int j = 0; j < OUTlen; j++) {
			fprintf(outputFile, "%f\n", OUT[j]);
		}

		//print_array(OUT, OUTlen);

		fclose(inputFile);
		fclose(outputFile);
		std::cin.get();
		return 0;
	}
}



//float *IN, float *OUT, int INlen, int OUTlen, int kernel_type, double param
void HW_resize1D(float *IN, float *OUT, int INlen, int OUTlen, int kernel_type, double param) {
	//cout << "Function call to HW_resize1D" << endl;

	int offset = 0;

	int i;
	int left, right;
	int pixel;
	double u, x;		//u is input , x is output
	double scale;
	double(*filter)(double, double) = nullptr;
	double fSupport;
	double fscale;
	double weight;
	double acc;

	scale = (double)OUTlen / INlen;
	//outlen should be 256 and inlen 32 , scale should be 8

	switch (kernel_type) {
	case 0:
		filter = boxFilter;
		fSupport = .5;
		break;

	case 1:
		filter = triFilter;
		fSupport = 1;
		break;

	case 2:
		filter = cubicConv;
		fSupport = 2;
		break;

	case 3:
		filter = lanczos;
		fSupport = param;
		break;

	case 4:
		filter = hann;
		fSupport = param;
		break;

	case 5:
		filter = hamming;
		fSupport = param;
		break;

	}

	double fwidth = fSupport;
	fscale = 1.0;
	scale = (double)OUTlen / INlen;

	if (scale < 1.0) {
		//minify
		fwidth = fSupport / scale;
		fscale = scale;
	}

	else //magnify
	{
		fscale = 1.0;
	}

	//x goes from 0 to 255
	for (x = 0; x < OUTlen; x++) {
		// u goes from 0 to 31
		u = x / scale;

		if (u - fwidth < 0) {
			left = FLOOR(u - fwidth);

		}
		else {
			left = CEILING(u - fwidth);
		}

		right = FLOOR(u + fwidth);
		acc = 0;

		for (i = left; i <= right; i++) {

			double y = (CLAMP(i, 0, INlen - 1));
			pixel = IN[(int)y];
			double z = ((u - i) * fscale);

			weight = (*filter)(((u - i) * fscale), param);
			acc += (pixel* weight);
		}

		OUT[(int)(x)] = acc * fscale;
	}



}

