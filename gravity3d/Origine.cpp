#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <glm/glm.hpp>


/*
	Program made by Sir.Rosticciano

	A simple gravity simulation made with allegro 5
	and glm.

	note: handy stuff at line 160
*/


#define W 90	
#define H 70
const int tileSize = 12;
// the display is made up of a resolution (WxH)
// and a tile size for each square in the screen

#define G 9.8   // gravitational constant
#define L 280   // luminosity --> how far an object has to be to become completely black
const ALLEGRO_COLOR ambient = al_map_rgb(0, 0, 0);	// ambient color
const float FPS = 40;	// frames per second

glm::vec3 camera = { W / 2, H / 2, -100 };	// viewpoint
// the camera is always pointing towards the x = 0, y = 0 plane.




// planets class
class Sphere {
public:
	glm::vec3 pos; //position
	double mass;
	ALLEGRO_COLOR color;
	glm::vec3 totalVel = { 0,0,0 }; // resultant of all forces


	Sphere(glm::vec3 Position, double Mass, ALLEGRO_COLOR Color) {
		pos = Position;
		mass = Mass;
		color = Color;
	}


	void move(Sphere* B) {
		Sphere c = *B;
		glm::vec3 temp = c.pos - pos; //distance between spheres

		if (glm::length(temp) > c.mass + mass) {	// spheres dont intersect
			double V = (G * c.mass) / pow(glm::length(temp), 2);
			temp = glm::normalize(temp);
			temp *= V;
			totalVel += temp;
		}
		else {	// spheres intersect
			double V = (G * c.mass) / pow(glm::length(temp), 2);
			temp = glm::normalize(temp);
			temp *= V;
			totalVel -= temp;
			//note: This is the part where the bouncy effect happens, which isn't meant to be realistic
			//		but is designed to be simple and keep the simulation alive.
		}
	}


};



//returns color to render and distance from camera
ALLEGRO_COLOR SphereIntersect(glm::vec3 A, glm::vec3 B, Sphere s, double *dist) {

	//calculate only if inside the field of view --> the objects behind the camera won't be rendered
	if (s.pos.z > camera.z) {
		double a = pow((B.x - A.x), 2) + pow((B.y - A.y), 2) + pow((B.z - A.z), 2);
		double b = -2 * ((B.x - A.x) * (s.pos.x - A.x) + (B.y - A.y) * (s.pos.y - A.y) + (s.pos.z - A.z) * (B.z - A.z));
		double c = pow((s.pos.x - A.x), 2) + pow((s.pos.y - A.y), 2) + pow((s.pos.z - A.z), 2) - pow(s.mass, 2);

		//discriminant between line and sphere
		float delta = (float)(pow(b, 2) - 4 * a * c);

		if (delta < 0)	// no intersection
			return ambient;
		else {
			float t = (-b - sqrt(delta)) / (2 * a);
			glm::vec3 pt = A + (B - A) * t;	//calculate point of intersection
			*dist = glm::length(A - pt);	// distance from camera is the magnitude of vector A-pt
			return s.color;
		}
	}
}



