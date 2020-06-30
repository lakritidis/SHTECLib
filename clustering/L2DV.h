#ifndef L2DV_H
#define L2DV_H


class L2DV : public ClusteringAlgorithm<L2DVEntity, L2DVEntity> {
	protected:
		class Token ** tokens_index;
		class CombinationsLexicon * combinations_lexicon;

		class ClusterUniverse<L2DVEntity> * cluster_universe;

		_score_t K;
		_score_t alpha;
		_score_t beta;

	private:
		uint32_t KazLibHash(char *);
		void insert_cluster_node(class L2DVEntity *);

	public:
		L2DV(class Settings *);
		~L2DV();
		void exec();
};

#endif // L2DV_H
