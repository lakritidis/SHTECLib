#ifndef AGGLOMERATIVE_H
#define AGGLOMERATIVE_H

template <class T, class U> class Agglomerative : public ClusteringAlgorithm<T,U> {
	private:
		uint32_t linkage_type;

	public:
		Agglomerative(class Settings *);
		~Agglomerative();

		void exec();
};

#endif // AGGLOMERATIVE_H
