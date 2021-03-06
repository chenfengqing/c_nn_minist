// NN_ARRAY.cpp: 定义控制台应用程序的入口点。
// 277178609@qq.com 2018/7/31

#include "stdafx.h"
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <math.h>
#include <fstream>
#include <Windows.h>

using namespace std;

const int LAYERS = 3;
const int IN_NUMBS = 784;
const int Hi_NUMBS = 128;
const int OUT_NUMBS = 10;

float * output_layers[LAYERS] = { 0 };	//指针的数组，每个指针指向不同的float数组，数组中的元素为上一神经层中的每个neural和此neural相连的对应的权重点乘后 的输出
float * gradient_layers[LAYERS] = { 0 };		//指针的数组，每个指针指向不同的float数组，数组中的元素为下一层神经层的输出

float eta = (float)0.1;	//学习效率
float alpha = (float)0.5;	//动能

const string training_image_fn = "mnist/train-images.idx3-ubyte";
const string training_label_fn = "mnist/train-labels.idx1-ubyte";
const string testing_image_fn = "mnist/t10k-images.idx3-ubyte";
const string testing_label_fn = "mnist/t10k-labels.idx1-ubyte";


const string model_fn_0 = "model-neural-network_0.dat";
const string model_fn_1 = "model-neural-network_1.dat";

const int nTraining = 60000;
const int nTesting = 10000;
const int width = 28;
const int height = 28;
const int epochs = 512;
const double epsilon = 1e-2;

ifstream image;
ifstream label;
ifstream image_t;
ifstream label_t;

