#ifndef LEADERCLUSTERING_H
#define LEADERCLUSTERING_H


template <class T, class U> class LeaderClustering : public ClusteringAlgorithm<T,U>{
	public:
		LeaderClustering(class Settings *);
		~LeaderClustering();

		void exec();
};

#endif // LEADERCLUSTERING_H
