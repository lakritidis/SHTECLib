#ifndef KMEANS_H
#define KMEANS_H

template <class T, class U> class KMeans : public ClusteringAlgorithm<T,U> {
	private:
		uint32_t K_clusters;
		uint16_t iterations;
		uint16_t type;

	public:
		KMeans(class Settings *);
		~KMeans();

		void exec();
		void exec_standard();
		void exec_standard_clustroids();
		void exec_spherical();

		uint32_t get_nearest_cluster(T *);
		void cluster();
};

#endif // KMEANS_H
