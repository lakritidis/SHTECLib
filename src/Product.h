#ifndef PRODUCT_H
#define PRODUCT_H


class Product : public Entity {
	protected:
		uint32_t vendor_id;

	public:
		Product();
		Product(uint32_t);
		~Product();

		uint32_t get_vendor_id();
		void set_vendor_id(uint32_t);
};

#endif // PRODUCT_H
