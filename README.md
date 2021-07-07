# 3d-Planets
A simple software-raytraced gravity simulation.

https://user-images.githubusercontent.com/35528856/124736828-af1bab00-df17-11eb-9cb5-014d08ca79d8.mp4

# How to use it
All the code is inside the "Origine.cpp" file inside 

# How it was made
This project was made using the Allegro 5 library (for the graphics part) and the GLM library (for the vector related stuff).

For the rendering part I used a simplified raytracer in order to handle better the sphere intersection
and to make the code simpler. Rather than using a full blown pixel by pixel renderer, i preferred to keep
the program low on computational effort by adopting scaled low resolution, which result in a pixelated
grid. I did not implement lighting or shadows, but in order to distinguish far and near object i made the
former darker and the latter lighter (as if there was a light in correspondence to the viewpoint).

For the physics part the only force present is the gravitational attraction
between planets (or Spheres as defined in the code). I'm still working on making the collisions between
planets more physically accurate, but in the meantime when two planets collide their gravitational attraction
is inverted, in order to push them away from each other.
