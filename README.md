# Ray Tracing

<img src="https://github.com/ngol0/RayTracing/blob/master/4.png" width="900" title="pic 2">

## Introduction
This project demonstrates the power and beauty of ray tracing to create realistic 3D graphics. With a clean, minimal setup from [Walnut](https://github.com/StudioCherno/Walnut), this application uses fundamental ray tracing techniques to render 3D spheres with accurate lighting, shadows, and reflections. This is made for educational purpose and the referenced sources are mentioned in the `Attribute` section.

## Features
* Ray Tracing Fundamentals: Implements essential ray tracing techniques, including ray-object intersection, shadows, and reflections.
* Realistic Lighting: Captures realistic light behavior to illuminate scenes beautifully, featuring emissive material to act as a light source.

## Screenshots
* Pic 1: light and shadow with emissive material
* Pic 2: spheres of different matierals (diffuse, emissive, mirror)

<img src="https://github.com/ngol0/RayTracing/blob/master/2.png" width="600" title="pic 1">

<img src="https://github.com/ngol0/RayTracing/blob/master/3.png" width="600" title="pic 2">

## Requirements
- [Visual Studio 2022](https://visualstudio.com) (not strictly required, however included setup scripts only support this)
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) (preferably a recent version)

## Running the project
To clone, use: `git clone --recursive https://github.com/ngol0/RayTracing`.
Once you've cloned, run `scripts/Setup.bat` to generate Visual Studio 2022 solution/project files. Hit F5 to run the project.

## Attribute
The application uses the template [Walnut](https://github.com/StudioCherno/Walnut) as a starting point. The RayTracing techniques and code references are from [Cherno's RayTracing tutorial](https://www.youtube.com/watch?v=gfW1Fhd9u9Q) and [RayTracing in One Weekend](https://raytracing.github.io/books/RayTracingInOneWeekend.html).
