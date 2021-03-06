#pragma once
#include "Geometry.h"
#include <iostream>
using namespace glm;
class Square :
	public Geometry
{
public:
	Square();
	Square(const vec3 & original_point, double length_distance, double width_distance, const vec3 & normal_vector);
	Square(const vec3 & original_point, double length_distance, double width_distance, const vec3 & normal_vector, std::vector<vec3> & vertices);
	~Square();
	void setOrigin(vec3 original_point);
	void setLength(double length_distance);
	void setWidth(double width_distance);
	void setNormal(vec3 normal_vector);
	void addVertex(vec3 vertex_point);
	const vec3 getOrigin();
	const double getLength();
	const double getWidth();
	const vec3 getNormal();
	const std::vector<vec3> getVertices();
	const bool isOriginal();
private:
	// original point
	vec3 origin;
	// the length
	double length;
	// the width
	double width;
	// the normal of the circle plane
	vec3 normal;
	// the vector of vertex
	std::vector<vec3> vertex_list;
};

