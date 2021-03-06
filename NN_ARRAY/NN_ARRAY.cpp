// NN_ARRAY.cpp: 定义控制台应用程序的入口点。
// 277178609@qq.com 2018/7/31

#include "stdafx.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <ctime>
#include <math.h>
#include <cstring>
#include <Windows.h>
#include <fstream>

using namespace std;

const int LAYERS = 3;
const int IN_NUMBS = 4;
const int OUT_NUMBS = 2;


double randomWeight(void) { return rand() / double(RAND_MAX); }		//随机数，范围0-1

double * output_layers[LAYERS] = { 0 };	//指针的数组，每个指针指向不同的double数组，数组中的元素为上一神经层中的每个neural和此neural相连的对应的权重点乘后 的输出
double * gradient_layers[LAYERS] = { 0 };		//指针的数组，每个指针指向不同的double数组，数组中的元素为下一层神经层的输出

double eta = 0.1;	//学习效率
double alpha = 0.5;	//动能

void init_matrix(double** matrix, int row, int col,bool def_int=1) {
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
				*((double*)matrix + col * r + c) = randomWeight();
			}
			else {
				*((double*)matrix + col * r + c) = 0.0;
			}
		}
	}
}
//void get_row_matrix(double** matrix, int row, int col, int w_row){
//	/*设置二维数组中的某行。
//		Args:
//			w_row：制定的某行。
//		Return:
//			void.
//	*/
//	for (int c = 0; c < col; c++) {
//		*((double*)matrix + col * w_row + c) = 1;	//cool*w_row不变，c 改变
//	}
//}
//void get_col_matrix(double** matrix, int row, int col, int w_col){
//	/*设置二维数组中的某列。
//		Args:
//			w_col：制定的某列。
//		Return:
//			void.
//	*/
//	for (int r = 0; r < row; r++) {
//		*((double*)matrix + r * col + w_col) = 1;	//w_col不变，r*col改变
//	}
//}
void display_matrix(double** matrix, int row, int col, const char* tips = 0) {
	/*打印二维数组中的元素 
		Args:
		Return:
			void.
	*/
	if (tips != 0) cout << tips << " \t" << endl;
	for (int r = 0; r < row; r++) {
		for (int c = 0; c < col; c++) {
			cout  <<" \t "<< *((double*)matrix + col * r + c) ;
		}
		cout << endl;
	}
}
void display_array(double* list, int n, const char* tips = 0) {
	/*打印 数组中的元素 
		Args:
		Return:
			void.
	*/
	if (tips != 0) cout << tips << " \t" << endl;
	for (int i = 0; i < n; i++) {
		cout<<" \t "<< list[i] << endl;
	}
	//cout << endl;
}

void copy_array(double* dest, int n, double* source) {
	/*从source复制到dest.
		Args:

		Returns :
			void.
	*/
	for (int i = 0; i < n; i++) {
		dest[i] = source[i];
	}
}
void mul_i_o(double** matrix, double* input_T, double* output,int row, int col) {
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
		double sum = 0.0;
		for (int c = 0; c < col; c++) {//c为列索引
			sum += *((double*)matrix + col * r + c) * input_T[c];
		}
		output[r] = sum;	
	}	
}

double tanh_f(double x) {
	/*	激活函数 和tanh_list一起使用 2个函数应该有个函数模板，待修改.
			
		Args:
		Returns:		
	*/
	return tanh(x);
}
double derivative(double x) {
	/*激活函数tanh的偏导数.

		Args:
		Return:
	*/
	return 1.0 - x * x;
}
void tanh_list(double* list, int n, double(*fun)(double) = tanh_f) {
	/*	对 list中的每个元素使用fun函数 。fun应该是激励函数，可以修改为其他函数，比如sigmoid等。

		Args:
			list： 数组的指针，一队double的地址
			n:数组的长度，即list的长度
			fun:函数指针，参数是double返回也是double 输入和输出参数类型一致。

		Returns:
			void.
	*/
	for (int i = 0; i < n; i++) {
		list[i] = fun(list[i]);
	}
}

