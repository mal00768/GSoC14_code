/*
Just a basic expression template and tree optimizer test code. 
Based on code gathered from:
http://en.wikipedia.org/wiki/Expression_templates
http://eigen.tuxfamily.org/index.php?title=Working_notes_-_Expression_evaluator
and various books.

By: Mark Lingle
Email: mal00768@gmail.com
*/

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>


//making life easy on myself and sticking with std::vector< std::vector<double> >
typedef double value_type; 
typedef std::vector< std::vector<value_type> > dense_matrix;

//Base class. Using the CRTP template pattern that seems to be popular in other libraries
template <typename MatXpr>
class Matrix_Base {

public:
	Matrix_Base() {}
	~Matrix_Base() {}

	//some useful operators and functions for this template pattern
	size_t size() const { 
		return static_cast<const MatXpr&>(*this).size(); 
	}

	value_type operator()(size_t i, size_t j) const { 
  		return static_cast<const MatXpr&>(*this)(i, j); 
  	}
 
  	operator MatXpr&() { 
  		return static_cast<MatXpr&>(*this); 
  	}

  	operator const MatXpr&() const { 
  		return static_cast<const MatXpr&>(*this); 
  	}

};

//Dense matrix class that extends the base class 
class Dense_Matrix : public Matrix_Base< Dense_Matrix > { //CRTP pattern
	
	public:
		Dense_Matrix(const std::string& name, int n = 1) : m_name(name) { //default size is 1 element, random elements
			std::vector<value_type> row(n, 0);
			data.resize(n, row);
			rand_init(); 
			std::cout << "- Create matrix " << m_name << std::endl;
		}
		
   		template <typename T>
   		Dense_Matrix(const Matrix_Base<T>& mexpr) { // Construct from a vector_expression
   			std::vector<value_type> row(mexpr.size(), 0);
     		data.resize(mexpr.size(), row );
     		for(size_t i = 0; i < mexpr.size(); ++i) {
     			for(size_t j = 0; j < mexpr.size(); ++j){
       				data[i][j] = mexpr(i,j);
       			}
     		}
   		}
		
		~Dense_Matrix() {}

		std::string name() const { return m_name; } //jsut the name

		size_t size() const { return data.size(); } //sticking with square matrices
		
		const value_type& operator()(size_t i, size_t j) const {
      		return data[i][j];
    	}

    	value_type& operator()(size_t i, size_t j) {
     		return data[i][j];
    	}

    	template <typename T>
		Dense_Matrix& operator=(const Matrix_Base<T>& right){
  			for(size_t i = 0; i < size(); ++i){
				for(size_t j = 0; j < size(); ++j){
					data[i][j] = right(i,j);
				}
			}
    		return *this;
		}

		void rand_init() {
			for(size_t i = 0; i < size(); ++i){
				for(size_t j = 0; j < size(); ++j){
					data[i][j] = rand() % 100;
				}
			}
		} 

		typedef const Dense_Matrix& Nested;

		std::string m_name;
		dense_matrix data;

};

//Represents the matrix sum. 
template <typename MatrixR, typename MatrixL>
class Matrix_Sum : public Matrix_Base< Matrix_Sum<MatrixR, MatrixL> > {

public:
	Matrix_Sum(const Matrix_Base<MatrixR>& ml, const Matrix_Base<MatrixL>& mr) : matrixr(ml), matrixl(mr) {}

	~Matrix_Sum() {}

	std::string name() const { return std::string("(") + matrixl.name() + " + " + matrixr.name() + ")"; }

	size_t size() const { return matrixr.size(); } //this should throw something if v1.size() != v2.size()
	
    value_type operator()(size_t i, size_t j) const {
      	return matrixr(i, j) + matrixl(i, j);
    }

    typedef const Matrix_Sum Nested; //these are needed for the tree optimizer object
    typename MatrixR::Nested matrixr;
  	typename MatrixL::Nested matrixl;
  	/*
	const MatrixR& matrixr;
	const MatrixL& matrixl;
	*/
};

//represents a matrix product
template <typename MatrixR, typename MatrixL>
class Matrix_Product : public Matrix_Base< Matrix_Product<MatrixR, MatrixL> > {

public:
	Matrix_Product(const Matrix_Base<MatrixR>& mr, const Matrix_Base<MatrixL>& ml) : matrixr(mr), matrixl(ml) {}

	~Matrix_Product() {}

	std::string name() const { return std::string("(") + matrixl.name() + " * " + matrixr.name() + ")"; }

	size_t size() const { return matrixr.size(); } //this should throw something if v1.size() != v2.size()

	// TO DO: implement the multiplication!

	typedef const Matrix_Product Nested;
	typename MatrixR::Nested matrixr;
  	typename MatrixL::Nested matrixl;
  	/*
	const MatrixR& matrixr;
	const MatrixL& matrixl;
	*/
};

