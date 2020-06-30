#ifndef FEATURE_H
#define FEATURE_H


class Feature {
	private:
		class Token * t;
		_score_t w;

	public:
		Feature();
		Feature(class Token *, _score_t);
		~Feature();

		/// Accessors
		uint32_t get_id();
		class Token * get_token();
		_score_t get_weight();

		/// Mutators
		void set_token(class Token *);
		void set_weight(_score_t);
};

#endif // FEATURE_H