double error(double* output, double* targets, int n) {
	/*最后层神经层的输出数组output和实际值的数组targets求单次误差。（应该对所有样本的误差求和）。
		Args:
			n：output,targets,output_grad长度都是一样的。
		Returns:
	*/
	double m_error = 0.0;
	for (int i = 0; i < n; ++i) {
		double delta = targets[i]- output[i];
		m_error += delta * delta;
	}
	m_error /= n; 
	m_error = sqrt(m_error); 
	return m_error;
}
void o_gradients(double* output, double* targets, double *output_grad, int n) {
	/*最后层神经层的输出数组output和实际值的数组targets求梯度放入output对应的梯度数组。
		Args:
			n：output,targets,output_grad长度都是一样的。
		Returns:	
			void.
	*/
	for (int i = 0; i < n; i++) {
		double delta = targets[i] - output[i];
		output_grad[i] = delta * derivative(output[i]);
	}
}
double sum(double** matrix, int row, int col, int w_col, double* next_g) {
	/*	下一层的梯度和matrix对应的列点乘得到的和
		Args:
			next_g:下一层的梯度数组
		Return:
			
	*/
	double sum = 0.0;
	for (int r = 0; r < row; r++) {
		sum+=*((double*)matrix + r * col + w_col)*next_g[r];
	}
	return sum;
}
void h_gradients(double** matrix, int row, int col, double* grad, double* output, double *next_g) {
	/*	下一层的梯度和matrix对应的列点乘得到的和
	Args:
		g:当前神经层的梯度数组
		o:当前神经层的输出数组
		next_g：下层神经层的梯度数组
	
	Return:

	*/
	for (int c = 0; c < col; c++) {
		double sum_gradient = sum(matrix, row, col, c, next_g);
		//cout << "\tSum" << derivative(output[c]) << endl;
		grad[c] = sum_gradient * derivative(output[c]);
	}
}

void updata_weight(double** matrix, int row, int col, double** delta_m, double* prelayer_output, double* layer_grad) {
	/*当前层对前面层中 每给neural求delta_weight然后更新

	*/
	for (int r = 0; r < row; r++) {
		for (int c = 0; c < col; c++) {
			double old_delta_w = *((double*)delta_m + col * r + c);
			double new_delta_w = eta * prelayer_output[c] * layer_grad[r] + alpha * old_delta_w;
			*((double*)delta_m + col * r + c) = new_delta_w;
			*((double*)matrix + col * r + c) += new_delta_w;
		}
	}
}
int main()
{
	srand((unsigned int)time(NULL));
	ofstream of("abc.txt");
	const int topology[LAYERS] = { 4,3,2 };
	double inputs[IN_NUMBS] = {};
	double targets[OUT_NUMBS] = { };

	for (int i = 0; i < LAYERS; i++) {
		output_layers[i] = new double[topology[i]+1];
		gradient_layers[i] = new double[topology[i]+1];	
		for (int j = 0; j < topology[i]+1; j++) {
			gradient_layers[i][j] = 0.0;
		}
		output_layers[i][topology[i]] = 1.0;
	}

	double mat_0[3][5];
	double mat_1[2][4];
	double delta_mat_0[3][5];
	double delta_mat_1[2][4];

	init_matrix((double**)mat_0, 3, 5);
	init_matrix((double**)mat_1, 2, 4);
	init_matrix((double**)delta_mat_0, 3, 5,0);
	init_matrix((double**)delta_mat_1, 2, 4,0);
	

	for (int i = 0; i < 100000 *1000; i++) {
		for (int j = 0; j < 4; j++) {
			inputs[j] = (rand() % 10) / (double)10 + 0.0001;
		}
		targets[0] = tanh(inputs[0] + inputs[1]) ;
		targets[1] = tanh(inputs[2] + inputs[3]) ;

		copy_array(output_layers[0], 4, inputs);
		display_array(output_layers[0], 5, "V output_layers[0]");///

		mul_i_o((double**)mat_0, (double*)output_layers[0], (double*)output_layers[1], 3, 5);
		display_matrix((double**)mat_0, 3, 5, "M mat_0");
		display_array(output_layers[1], 4, "V output_layers[1]");
		tanh_list(output_layers[1], 3);

		display_matrix((double**)mat_1, 2, 4, "M mat_1");
		mul_i_o((double**)mat_1, (double*)output_layers[1], (double*)output_layers[2], 2, 4);
		display_array(output_layers[2], 3, "V output_layers[2]");
		tanh_list(output_layers[2], 2);
		
		display_array(targets, 2, "V targets");
		o_gradients(output_layers[2], targets, gradient_layers[2], 2);	//输出最后一个元素未参与计算。
		h_gradients((double**)mat_1, 2, 4, gradient_layers[1], output_layers[1], gradient_layers[2]);
		

		updata_weight((double**)mat_1, 2, 4, (double**)delta_mat_1, output_layers[1], gradient_layers[2]);
		updata_weight((double**)mat_0, 3, 5, (double**)delta_mat_0, output_layers[0], gradient_layers[1]);

		double e = error(output_layers[2], targets, 2);
		of	<< "\toutput_layers\t" << output_layers[2][0] << "\t" << output_layers[2][1] << "\r"\
			<< "\ttargets" << "\t\t" << targets[0] << "\t" << targets[1] << "\t\t\t" << e << "\r";
		
		cout << e << " \n ";

		system("cls");
	}
	of.close();
	for (int i = 0; i < LAYERS; i++) {
		delete output_layers[i];
		delete gradient_layers[i];
	}
		
    return 0;
}

