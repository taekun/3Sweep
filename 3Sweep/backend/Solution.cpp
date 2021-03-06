#include "Solution.h"
#include "ThreeSweepCmd.h"
using namespace glm;

bool Solution::compute()
{
	// TODO
	// for circle
	if (input.size() < 4) return false;
	if (shape == NOTHING) {
		vec3 first = input.getPoint(0);
		vec3 second = input.getPoint(1);
		vec3 third = input.getPoint(2);
		vec3 fourth = input.getPoint(3);

		// two strokes 
		vec3 stroke1 = second - first;
		vec3 stroke2 = fourth - third; // TODO: third or second

		shape = dot(stroke1, stroke2) < 0 ? CIRCLE : SQUARE;
	}	

	if (shape == CIRCLE) {
		if (!curt) return compute_circle();
		update_circle();
	}
	// for square
	if (shape == SQUARE) {
		if (!curt) return compute_square();
		update_square();
	}	
	return false;
}

bool Solution::compute_square()
{
	// camera is always (0, 0, -1) in this case
	// only 3 strokes can compute the rectangle
	if (input.size() < 6) return false;

	vec3 p0 = input.getPoint(0);
	vec3 p1 = 0.5f * (input.getPoint(1) + input.getPoint(2));
	vec3 p2 = 0.5f * (input.getPoint(3) + input.getPoint(4));
	vec3 p3 = input.getPoint(5);
	
	// 3 strokes 
	vec3 stroke1 = p1 - p0;
	vec3 stroke2 = p2 - p1; // TODO: third or second
	vec3 stroke3 = p3 - p2;

	// compute CN = dot(stroke_m, stroke_ohter)
	float C1 = dot(stroke1, stroke2);
	float C2 = dot(stroke2, stroke3);
	float C3 = dot(stroke3, stroke1);

	// compute z01, z12, z23
	// C1 + z01 * z12 = 0;
	// C2 + z12 * z23 = 0;
	// C3 + z23 * z01 = 0;
	// z01 > 0; z12 < 0
	float z01 = sqrtf(max(-C3 * C1 / C2, 0.0f));
	float z12 = -sqrtf(max(-C1 * C2 / C3, 0.0f));
	float z23 = C3 / z01;
	
	// update p0, p1, p2, p3
	p0 += vec3(0.f);
	p1 += vec3(0.f, 0.f, z01);
	p2 += vec3(0.f, 0.f, z01 + z12);
	p3 += vec3(0.f, 0.f, z01 + z12 + z23);

	// origin
	vec3 origin = 0.5f * (p0 + p2);

	// length and width
	float length = glm::length(p1 - p0);
	float width = glm::length(p2 - p1);

	// TODO: normal
	vec3 normal = normalize(p3 - p2);

	// list of points
	std::vector<vec3> vertices;
	vertices.push_back(p0);
	vertices.push_back(p1);
	vertices.push_back(p2);
	vertices.push_back(p3);
	// Square
	curt = new Square(origin, length, width, normal, vertices);
	history.push_back(curt);
	return update_square();
}

bool Solution::update_square()
{
	// TEST BASICS
	//return true;
	const float TH_DOT_ERR = 0.2f;
	const float TH_MAX_NUMBER_OF = 5.0f;
	/**** TODO: update circle ****/
	// things in the edge detection
	// if size < 4: means the init is not over
	if (input.size() < 6) return false;
	Square* pre_square = (Square*)curt;
	float normal_z = curt->getNormal().z;
	// to calculate the xy's length
	float length2_xy = 1.0f - normal_z * normal_z;
	float length_xy = sqrtf(length2_xy);
	for (int i = 6; i < input.size(); i++) {
		// get curt point
		vec3 curt_point = input.getPoint(i);
		vec3 pre_point = i == 6 ? curt_point : input.getPoint(i - 1);
		vec3 next_point = i == input.size() - 1 ? curt_point : input.getPoint(i + 1);
		// compute the new origin, normal; calculate the new radius
		// TODO:: compute origin with two edges
		vec3 origin = pre_square->getOrigin() + (curt_point - pre_point) + length(curt_point - pre_point) / length_xy * normal_z * vec3(0.0, 0.0f, -1.0f);
		// TODO:: curt - pre is not correct
		vec3 normal_2d = normalize(next_point - pre_point);
		vec3 perpend = normalize(cross(normal_2d, camera_direction));
		// at this time contour vector should have been inited
		// compute the radius by shooting ray at each contour point
		/***************Edge Detection*****************/
		/***
		
		Update the length and width
		
		***/		
		/***************Edge Detection*****************/
		// construct a new square
		vec3 normal = normalize(length_xy * normal_2d - vec3(0.0f, 0.0f, normal_z));
		Square* new_square = new Square(origin, pre_square->getLength(), pre_square->getWidth(), normal);
		// get the circle pointer of previous one
		pre_square = new_square;
		// push the previous circle to the history list
		history.push_back(new_square);
	}
	return true;
}

