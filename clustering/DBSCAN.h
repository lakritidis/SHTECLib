#ifndef DBSCAN_H
#define DBSCAN_H


template <class T, class U> class DBSCAN : public ClusteringAlgorithm<T,U> {
	private:
		uint32_t min_points;

	public:
		DBSCAN(class Settings *);
		~DBSCAN();

		uint32_t dbscan_range_query(T ***, uint32_t *, T * q , _score_t);
		void exec();
};

#endif // DBSCAN_H
