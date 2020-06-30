#ifndef UPM_H
#define UPM_H


class UPM : public ClusteringAlgorithm <UPMProduct, EntitiesGroup<UPMProduct> >{
	protected:
		class Token ** tokens_index;
		class TokensLexicon * units_lexicon;
		class CombinationsLexicon * combinations_lexicon;
		class ClusterUniverse<EntitiesGroup<UPMProduct> > * cluster_universe;

		_score_t K;
		_score_t alpha;
		_score_t beta;

	public:
		UPM(class Settings *);
		~UPM();

		void read_from_file(char *);
		void exec();
};

#endif // UPM_H
