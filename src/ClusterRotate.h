#ifndef CLUSTERROTATE_H
#define CLUSTERROTATE_H


#include "Evrot.cpp"

class ClusterRotate {
	private:
		int mMethod;
		double mMaxQuality;

	public:
		ClusterRotate(int32_t method = 1);
		~ClusterRotate();

		std::vector<std::vector<int32_t> > cluster(Eigen::MatrixXd& X);
		_score_t getMaxQuality() { return mMaxQuality; };
};

#endif // CLUSTERROTATE_H
