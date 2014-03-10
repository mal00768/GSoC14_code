/*
This is just a simple benchmark code using chrono to make a comparision between the uBlas, Eigen, and Blaze C++ libraries.
Eigen makes use of a tree optimizer object while Blaze uses smart expression templates. Both libraries proved to
be faster than the uBlas implementation.

By: Mark Lingle
Email: mal00768@gmail.com

*/

#include <chrono>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <Eigen/Dense>
#include <blaze/Math.h>

typedef double value_type;
typedef boost::numeric::ublas::matrix<value_type> ublas_matrix_type;
typedef Eigen::Matrix<value_type, Eigen::Dynamic, Eigen::Dynamic> eigen_matrix_type;
typedef blaze::DynamicMatrix<value_type> blaze_matrix_type;

size_t size = 20000;

template <typename Tvector>
inline void rand_init_ublasv(Tvector& data) {
	for(size_t i = 0; i < data.size1(); ++i){
		data(i) = rand() % 100;
	}
} 	

template <typename Tmatrix>
inline void rand_init_ublasm(Tmatrix& data) {
	for(size_t i = 0; i < data.size1(); ++i){
		for(size_t j = 0; j < data.size2(); ++j){
			data(i, j) = rand() % 100;
		}
	}
} 	

template <typename Tvector>
inline void rand_init_eigenv(Tvector& data) {
	for(size_t i = 0; i < data.rows(); ++i){
		data(i) = rand() % 100;
	}
} 	

template <typename Tmatrix>
inline void rand_init_eigenm(Tmatrix& data) {
	for(size_t i = 0; i < data.rows(); ++i){
		for(size_t j = 0; j < data.cols(); ++j){
			data(i,j) = rand() % 100;
		}
	}
} 	

template <typename Tvector>
inline void rand_init_blazev(Tvector& data) {
	for(size_t i = 0; i < data.rows(); ++i){
		data(i) = rand() % 100;
	}
} 	

template <typename Tmatrix>
inline void rand_init_blazem(Tmatrix& data) {
	for(size_t i = 0; i < data.rows(); ++i){
		for(size_t j = 0; j < data.columns(); ++j){
			data(i,j) = rand() % 100;
		}
	}
} 	

int main() {

	srand(time(NULL)); //we're going to randomly generate the vectors
/*		
	ublas_matrix_type a(size, size), b(size, size), c(size, size), d(size, size);

	rand_init_ublasm(a);
	rand_init_ublasm(b);
	rand_init_ublasm(c);

	auto start = std::chrono::steady_clock::now();
	//noalias(d) = prod(b , c);
	boost::numeric::ublas::axpy_prod(b, c, a,false);
	d += a;
	auto end = std::chrono::steady_clock::now();
*/
	/*
	for(size_t i = 0; i < a.size1(); ++i){
		for(size_t j = 0; j < a.size2(); ++j){
			std::cout << a(i, j) << " ";
		} std::cout << std::endl;
	} std::cout << std::endl;
	*/

	
	eigen_matrix_type a(size, size), b(size, size), c(size, size), d(size, size); 

	rand_init_eigenm(a);
	rand_init_eigenm(b);
	rand_init_eigenm(c);
	
	auto start = std::chrono::steady_clock::now();	
	d = a + b*c;
	auto end = std::chrono::steady_clock::now();
	
	/*
	for(size_t i = 0; i < a.rows(); ++i){
		for(size_t j = 0; j < a.cols(); ++j){
			std::cout << a(i, j) << " ";
		} std::cout << std::endl;
	} std::cout << std::endl;
	*/
	/*
	blaze_matrix_type a(size, size), b(size, size), c(size, size), d(size, size);

	rand_init_blazem(a);
	rand_init_blazem(b);
	rand_init_blazem(c);

	auto start = std::chrono::steady_clock::now();
	d = a + b*c;
	auto end = std::chrono::steady_clock::now();
	*/
	//std::cout << d;
	
	auto diff = end - start;
    	std::cout << size << " " << std::chrono::duration <double, std::milli> (diff).count() << std::endl;

	return 0;
}