float randomWeight(void) {
	int sign = rand() % 2;
	float rand_w= (float)rand() / RAND_MAX;
	if (sign == 1) {
		rand_w = -rand_w;
	}
	return rand_w;
}		
void init_matrix(float** matrix, int row, int col, bool def_int = 1) {
	/*根据def_int初始化二维数组matrix，为1时初始化连接神经元的weights；为0时初始化对应的delta weights

	Args:
	matrix:二维数组的指针
	row:行
	col:列

	return:
	void.
	*/
	for (int r = 0; r < row; r++) {
		for (int c = 0; c < col; c++) {
			if (def_int) {
				*((float*)matrix + col * r + c) = randomWeight();
			}
			else {
				*((float*)matrix + col * r + c) = 0.0;
			}
		}
	}
}
void get_row_matrix(float** matrix, int row, int col, int w_row) {
	/*设置二维数组中的某行。
	Args:
	w_row：制定的某行。
	Return:
	void.
	*/
	for (int c = 0; c < col; c++) {
		*((float*)matrix + col * w_row + c) = 1;	//cool*w_row不变，c 改变
	}
}
void get_col_matrix(float** matrix, int row, int col, int w_col) {
	/*设置二维数组中的某列。
	Args:
	w_col：制定的某列。
	Return:
	void.
	*/
	for (int r = 0; r < row; r++) {
		*((float*)matrix + r * col + w_col) = 1;	//w_col不变，r*col改变
	}
}
void display_matrix(float** matrix, int row, int col, const char* tips = 0) {
	/*打印二维数组中的元素
	Args:
	Return:
	void.
	*/
	if (tips != 0) cout << tips << " \t" << endl;
	for (int r = 0; r < row; r++) {
		for (int c = 0; c < col; c++) {
			cout << " " << *((float*)matrix + col * r + c);
		}
		cout << endl;
	}
}
void display_array(float* list, int n, const char* tips = 0) {
	/*打印 数组中的元素
	Args:
	Return:
	void.
	*/
	if (tips != 0) cout << tips << "\t" << endl;
	for (int i = 0; i < n; i++) {
		cout << " \t " <<i<<"-"<< list[i];
	}
	cout << endl;
}
void copy_array(float* dest, int n, float* source) {
	/*从source复制到dest.
	Args:

	Returns :
	void.
	*/
	for (int i = 0; i < n; i++) {
		dest[i] = source[i];
	}
}
void mul_i_o(float** matrix, float* input_T, float* output, int row, int col) {
	/*matrix和input_T进行矩阵乘法，matrix 每行和input_T相乘结果放入output对应的元素
	Args:
	matrix：二维数组
	row:行数
	col:列数
	input_T：输入.input_T中的_T表示为列向量，此数组的长度和matrix的列一致。
	output:输出。此数组的长度和matrix的行是一致的。
	Return:
	void.
	*/
	for (int r = 0; r < row; r++) {//r 为行索引
		float sum = 0.0;
		for (int c = 0; c < col; c++) {//c为列索引
			sum += *((float*)matrix + col * r + c) * input_T[c];
		}
		output[r] = sum;
	}
}
float sigmoid_f(float x) {
	/*激活函数sigmoid的偏导数.

	Args:
	Return:
	*/
	return float(1.0 / (1.0 + exp(-x)));
}
float sigmoid_df(float x) {
	/*激活函数sigmoid的偏导数.

	Args:
	Return:
	*/
	return x * (1 - x);
}
float tanh_f(float x) {
	/*	激活函数 和tanh_list一起使用 2个函数应该有个函数模板，待修改.

	Args:
	Returns:
	*/
	return tanh(x);
}
float tanh_df(float x) {
	/*激活函数tanh的偏导数.

	Args:
	Return:
	*/
	return (float)(1.0 - x * x);
}
void activefunction(float* list, int n, float(*fun)(float)) {
	/*	对 list中的每个元素使用fun函数 。fun应该是激励函数，可以修改为其他函数，比如sigmoid relu等。

	Args:
	list： 数组的指针，一队float的地址
	n:数组的长度，即list的长度
	fun:函数指针，参数是float返回也是float 输入和输出参数类型一致。

	Returns:
	void.
	*/
	for (int i = 0; i < n; i++) {
		list[i] = fun(list[i]);
	}
}
float error(float* output, float* targets, int n) {
	/*最后层神经层的输出数组output和实际值的数组targets求单次误差。（应该对所有样本的误差求和）。
	Args:
	n：output,targets,output_grad长度都是一样的。
	Returns:
	void.
	*/
	float m_error = 0.0;
	for (int i = 0; i < n; ++i) {
		float delta = targets[i] - output[i];
		m_error += delta * delta;
	}
	m_error /= n;
	m_error = sqrt(m_error);
	return m_error;
}
void o_gradients(float* output, float* targets, float *output_grad, int n, float(*fun)(float)) {
	/*最后层神经层的输出数组output和实际值的数组targets求梯度放入output对应的梯度数组。
	Args:
	n：output,targets,output_grad长度都是一样的。
	Returns:
	void.
	*/
	for (int i = 0; i < n; i++) {
		float delta = targets[i] - output[i];
		output_grad[i] = delta * fun(output[i]);
	}
}
float sum(float** matrix, int row, int col, int w_col, float* next_g) {
	/*	下一层的梯度和matrix对应的列点乘得到的和
	Args:
	next_g:下一层的梯度数组
	Return:

	*/
	float sum = 0.0;
	for (int r = 0; r < row; r++) {
		sum += *((float*)matrix + r * col + w_col)*next_g[r];
	}
	return sum;
}
void h_gradients(float** matrix, int row, int col, float* grad, float* output, float *next_g, float(*fun)(float)) {
	/*	下一层的梯度和matrix对应的列点乘得到的和
	Args:
	g:当前神经层的梯度数组
	o:当前神经层的输出数组
	next_g：下层神经层的梯度数组

	Return:

	*/
	for (int c = 0; c < col; c++) {
		float sum_gradient = sum(matrix, row, col, c, next_g);
		//cout << "\tSum" << derivative(output[c]) << endl;
		grad[c] = sum_gradient * fun(output[c]);
	}
}
void updata_weight(float** matrix, int row, int col, float** delta_m, float* prelayer_output, float* layer_grad) {
	/*当前层对前面层中 每给neural求delta_weight然后更新

	*/
	for (int r = 0; r < row; r++) {
		for (int c = 0; c < col; c++) {
			float old_delta_w = *((float*)delta_m + col * r + c);
			float new_delta_w = eta * prelayer_output[c] * layer_grad[r] + alpha * old_delta_w;
			*((float*)delta_m + col * r + c) = new_delta_w;
			*((float*)matrix + col * r + c) += new_delta_w;
		}
	}
}
void input(float* inputs,float* targets,ifstream& image,ifstream& label,bool flag=false) {
	char number;
	for (int j = 0; j < height; ++j) {
		for (int i = 0; i < width; ++i) {
			image.read(&number, sizeof(char));
			if (number == 0) {
				inputs[i + j * width] = 0;
			}
			else {
				inputs[i + j * width] = 1;
			}
		}
	}
	if (flag) {
		cout << "Image:" << endl;
		for (int j = 0; j < height; ++j) {
			for (int i = 0; i < width; ++i) {
				cout << inputs[i + j * width];
			}
			cout << endl;
		}
	}

	label.read(&number, sizeof(char));
	for (int i = 0; i < OUT_NUMBS; ++i) {
		targets[i] = 0.0;
	}
	targets[number] = 1.0;
	if(flag)cout << "Label: " << (int)(number) << endl;
}
void write_matrix(string file_name, float** matrix, int row, int col) {
	ofstream file(file_name.c_str(), ios::out);
	for (int r = 0; r < row; ++r) {
		for (int c = 0; c < col; ++c) {
			file << *((float*)matrix + col * r + c) <<"\t";
		}
		file << endl;
	}
	file.close();
}
void read_matrix(string file_name, float** matrix, int row, int col) {
	ifstream file(file_name.c_str(), ios::in);
	for (int r = 0; r < row; ++r) {
		for (int c = 0; c < col; ++c) {
			file >> *((float*)matrix + col * r + c);
		}
	}
	file.close();
}
int main()
{
	srand((unsigned int)time(NULL));
	
	image.open(training_image_fn.c_str(), ios::in | ios::binary); 
	label.open(training_label_fn.c_str(), ios::in | ios::binary);
	image_t.open(testing_image_fn.c_str(), ios::in | ios::binary);
	label_t.open(testing_label_fn.c_str(), ios::in | ios::binary);

	char number;
	for (int i = 1; i <= 16; ++i) {
		image.read(&number, sizeof(char));
	}
	for (int i = 1; i <= 8; ++i) {
		label.read(&number, sizeof(char));
	}

	const int topology[LAYERS] = { IN_NUMBS,Hi_NUMBS,OUT_NUMBS };
	float inputs[IN_NUMBS] = {};
	float targets[OUT_NUMBS] = {};

	for (int i = 0; i < LAYERS; i++) {
		output_layers[i] = new float[topology[i] + 1];
		gradient_layers[i] = new float[topology[i] + 1];
		for (int j = 0; j < topology[i] + 1; j++) {
			gradient_layers[i][j] = 0.0;
		}
		output_layers[i][topology[i]] = 1.0;
	}

	float mat_0[Hi_NUMBS][IN_NUMBS+1];
	float mat_1[OUT_NUMBS][Hi_NUMBS+1];
	float delta_mat_0[Hi_NUMBS][IN_NUMBS + 1];
	float delta_mat_1[OUT_NUMBS][Hi_NUMBS + 1];

	init_matrix((float**)mat_0, Hi_NUMBS, IN_NUMBS + 1);
	init_matrix((float**)mat_1, OUT_NUMBS, Hi_NUMBS + 1);
	
	for (int i = 0; i < nTraining; i++) {
		input(inputs, targets,image,label);
		copy_array(output_layers[0], IN_NUMBS, inputs);
		init_matrix((float**)delta_mat_0, Hi_NUMBS, IN_NUMBS + 1, 0);
		init_matrix((float**)delta_mat_1, OUT_NUMBS, Hi_NUMBS + 1, 0);
		float o_e = 0.0;
		for (int j = 0; j < epochs; j++) {
			mul_i_o((float**)mat_0, (float*)output_layers[0], (float*)output_layers[1], Hi_NUMBS, IN_NUMBS + 1);
			activefunction(output_layers[1], Hi_NUMBS, sigmoid_f);

			mul_i_o((float**)mat_1, (float*)output_layers[1], (float*)output_layers[2], OUT_NUMBS, Hi_NUMBS + 1);
			activefunction(output_layers[2], OUT_NUMBS, sigmoid_f);

			o_gradients(output_layers[2], targets, gradient_layers[2], OUT_NUMBS, sigmoid_df);
			h_gradients((float**)mat_1, OUT_NUMBS, Hi_NUMBS + 1, gradient_layers[1], output_layers[1], gradient_layers[2], sigmoid_df);

			updata_weight((float**)mat_1, OUT_NUMBS, Hi_NUMBS + 1, (float**)delta_mat_1, output_layers[1], gradient_layers[2]);
			updata_weight((float**)mat_0, Hi_NUMBS, IN_NUMBS + 1, (float**)delta_mat_0, output_layers[0], gradient_layers[1]);

			float e = error(output_layers[2], targets, OUT_NUMBS);
			if (e < epsilon) {
				o_e = e;
				break;
			}
		}
		cout << i<<"\t";
		cout << o_e << endl;
	}
	write_matrix(model_fn_0, (float**)mat_0, Hi_NUMBS, IN_NUMBS + 1);
	write_matrix(model_fn_1, (float**)mat_1, OUT_NUMBS, Hi_NUMBS + 1);

	image.close();
	label.close();

	for (int i = 1; i <= 16; ++i) {
		image_t.read(&number, sizeof(char));
	}
	for (int i = 1; i <= 8; ++i) {
		label_t.read(&number, sizeof(char));
	}

	read_matrix(model_fn_0, (float**)mat_0, Hi_NUMBS, IN_NUMBS + 1);
	read_matrix(model_fn_1, (float**)mat_1, OUT_NUMBS, Hi_NUMBS + 1);
	int right_i = 0;
	for (int ti = 0; ti < nTesting; ti++) {
		input(inputs, targets,image_t,label_t);
		copy_array(output_layers[0], IN_NUMBS, inputs);
		mul_i_o((float**)mat_0, (float*)output_layers[0], (float*)output_layers[1], Hi_NUMBS, IN_NUMBS + 1);
		activefunction(output_layers[1], Hi_NUMBS, sigmoid_f);

		mul_i_o((float**)mat_1, (float*)output_layers[1], (float*)output_layers[2], OUT_NUMBS, Hi_NUMBS + 1);
		activefunction(output_layers[2], OUT_NUMBS, sigmoid_f);
		
		int output_i = 0;
		int target_i = 0;
		float output_max = 0.0;
		float target_max = 0.0;
		for (int ot_i = 0; ot_i < 10; ot_i++) {
			if (output_layers[2][ot_i] > output_max) {
				output_max = output_layers[2][ot_i];
				output_i = ot_i;
			}
			if (targets[ot_i] > target_max) {
				target_max = targets[ot_i];
				target_i = ot_i;
			}
		}
		if (output_i == target_i&& output_max>0.6) {//
			right_i++;
			float e = error(output_layers[2], targets, OUT_NUMBS);
			cout <<"\tti"<< ti << "\te: " << e << "\toutput_i: " << output_i << "\ttarget_i: " << target_i << "\toutput_max: " << output_max << endl;
		}
	}
	cout<< right_i <<endl;
	cout << (double)(right_i) / nTesting * 100.0 << "%" << endl;

	for (int i = 0; i < LAYERS; i++) {
		delete output_layers[i];
		delete gradient_layers[i];
	}
	
	image_t.close();
	label_t.close();

	return 0;
}

