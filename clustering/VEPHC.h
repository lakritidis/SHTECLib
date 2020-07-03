#ifndef VEPHC_H
#define VEPHC_H


class VEPHC : public ClusteringAlgorithm<VEPHCEntity, VEPHCEntity> {
	protected:
		class Token ** tokens_index;
		class CombinationsLexicon * combinations_lexicon;

		class ClusterUniverse<VEPHCEntity> * cluster_universe;

		_score_t K;
		_score_t alpha;
		_score_t beta;

	private:
		uint32_t KazLibHash(char *);
		void insert_cluster_node(class VEPHCEntity *);

	public:
		VEPHC(class Settings *);
		~VEPHC();
		void exec();
};

#endif // VEPHC_H
