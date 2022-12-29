#include "Util.h"


int in_dr = 3;
int out_dr = 2;

int ipart(float x) {
        return (int)floor(x);
}

int round(int x) {
    return (int)ipart(x + 0.5f);
}

float fpart(float x) {
    return x - floor(x);
}

float rfpart(float x) {
    return 1 - fpart(x);
}

png_byte get_pixel(png_byte** row_ptr, int x, int y, int depth_ratio) {
    return row_ptr[y][x * depth_ratio];
}

std::default_random_engine dre((unsigned int)(std::chrono::steady_clock::now().time_since_epoch().count()));     // provide seed
int random(int lim)
{
    std::uniform_int_distribution<int> uid{ 0,lim };   // help dre to generate nos from 0 to lim (lim included);
    return uid(dre);    // pass dre as an argument to uid to generate the random no
}


point find_darkest_point(png_byte** row_ptr, int width, int height, int depth_ratio) {

    std::vector<point> dark_points = { {0,0} };
    png_byte darkest = get_pixel(row_ptr, 0, 0, depth_ratio), temp;

    for (int y = 1; y < height; ++y) {
        for (int x = 1; x < width; ++x) {
            temp = get_pixel(row_ptr, x, y, depth_ratio);
            if (temp == darkest) {
                dark_points.push_back({ x,y });
                
            }
            else if (temp < darkest) {
                darkest = temp;
                dark_points = { {x,y} };
            }
        }
    }
    
    point p = dark_points[random(dark_points.size() - 1)];
    return p;
}


float calculate_line_value(png_byte** row_ptr, png_byte** buffer, int width, int height, int x0, int y0, int x1, int y1) {
    
    long value = 0;
    int pixel_count = 4;

    find_edges(x0, y0, x1, y1, width, height);
    
    bool steep = is_steep(x0, y0, x1, y1);

    int dx = x1 - x0;
    int dy = y1 - y0;
    float gradient = (dx == 0) ? 1.0f : (float)dy / dx;

    // First endpoint
    int xe = round(x0);
    float ye = y0 + gradient * (xe - x0);
    float xg = rfpart(x0 + 0.5f); 
    int xpxl1 = xe;
    int ypxl1 = ipart(ye);
    
    if (steep) {
        value += row_ptr[xpxl1][ypxl1 * in_dr];
        value += row_ptr[xpxl1 + 1][ypxl1 * in_dr];
    }
    else {
        value += row_ptr[ypxl1][xpxl1 * in_dr];
        value += row_ptr[ypxl1][(xpxl1 + 1) * in_dr];
    }

    float intery = ye + gradient;

    // Second endpoint
    xe = round(x1);
    ye = y1 + gradient * (xe - x1);
    xg = fpart(x1 + 0.5f);
    int xpxl2 = xe;
    int ypxl2 = ipart(ye);

    if (steep) {
        value += row_ptr[xpxl1][ypxl1 * in_dr];
        value += row_ptr[xpxl1 + 1][ypxl1 * in_dr];
    }
    else {
        value += row_ptr[ypxl2][xpxl2 * in_dr];
        value += row_ptr[ypxl2][(xpxl2 + 1) * in_dr];
    }

    // Main loop
    if (steep) {
        for (int x = xpxl1 + 1; x < xpxl2 - 1; ++x) {
            value += row_ptr[x][ipart(intery) * in_dr];
            value += row_ptr[x + 1][ipart(intery) * in_dr];
            pixel_count += 2;
            intery += gradient;
        }
    }
    else {
        for (int x = xpxl1 + 1; x < xpxl2 - 1; ++x) {
            value += row_ptr[ipart(intery)][x * in_dr];
            if (intery < height)
            {
                pixel_count += 2;
                value += row_ptr[ipart(intery) + 1][x * in_dr]; 
            }
            
                
            intery += gradient;
        }
    }
    
    return (((float)value / pixel_count));
}


