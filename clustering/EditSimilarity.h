#ifndef EDITSIMILARITY_H
#define EDITSIMILARITY_H


template <class T> class EditSimilarity : public ClusteringMethod<T> {
	private:
		uint32_t min3(uint32_t, uint32_t, uint32_t);
		uint32_t levenshtein_dis(char *, uint32_t, char *, uint32_t);
		_score_t levenshtein_sim(char *, uint32_t, char *, uint32_t);

	public:
		EditSimilarity();
		EditSimilarity(class Settings *);
		~EditSimilarity();

		void exec();
};

#endif // EDITSIMILARITY_H
