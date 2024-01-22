#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

#include "color.h"
#include "hittable.h"
#include "material.h"

#include <iostream>

// Constructs and dispatches rays into world, renders image
class camera {
	public:
		double aspect_ratio      = 1.0; // Ratio of image width over height
		int    image_width       = 100; // Rendered image width in pixel count
		int    samples_per_pixel = 10;  // Count of random samples per pixel
		int    max_depth         = 10;  // Maximum number of ray bounces into scene

		double vfov     = 90;               // Vertical view angle (field of view)
		point3 lookfrom = point3(0, 0, -1); // Point camera is looking from (camera origin)
		point3 lookat   = point3(0, 0, 0);  // Point camera is looking at
		vec3 vup        = vec3(0, 1, 0);    // Camera-relative "up" direction

		double defocus_angle = 0;  // Variation angle of rays through each pixel
		double focus_dist    = 10; // Distance from camera lookfrom point to plane of perfect focus

		void render(const hittable &world) {
			initialize();

			std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

			// Write pixels left to right, top to bottom
			for (int j = 0; j < image_height; ++j) {
				std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;

				for (int i = 0; i < image_width; ++i) {
					color pixel_color(0, 0, 0);
					for (int sample = 0; sample < samples_per_pixel; ++sample) {
						ray r = get_ray(i, j);
						pixel_color += ray_color(r, max_depth, world);
					}

					write_color(std::cout, pixel_color, samples_per_pixel);
				}
			}

			std::clog << "\rDone.                 \n";
		}

	private:
		int	   image_height;   // Rendered image height
		point3 center;		   // Camera center
		point3 pixel00_loc;	   // Location of pixel (0, 0)
		vec3   pixel_delta_u;  // Offset to pixel to the right
		vec3   pixel_delta_v;  // Offset to pixel below
		vec3   u, v, w;        // Camera frame basis vectors
		vec3   defocus_disk_u; // Defocus disk horizontal radius
		vec3   defocus_disk_v; // Defocus disk vertical radius

		void initialize() {
			// Calculate the image height, and ensure that it's at least 1
			image_height = static_cast<int>(image_width / aspect_ratio);
			image_height = (image_height < 1) ? 1 : image_height; // Clamp to 1

			center = lookfrom;
		
			// Determine viewport dimensions
			double theta = degrees_to_radians(vfov);
			auto h = tan(theta/2);
			double viewport_height = 2 * h * focus_dist;
			double viewport_width = viewport_height * (static_cast<double>(image_width) / image_height); // Calculate with actual image aspect ratio

			// Calculate the u, v, w unit basis vectors for the camera coordinate frame
			w = unit_vector(lookfrom - lookat);
			u = unit_vector(cross(vup, w)); // Right
			v = cross(w, u);                // Up

			// Calculate the vectors across the horizontal and down the vertical viewport edges
			vec3 viewport_u = viewport_width * u;   // Vector across viewport horizontal edge
			vec3 viewport_v = viewport_height * -v; // Viewport across viewport vertical edge

			// Calculate the horizontal and vertical delta vectors from pixel to pixel
			pixel_delta_u = viewport_u / image_width;
			pixel_delta_v = viewport_v / image_height;

			// Calculate the location of the upper left pixel
			point3 viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2; // Camera vector goes through center of viewport
			pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);  // Scale pixel center offset by actual pixel deltas

			// Calculate the camera defocus disk basis vectors
			double defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle/2));
			defocus_disk_u = u * defocus_radius;
			defocus_disk_v = v * defocus_radius;
		}

		ray get_ray(int i, int j) const {
			// Get a randomly sampled camera ray for the pixel at location i,j, originating from
			// the camera defocus disk
			point3 pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
			point3 pixel_sample = pixel_center + pixel_sample_square();

			point3 ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
			vec3 ray_direction = pixel_sample - ray_origin;

			return ray(ray_origin, ray_direction);
		}

		vec3 pixel_sample_square() const {
			// Returns a random point in the square surrounding a pixel at the origin
			double px = -0.5 + random_double();
			double py = -0.5 + random_double();
			return (px * pixel_delta_u) + (py * pixel_delta_v);
		}

		point3 defocus_disk_sample() const {
			// Returns a random point in the camera defocus disk
			point3 p = random_in_unit_disk();
			return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
		}

		color ray_color(const ray &r, int depth, const hittable &world) const {
			hit_record rec;

			// If we've exceeded the ray bounce limit, no more light is gathered
			if (depth <= 0)
				return color(0, 0, 0);

			if (world.hit(r, interval(0.001, infinity), rec)) { // Ignore hits that are very close to intersection point
				ray scattered;
				color attenuation;
				if (rec.mat->scatter(r, rec, attenuation, scattered)) {
					return attenuation * ray_color(scattered, depth-1, world);
				}
				return color(0, 0, 0);
			}

			// Blue-white gradient background
			vec3 unit_direction = unit_vector(r.direction());
			double a = 0.5 * (unit_direction.y() + 1.0); // Scale+bias to [0, 1]
			return (1.0 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
		}
};

#endif