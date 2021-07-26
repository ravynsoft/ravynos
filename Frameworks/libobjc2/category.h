#pragma once
/**
 * The structure used to represent a category.
 *
 * This provides a set of new definitions that are used to replace those
 * contained within a class.
 */
struct objc_category 
{
	/** 
	 * The name of this category.
	 */
	const char                *name;
	/**
	 * The name of the class to which this category should be applied.
	 */
	const char                *class_name;
	/**
	 * The list of instance methods to add to the class.
	 */
	struct objc_method_list   *instance_methods;
	/**
	 * The list of class methods to add to the class.
	 */
	struct objc_method_list   *class_methods;
	/**
	 * The list of protocols adopted by this category.
	 */
	struct objc_protocol_list *protocols;
	/**
	 * The list of properties added by this category
	 */
	struct objc_property_list *properties;
	/**
	 * Class properties.
	 */
	struct objc_property_list *class_properties;
};

struct objc_category_gcc
{
	/** 
	 * The name of this category.
	 */
	const char                *name;
	/**
	 * The name of the class to which this category should be applied.
	 */
	const char                *class_name;
	/**
	 * The list of instance methods to add to the class.
	 */
	struct objc_method_list_gcc   *instance_methods;
	/**
	 * The list of class methods to add to the class.
	 */
	struct objc_method_list_gcc   *class_methods;
	/**
	 * The list of protocols adopted by this category.
	 */
	struct objc_protocol_list *protocols;
};
