#ifndef EVROT_H
#define EVROT_H


#define DEBUG 0    // set to 1 to see print outs
#define  EPS 2.2204e-16

class Evrot {

public:
	Evrot(Eigen::MatrixXd& X, int method);
	virtual ~Evrot();
	double getQuality() { return mQuality; }
	std::vector<std::vector<int> > getClusters() { return mClusters; }
	Eigen::MatrixXd& getRotatedEigenVectors() { return mXrot; }

protected:
	void evrot();
	void cluster_assign();
	double evqual(Eigen::MatrixXd& X);
	double evqualitygrad(Eigen::VectorXd& theta, int angle_index);
	Eigen::MatrixXd& rotate_givens(Eigen::VectorXd& theta);
	Eigen::MatrixXd& build_Uab(Eigen::VectorXd& theta, int a, int b);
	Eigen::MatrixXd& gradU(Eigen::VectorXd& theta, int k);

	//constants

	int mMethod;
	const int mNumDims;
	const int mNumData;
	int mNumAngles;
	Eigen::VectorXi ik;
	Eigen::VectorXi jk;

	//current iteration
	Eigen::MatrixXd& mX;
	Eigen::MatrixXd mXrot;
	double mQuality;

	std::vector<std::vector<int> > mClusters;
};


#endif // EVROT_H
