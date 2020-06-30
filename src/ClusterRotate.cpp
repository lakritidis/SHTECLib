#include "ClusterRotate.h"

/// Default Constructor
ClusterRotate::ClusterRotate(int method): mMethod(method), mMaxQuality(0) { }

/// Destructor
ClusterRotate::~ClusterRotate() { }

///
std::vector<std::vector<int> > ClusterRotate::cluster(Eigen::MatrixXd& X) {
	uint32_t i = 0, j = 0;
	int32_t g = 0;
	_score_t d2 = 0.0;

	mMaxQuality = 0;
	std::vector<std::vector<int> > clusters;
	Eigen::MatrixXd vecRot;
	Eigen::MatrixXd vecIn = X.block(0, 0, X.rows(), 2);
	Evrot * e = NULL;

	for (g = 2; g <= X.cols(); g++) {
		printf("%d/%d ", g, X.cols()); fflush(NULL);
		/// Make it incremental (used already aligned vectors)
		if( g > 2 ) {
			vecIn.resize(X.rows(), g);
			vecIn.block(0, 0, vecIn.rows(), g - 1) = e->getRotatedEigenVectors();
			vecIn.block(0, g - 1, X.rows(), 1) = X.block(0, g - 1, X.rows(), 1);
			delete e;
		}

		/// Perform the rotation for the current number of dimensions
		e = new Evrot(vecIn, mMethod);

		/// Save max quality
		if (e->getQuality() > mMaxQuality) {
			mMaxQuality = e->getQuality();
		}

		/// Save cluster data for max cluster or if we're near the max cluster (so prefer more clusters)
		if ((e->getQuality() > mMaxQuality) || (mMaxQuality - e->getQuality() <= 0.001)) {
			clusters = e->getClusters();
			vecRot = e->getRotatedEigenVectors();
		}
	}

	Eigen::MatrixXd clusterCentres = Eigen::MatrixXd::Zero(clusters.size(),vecRot.cols());
	for (i = 0; i < clusters.size(); i++) {
		for (j = 0; j < clusters[i].size(); j++) {
			/// Sum points within cluster
			clusterCentres.row(i) += vecRot.row(clusters[i][j]);
		}
	}


	for (i = 0; i < clusters.size(); i++) {
		/// Find average point within cluster
		clusterCentres.row(i) = clusterCentres.row(i) / clusters[i].size();
	}

	/// Order clustered points by (ascending) distance to cluster centre
	for (i = 0; i < clusters.size(); i++) {
		std::multimap<double,int> clusterDistance;
		for (j = 0; j < clusters[i].size(); j++) {
			d2 = (vecRot.row(clusters[i][j]) - clusterCentres.row(i)).squaredNorm();
			clusterDistance.insert(std::make_pair(d2, clusters[i][j]));
		}

		/// The map will be sorted based on the key so just loop through it
		/// To get set of data indices sorted on the distance to cluster
		clusters[i].clear();
		for (std::multimap<double,int>::iterator it = clusterDistance.begin(); it != clusterDistance.end(); it++) {
			clusters[i].push_back(it->second);
		}
	}

	return clusters;
}
