#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable {
	public:
		sphere(point3 _center, double _radius) : center(_center), radius(_radius) {}

		bool hit(const ray &r, interval ray_t, hit_record &rec) const override {
			vec3 oc = r.origin() - center;
			double a = r.direction().length_squared();
			double half_b = dot(oc, r.direction());
			double c = oc.length_squared() - radius * radius;

			double discriminant = half_b * half_b - a * c;
			if (discriminant < 0) return false; // No real solutions
			double sqrtd = sqrt(discriminant);

			// Find the nearest root that lies in the acceptable range
			double root = (-half_b - sqrtd) / a;
			if (!ray_t.surrounds(root)) {
				root = (-half_b + sqrtd) / a;
				if (!ray_t.surrounds(root)) {
					return false;
				}
			}

			rec.t = root;
			rec.p = r.at(rec.t);
			vec3 outward_normal = (rec.p - center) / radius;
			rec.set_face_normal(r, outward_normal);

			return true;
		}

	private:
		point3 center;
		double radius;
};

#endif