//useful operator overloads for + and *
template <typename MatrixR, typename MatrixL>
Matrix_Sum<MatrixR, MatrixL> operator+(const Matrix_Base<MatrixR>& mr, const Matrix_Base<MatrixL>& ml){
	return Matrix_Sum<MatrixR, MatrixL> (mr, ml);
} 

template <typename MatrixR, typename MatrixL>
Matrix_Product<MatrixR, MatrixL> operator*(const Matrix_Base<MatrixR>& mr, const Matrix_Base<MatrixL>& ml){
	return Matrix_Product<MatrixR, MatrixL> (mr, ml);
} 

// Default tree optimizer that copies the expression by value
template<typename MatXpr> 
class Tree_Optimizer {
public:
  typedef MatXpr NMatXpr;
  static MatXpr build(const MatXpr& mxpr) { return mxpr; }
};

// Tree optimizer for Matrix that copies by reference
template<> 
class Tree_Optimizer<Dense_Matrix> {
public:
  typedef Dense_Matrix NMatXpr;
  static const Dense_Matrix& build(const Dense_Matrix& mxpr) { return mxpr; }
};

// Needed to forward the optimizer to the children
template<typename A, typename B> 
class Tree_Optimizer<Matrix_Sum<A, B> > {
public:
  typedef Matrix_Sum<A, B> MatXpr;
  typedef typename Tree_Optimizer<A>::NMatXpr NMatA;
  typedef typename Tree_Optimizer<B>::NMatXpr NMatB;
  typedef Matrix_Sum<NMatA, NMatB> NMatXpr;
  static NMatXpr build(const MatXpr& mxpr) { return Tree_Optimizer<A>::build(mxpr.matrixr) + Tree_Optimizer<B>::build(mxpr.matrixl); }
};

// catch A * B + C and builds C + A*B
template<typename A, typename B, typename C>
class Tree_Optimizer<Matrix_Sum<Matrix_Product<A, B>, C> > {
public:
  typedef Matrix_Sum<Matrix_Product<A, B>, C> MatXpr;
  typedef typename Tree_Optimizer<C>::NMatXpr NMatC;
  typedef Matrix_Sum<NMatC, Matrix_Product<A, B> > NMatXpr;
  static NMatXpr build(const MatXpr& mxpr) { return Tree_Optimizer<C>::build(mxpr.matrixr) + mxpr.matrixl; }
};

// catch C + A * B + D and builds (C + D) + (A*B)
template<typename A, typename B, typename C, typename D>
class Tree_Optimizer<Matrix_Sum<Matrix_Sum<C, Matrix_Product<A,B> >,D> > {
public:
  typedef Matrix_Sum<Matrix_Sum<C, Matrix_Product<A,B> >, D> MatXpr;
  typedef typename Tree_Optimizer<C>::NMatXpr NMatC;
  typedef typename Tree_Optimizer<D>::NMatXpr NMatD;
  typedef Matrix_Sum<Matrix_Sum<NMatC, NMatD>, Matrix_Product<A,B> > NMatXpr;
  static NMatXpr build(const MatXpr& mxpr) { return (Tree_Optimizer<C>::build(mxpr.matrixr.matrixr) + Tree_Optimizer<D>::build(mxpr.matrixl)) + mxpr.matrixr.matrixl; }
};

int main(){

	srand(time(NULL)); //we're going to randomly generate the matrices

	Dense_Matrix a("a", 3), b("b", 3), c("c", 3), d("d", 3);

	d = a + b + c;
	/*
	for(size_t i = 0; i < d.size(); ++i){
		for(size_t j = 0; j < d.size(); ++j){
			std::cout << d(i,j) << " ";
		} std::cout << std::endl;
	}
	*/
	auto xpr = a + b*c + d + c;
    //auto xpr = a + b*c;
    
    typedef __typeof(xpr) Xpr;
    
    std::cout << "init version:";
    std::cout << " " << xpr.name() << "\n";
    
    auto xpr1 = Tree_Optimizer<Xpr>::build(xpr);
    typedef __typeof(xpr1) Xpr1;
    std::cout << std::endl << "optimized version 1:";
    std::cout << " " << xpr1.name() << std::endl;
    
    auto xpr2 = Tree_Optimizer<Xpr1>::build(xpr1);
    typedef __typeof(xpr2) Xpr2;
    std::cout << std::endl << "optimized version 2:";
    std::cout << " " << xpr2.name() << std::endl;

    auto xpr3 = Tree_Optimizer<Xpr2>::build(xpr2);
    typedef __typeof(xpr3) Xpr3;
    std::cout << std::endl << "optimized version 3:";
    std::cout << " " << xpr3.name() << std::endl;

	return 0;
}