void draw_line(png_byte** row_ptr, png_byte** buffer, int width, int height, int x0, int y0, int x1, int y1) {

    find_edges(x0, y0, x1, y1, width, height);

    bool steep = is_steep(x0, y0, x1, y1); 

    int dx = x1 - x0;
    int dy = y1 - y0;
    float gradient = (dx == 0) ? 1.0f : (float)dy / dx;
    short color = 16;
    unsigned int in_color = 16;
    unsigned int r = 0;

    unsigned char col_cap = 255 - in_color;

    // First endpoint
    int xe = round(x0);
    float ye = y0 + gradient * (xe - x0);
    float xg = rfpart(x0 + 0.5f);
    int xpxl1 = xe;
    int ypxl1 = ipart(ye);
    
    if (steep) {
        if (buffer[xpxl1][ypxl1 * out_dr] >= color)
            buffer[xpxl1][ypxl1 * out_dr] -= color;
        else buffer[xpxl1][ypxl1 * out_dr] = 0;

        if (buffer[xpxl1][(ypxl1 + 1) * out_dr] >= color)
            buffer[xpxl1][(ypxl1 + 1) * out_dr] -= color;
        else buffer[xpxl1][(ypxl1 + 1) * out_dr] = 0;

        if (row_ptr[xpxl1][ypxl1 * in_dr] < col_cap) {
            row_ptr[xpxl1][ypxl1 * in_dr] += in_color;
        }

        if (row_ptr[xpxl1][ypxl1 + 1 * in_dr] < col_cap) {
            row_ptr[xpxl1][ypxl1 + 1 * in_dr] += in_color;
        }
    }
    else {
        if (buffer[ypxl1][xpxl1 * out_dr] >= color) 
            buffer[ypxl1][xpxl1 * out_dr] -= color; 
        else buffer[ypxl1][xpxl1 * out_dr] = 0;

        if (ypxl1 + 1 < height) {
            if (buffer[ypxl1 + 1][xpxl1 * out_dr] >= color)
                buffer[ypxl1 + 1][xpxl1 * out_dr] -= color;
            else buffer[ypxl1 + 1][xpxl1 * out_dr] = 0;

            if (row_ptr[ypxl1 + 1][xpxl1 * in_dr] < col_cap) {
                row_ptr[ypxl1 + 1][xpxl1 * in_dr] += in_color;
            }
        }

        if (row_ptr[ypxl1][xpxl1 * in_dr] < col_cap) {
            row_ptr[ypxl1][xpxl1 * in_dr] += in_color;
        }
    }
        

    float intery = ye + gradient;

    // Second endpoint
    xe = round(x1);
    ye = y1 + gradient * (xe - x1);
    xg = fpart(x1 + 0.5f);
    int xpxl2 = xe;
    int ypxl2 = ipart(ye);

    if (steep) {
        if (buffer[xpxl2][ypxl2 * out_dr] >= color) 
            buffer[xpxl2][ypxl2 * out_dr] -= color; 
        else buffer[xpxl2][ypxl2 * out_dr] = 0;

        if (buffer[xpxl2][(ypxl2 + 1) * out_dr] >= color)
            buffer[xpxl2][(ypxl2 + 1) * out_dr] -= color;
        else buffer[xpxl2][(ypxl2 + 1) * out_dr] = 0;

        if (row_ptr[xpxl2][ypxl2 * in_dr] < col_cap) {
            row_ptr[xpxl2][ypxl2 * in_dr] += in_color;
        }

        if (row_ptr[xpxl2][(ypxl2 + 1) * in_dr] < col_cap) {
            row_ptr[xpxl2][(ypxl2 + 1) * in_dr] += in_color;
        }
    }
    else {
        if (buffer[ypxl2][xpxl2] >= color) 
            buffer[ypxl2][xpxl2] -= color; 
        else buffer[ypxl2][xpxl2] = 0;

        if (ypxl2 + 1 < height) {
            if (buffer[ypxl2 + 1][xpxl2 * out_dr] >= color)
                buffer[ypxl2 + 1][xpxl2 * out_dr] -= color;
            else buffer[ypxl2 + 1][xpxl2 * out_dr] = 0;

            if (row_ptr[ypxl2 + 1][xpxl2 * in_dr] < col_cap) {
                row_ptr[ypxl2 + 1][xpxl2 * in_dr] += in_color;
            }
        }

        if (row_ptr[ypxl2][xpxl2 * in_dr] < col_cap) {
            row_ptr[ypxl2][xpxl2 * in_dr] += in_color;
        }
    }

    // Main loop
    if (steep) {
        for (int x = xpxl1 + 1; x < xpxl2 - 1; ++x) {
            if (buffer[x][ipart(intery) * out_dr] >= color) 
                buffer[x][ipart(intery) * out_dr] -= color; 
            else buffer[x][ipart(intery) * out_dr] = 0;

            if (buffer[x + 1][ipart(intery) * out_dr] >= color) 
                buffer[x + 1][ipart(intery) * out_dr] -= color; 
            else buffer[x + 1][ipart(intery) * out_dr] = 0;

            if (row_ptr[x][ipart(intery) * in_dr] < col_cap) {
                row_ptr[x][ipart(intery) * in_dr] += in_color;
            }

            if (row_ptr[x + 1][ipart(intery) * in_dr] < col_cap) {
                row_ptr[x + 1][ipart(intery) * in_dr] += in_color;
            }
            intery += gradient;
        }
    }
    else {
        for (int x = xpxl1 + 1; x < xpxl2 - 1; ++x) {
            if (buffer[ipart(intery)][x * out_dr] >= color) {
                buffer[ipart(intery)][x * out_dr] -= color;
            }
            else {
                buffer[ipart(intery)][x * out_dr] = 0;
            }

            if (intery < height) {
                if (buffer[ipart(intery) + 1][x * out_dr] >= color)
                    buffer[ipart(intery) + 1][x * out_dr] -= color;
                else buffer[ipart(intery) + 1][x * out_dr] = 0;

                if (row_ptr[ipart(intery) + 1][x * in_dr] < col_cap) {
                    row_ptr[ipart(intery) + 1][x * in_dr] += in_color;
                }
            } 
            
            if (row_ptr[ipart(intery)][x * in_dr] <= col_cap) {
                row_ptr[ipart(intery)][x * in_dr] += in_color;
            }
           
            intery += gradient;
        }
    }

}

