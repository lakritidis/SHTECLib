#ifndef SIMILARITYMEASURE_H
#define SIMILARITYMEASURE_H


template <class T> class SimilarityMeasure : public ClusteringMethod<T> {
	public:
		SimilarityMeasure();
		SimilarityMeasure(uint32_t, uint32_t, const char *, const uint32_t, const uint32_t);
		SimilarityMeasure(class Settings *);
		~SimilarityMeasure();

		void exec();
		void exec_baseline();
		void exec_weighted();
};

#endif // SIMILARITYMEASURE_H
