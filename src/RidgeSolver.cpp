#include "RidgeSolver.h"
#include "Exception.h"
#include <algorithm>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>

// ############################################################################

RegressionResult::RegressionResult(uvector beta, uvector yEst, float sqError) {
	_beta = beta;
	_yEst = yEst;
	_sqError = sqError;
}

RegressionResult::RegressionResult() {
	_beta = uvector(0);
	_yEst = uvector(0);
	_sqError = 0.0f;
}

void RegressionResult::print(ostream &target) {
	target << "beta =\t" << _beta << "\n";
	target << "yEst =\t" << _yEst << "\n";
	target << "_sqError =\t" << _sqError << "\n";
}

// ############################################################################

RidgeRegressionResult::RidgeRegressionResult() {
	RegressionResult();
	_lambda = 0;
}

RidgeRegressionResult::RidgeRegressionResult(uvector beta, uvector yEst,
		float sqError, float lambda) {
	RegressionResult(beta, yEst, sqError);
	_lambda = lambda;
}

void RidgeRegressionResult::print(ostream &target) {
	RegressionResult::print(target);
	target << "lambda =\t" << _lambda << "\n";

}

// ############################################################################

RidgeRegression::RidgeRegression(boost::numeric::ublas::matrix<float> X,
		uvector Y, float lambda) {

	rrX = X;
	rrXt = trans(X);
	rrY = Y;
	rrlambda = lambda;

	//cerr << X << endl;

}

RidgeRegressionResult RidgeRegression::solve() {

	RidgeRegressionResult res;

	try {

		size_t n = rrX.size2();
		boost::numeric::ublas::matrix<float> inv(n, n);
		boost::numeric::ublas::matrix<float> P(n,n);
		boost::numeric::ublas::axpy_prod(rrXt, rrX, P);
		boost::numeric::ublas::identity_matrix<float> I(n);
		boost::numeric::ublas::matrix<float> regMat = (P + (rrlambda * I));

		InvertMatrix(regMat, inv);

		boost::numeric::ublas::vector<float> tmp(n);
		boost::numeric::ublas::axpy_prod(rrXt, rrY, tmp);

		res._lambda = rrlambda;
		res._beta = boost::numeric::ublas::vector<float>(n);
		res._yEst = boost::numeric::ublas::vector<float>(n);

		boost::numeric::ublas::axpy_prod(inv, tmp, res._beta);
		boost::numeric::ublas::axpy_prod(rrX, res._beta, res._yEst);

		res._sqError = pow(norm_2(rrY - res._yEst), 2) / float(rrX.size1());


		inv.resize(0,0,false);
		P.resize(0,0,false);
		I.resize(0,false);
		regMat.resize(0,0,false);
		tmp.resize(0,false);


	} catch (boost::numeric::ublas::internal_logic &e) {
		cerr << "\nERROR in RidgeRegression::solve: numeric instability.\n";
		cerr << "The RidgeRegression caught the error boost::numeric::ublas::internal_logic,\n";
		cerr << "this means boost thinks that the current optimization problem is unstable.\n";
		cerr << "Did you set a lambda parameter that was too small?\n";

		cerr << "\nYour lambda: " << rrlambda << endl;

		throw (Exception("Numeric instability"));
	}

	return res;

}