bool Solution::compute_circle()
{	
	// if curt exists, should update the circle from the init one. 
	// if (curt) return update_circle();
	// this is to compute the init circle
	// check the size of stroke before computing
	if (input.size() < 4) return false;
	// input.size >= 3
	// get the first 3 points in the Stroke
	// the first and second determine the origin and radius
	// the all 3 determine the plane normal
	vec3 first = input.getPoint(0);
	vec3 second = input.getPoint(1);
	vec3 third = input.getPoint(3);
	// for the 3rd sweep's start point
	// previous_point = third;
	/**** compute the orignal point ****/
	// middle point of 1st and 2nd
	vec3 origin = 0.5f * (first + second);
	// for cross product 
	vec3 r1 = second - origin;
	/**** compute the radius of the circle ****/
	// half of the distance between 1st and 2nd
	float radius = 0.5f * length(second - first);
	/**** compute the normal vector ****/
	// 1. get the short projection of radius
	vec3 proj_short = third - origin;
	// 2. calculate the length of the other right-angle side
	float right_angle_side_length = radius * radius - dot(proj_short, proj_short);
	if (right_angle_side_length < 0) right_angle_side_length = 0.0f;
	else right_angle_side_length = sqrt(right_angle_side_length);
	// 3. compute the offset vector in the reverse_camera_direction
	vec3 offset = right_angle_side_length * camera_direction;
	// 4. compute the real-world-axis vector of the short side
	vec3 r2 = proj_short + offset;
	// 5. calibrate
	// NEED DISCUSSION: cailbration
	// 6. compute the normal vector by rot
	vec3 normal = cross(r1, r2);
	// TODO::perspective view needed
	//vec3 normal = normalize(proj_short);
	// set output instance
	curt = new Circle(origin, radius, normal);
	// push the previous circle to the history list
	history.push_back(curt);
	//TEST
	//test(proj_short);
	//cout << "radius: " << radius << endl;
	//cout << "length(proj_short): " << length(proj_short) << endl;

	// return the success flag
	return true;
}

bool Solution::update_circle()
{
	// TEST BASICS
	//return true;
	const float TH_DOT_ERR = 0.2f;
	const float TH_MAX_NUMBER_OF = 5.0f;
	/**** TODO: update circle ****/
	// things in the edge detection
	// if size < 4: means the init is not over
	if (input.size() < 4) return false;
	Circle* pre_circle = (Circle*)curt;
	// to record the normal_z 
	float normal_z = curt->getNormal().z;
	// to calculate the xy's length
	float length2_xy = 1.0f - normal_z * normal_z;
	float length_xy = sqrtf(length2_xy);
	for (int i = 4; i < input.size(); i++) {		
		// get curt point
		vec3 curt_point = input.getPoint(i);
		vec3 pre_point = i == 4 ? curt_point : input.getPoint(i - 1);
		vec3 next_point = i == input.size() - 1 ? curt_point : input.getPoint(i + 1);
		// compute the new origin, normal; calculate the new radius
		// TODO:: compute origin with two edges
		vec3 origin = pre_circle->getOrigin() + (curt_point - pre_point) + length(curt_point - pre_point) / length_xy * normal_z * vec3(0.0, 0.0f, -1.0f);
		// TODO:: curt - pre is not correct
		vec3 normal_2d = normalize(next_point - pre_point);
		vec3 perpend = normalize(cross(normal_2d, camera_direction));
		// at this time contour vector should have been inited
		// compute the radius by shooting ray at each contour point
		float max = std::numeric_limits<float>::max(); 
		float min = -std::numeric_limits<float>::max();

		for (int j = 0; j < contours.size(); j++) {
			vec3 ray = contours[j] - curt_point;
			vec3 ray_norm = normalize(ray);
			if (abs(dot(ray_norm, normal_2d)) < TH_DOT_ERR) {
				float projection = dot(ray, perpend);
				//TODO:
				if (projection > 0) {
					max = fminf(max, projection);
				}else{ 
					min = fmaxf(min, projection); 
				}
			}
		}

		float radius = pre_circle->getRadius();
		if (max != 0.0f && min != 0.0f) radius = 0.5f * (max - min);
		else if (max != 0.0f && i) radius = max;
		else if (min != 0.0f && i) radius = -min;
		// else remain the radius
		if(radius > TH_MAX_NUMBER_OF * pre_circle->getRadius()) radius = pre_circle->getRadius();
		// construct a new circle 
		// TODO:: current normal is just for 2d
		// REMEMBER THE '-' HERE!!!!
		//if (i == input.size() - 1) {
		//	radius = pre_circle->getRadius();
		//	normal_2d = -pre_circle->getNormal();
		//}
		vec3 normal = normalize(length_xy * normal_2d - vec3(0.0f, 0.0f, normal_z));
		Circle* new_circle = nullptr;
		// update circle
		if (i - 3< history.size()) {
			new_circle = (Circle*)history[i - 3];
			new_circle->setOrigin(origin);
			new_circle->setNormal(-normal);
			new_circle->setRadius(radius);
		}
		// add circle
		else {
			new_circle = new Circle(origin, radius, -normal);
			// push the previous circle to the history list
			history.push_back(new_circle);
		}
		// get the circle pointer of previous one
		pre_circle = new_circle;		
	}	
	return true;
}

void Solution::add_point(const vec3 & point)
{
	input.push(point);
}

void Solution::set_camera_direction(const vec3 & cd)
{
	camera_direction = cd;
}

void Solution::test(const vec3 & input)
{
	test(input, "TEST");
}

void Solution::test(const vec3 & input, char * name)
{
	std::cout << name << ": " << input.x << ", " << input.y << ", " << input.z << std::endl;
}

void Solution::set_contours(std::string filename)
{
	std::ifstream infile(filename);
	int rows, cols;
	infile >> rows >> cols;
	int x, y;
	while (infile >> x >> y) {
		contours.push_back(vec3(0.01 * (y - cols/2.0) , 0.01 *(rows/2.0 - x), 0.0));
	}
	infile.close();
}
