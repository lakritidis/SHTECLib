#ifndef CLUSTERHASHNODE_H
#define CLUSTERHASHNODE_H


template <class T> class ClusterHashNode : public Cluster<T> {
	private:
		class ClusterHashNode * next;

	public:
		ClusterHashNode(uint32_t);
		~ClusterHashNode();

		class ClusterHashNode<T> * get_next();
		void set_next(class ClusterHashNode<T> *);
};

#endif // UPMCLUSTERNODE_H
