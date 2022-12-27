#pragma once
#include <iostream>
#include <stdio.h>
#include <libpng16/png.h>
#include <string>
#include <random>
#include <chrono>

struct point
{
	int x, y;
};

int ipart(float x);
int round(int x);
float fpart(float x);
float rfpart(float x);
bool is_steep(int& x0, int& y0, int& x1, int& y1);
png_byte get_pixel(png_byte** row_ptr, int x, int y, int depth_ratio);
int random(int lim);

point find_darkest_point(png_byte** row_ptr, int width, int height, int depth_ratio);

/*
* Using Xiaolin Wu line algorithm, gets sum of the color values of the pixels
* touching a full line going through (x0,y0), (x1,y1)
* Smaller value means the line goes through darker pixels
*/
float calculate_line_value(png_byte** row_ptr, png_byte** buffer, int width, int height, int x0, int y0, int x1, int y1);

/*
* Extends lines from two points to span entire canvas, and
* returns the two endpoints;
*/
void find_edges(int& x0, int& y0, int& x1, int& y1, int width, int height);

/*
* Draws a full line with Xiaolin Wu line algorithm
*/
void draw_line(png_byte** row_ptr, png_byte** buffer, int width, int height, int x0, int y0, int x1, int y1);