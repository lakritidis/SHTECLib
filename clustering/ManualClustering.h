#ifndef MANUALCLUSTERING_H
#define MANUALCLUSTERING_H



template <class T, class U> class ManualClustering : public ClusteringAlgorithm<T,U>{
	public:
		ManualClustering(class Settings *);
		~ManualClustering();

		void exec();
};

#endif // MANUALCLUSTERING_H