int main(int argc, char* argv[])
{
	srand(time(NULL));

	ALLEGRO_DISPLAY* display = NULL;
	ALLEGRO_EVENT_QUEUE* event_queue = NULL;
	ALLEGRO_TIMER* timer = NULL;

	bool running = true;
	bool redraw = true;

	// Initialize allegro
	if (!al_init()) {
		fprintf(stderr, "Failed to initialize allegro.\n");
		return 1;
	}

	// Initialize the timer
	timer = al_create_timer(1.0 / FPS);
	if (!timer) {
		fprintf(stderr, "Failed to create timer.\n");
		return 1;
	}

	// Create the display
	
	display = al_create_display(W * tileSize, H * tileSize);

	if (!display) {
		fprintf(stderr, "Failed to create display.\n");
		return 1;
	}

	
	al_set_window_title(display, "3d Planets");

	// Create the event queue
	event_queue = al_create_event_queue();
	if (!event_queue) {
		fprintf(stderr, "Failed to create event queue.");
		return 1;
	}

	// Register event sources
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_init_primitives_addon();

	// Display a black screen
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_flip_display();

	// Start the timer
	al_start_timer(timer);


	// This is where the planets are generated: modify the for loop to change the planets number and settings.
	std::vector<Sphere> spheres;
	for (int i = 0; i < 4; i++) {
		glm::vec3 tempPosition = { rand() % W, rand() % H, rand() % 200 + 10 };
		double tempMass = (double)(rand() % 10 + 5);
		ALLEGRO_COLOR tempColor = al_map_rgb(0, 255, 255);
		spheres.push_back(Sphere(tempPosition, tempMass, tempColor));
	}

	//Uncomment to generate a little and ready sun
	//spheres.push_back(Sphere({W / 2, H / 2, 100}, 50, al_map_rgb(255, 0, 0)));

	// a copy of spheres wich holds the next instant to render
	std::vector<Sphere> newSpheres = spheres;


	// Game loop
	while (running) {
		ALLEGRO_EVENT event;
		ALLEGRO_TIMEOUT timeout;

		// Initialize timeout
		al_init_timeout(&timeout, 0.06);

		// Fetch the event (if one exists)
		bool get_event = al_wait_for_event_until(event_queue, &event, &timeout);

		// Handle the event
		if (get_event) {
			switch (event.type) {
			case ALLEGRO_EVENT_TIMER:
				redraw = true;
				break;
			case ALLEGRO_EVENT_DISPLAY_CLOSE:
				running = false;
				break;
			default:
				fprintf(stderr, "Unsupported event received: %d\n", event.type);
				break;
			}
		}

		// Check if we need to redraw
		if (redraw && al_is_event_queue_empty(event_queue)) {

			al_clear_to_color(ambient);
			spheres = newSpheres;


			//Physics part
			for (int i = 0; i < spheres.size(); i++) {
				for (int j = 0; j < spheres.size(); j++) {
					if (i != j) // calculate attraction between every sphere, except for the current (in order not to self attract)
						newSpheres[i].move(&spheres[j]);
				}
				newSpheres[i].pos += newSpheres[i].totalVel;	// add the resultant to the position in order to move the planet
			}
			

			// Raytracing part
			// j and i are the coordinates of the current tile
			for (int i = 0; i < H; i++) {
				for (int j = 0; j < W; j++) {

					glm::vec3 temp = { j,i,0 };	// current point where the line is passing through
					ALLEGRO_COLOR final; // output color

					std::vector<double> distances;	// holds distance of all intersections

					for (Sphere s : spheres) {	// For each sphere calculate distance of the intersection from the camera.
												// If no intersection distance will be 0, thus not rendered.
						double tempDist = 0;
						SphereIntersect(camera, temp, s, &tempDist);
						distances.push_back(tempDist);
					}
					

					// store all elements in newDistances and remove all distances equal to 0
					std::vector<double> newDistances = distances;
					newDistances.erase(std::remove(newDistances.begin(), newDistances.end(), 0), newDistances.end());


					// if there are intersections
					if (newDistances.size() > 0) {
						double minValue = *std::min_element(newDistances.begin(), newDistances.end());	// nearest intersection point

						std::vector<double>::iterator it = std::find(distances.begin(), distances.end(), minValue);
						int index = std::distance(distances.begin(), it);	// find the index of minValue inside distances

						// once the index is found, the rectangle will take the color of the corresponding sphere
						ALLEGRO_COLOR col = spheres[index].color;

						// shading the color --> the farther the darker
						col.r *= 1 - distances[index]/L;
						col.g *= 1 - distances[index]/L;
						col.b *= 1 - distances[index]/L;

						al_draw_filled_rectangle(j * tileSize, i * tileSize, j * tileSize + tileSize, i * tileSize + tileSize,
							col);
					}
				}
			}
			al_flip_display();
			redraw = false;
		}
	}

	// Clean up
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);

	return 0;
}