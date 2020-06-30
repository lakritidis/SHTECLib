#ifndef SPECTRALCLUSTERING_H
#define SPECTRALCLUSTERING_H

#include "../lib/eigen/Eigen/QR"
#include "../lib/eigen/Eigen/Core"
#include "../lib/eigen/Eigen/Array"
#include "../src/ClusterRotate.cpp"

template <class T, class U> class SpectralClustering : public ClusteringAlgorithm<T,U> {
	private:
		Eigen::MatrixXd mEigenVectors;
		uint32_t iterations;
		uint32_t K_clusters;
		bool autotune;

	public:
		SpectralClustering(class Settings *);
		~SpectralClustering();

		void exec();
		std::vector<std::vector<int32_t> > clusterRotate();
		std::vector<std::vector<int32_t> > clusterKmeans(int32_t);
};

#endif // SPECTRALCLUSTERING_H
