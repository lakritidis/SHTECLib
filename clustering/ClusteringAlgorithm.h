#ifndef CLUSTERINGALGORITHM_H
#define CLUSTERINGALGORITHM_H


template <class T, class U> class ClusteringAlgorithm : public ClusteringMethod<T> {
	protected:
		uint32_t num_clusters;
		uint32_t num_alloc_clusters;
		class Cluster<U> ** clusters;

		bool verification;
		uint32_t distance_metric;

	public:
		ClusteringAlgorithm(class Settings *);
		~ClusteringAlgorithm();

		_score_t ** create_knowledge_graph();
		_score_t ** create_affinity_matrix();

		void perform_verification(uint16_t);
		void HC_Stage(Cluster<T> **, _score_t, _score_t);

		void evaluate();
		double evaluate_nmi(uint32_t);
};

#endif // LEADERCLUSTERING_H