void find_edges(int& x0, int& y0, int& x1, int& y1, int width, int height) {
    int dx, dy, bx, by;
    float tx, ty;

    // Find first edge
    dx = x0 - x1;
    dy = y0 - y1;
    bx = (dx > 0) ? width : 0;
    by = (dy > 0) ? height : 0;
    if (dx == 0) {
        y1 = by;
    }
    else if (dy == 0) {
        x1 = bx;
    }
    else {
        tx = (float)(bx - x1) / dx;
        ty = (float)(by - y1) / dy;
        if (tx <= ty) {
            x1 = bx;
            y1 = y1 + (int)(tx * dy);
        }
        else {
            y1 = by;
            x1 = x1 + (int)(ty * dx);
        }
    }

    // Find other edge
    dx *= -1;
    dy *= -1;
    bx = (dx > 0) ? width : 0;
    by = (dy > 0) ? height : 0;
    if (dx == 0) {
        y0 = by;
    }
    else if (dy == 0) {
        x0 = bx;
    }
    else {
        tx = (float)(bx - x0) / dx;
        ty = (float)(by - y0) / dy;
        if (tx <= ty) {
            x0 = bx;
            y0 = y0 + (int)(tx * dy);
        }
        else {
            y0 = by;
            x0 = x0 + (int)(ty * dx);
        }
    }
}

bool is_steep(int& x0, int& y0, int& x1, int& y1) {
    bool steep = abs(y1 - y0) > abs(x1 - x0);


    if (steep) {
        // swap (x0,y0) (x1,y1)
        x0 += y0;
        y0 = x0 - y0;
        x0 -= y0;

        x1 += y1;
        y1 = x1 - y1;
        x1 -= y1;
    }

    if (x0 > x1) {
        //swap (x0,x1) (y0,y1)
        x0 += x1;
        x1 = x0 - x1;
        x0 -= x1;

        y0 += y1;
        y1 = y0 - y1;
        y0 -= y1;
    }
    return steep;
}