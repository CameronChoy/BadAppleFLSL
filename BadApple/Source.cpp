#define _CRTDBG_MAP_ALLOC
#include <iostream>
#include <stdio.h>
#include <libpng16/png.h>
#include <string>
#include <random>
#include <chrono>
#include "Util.h"
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

#ifdef _DEBUG
#define new new( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define new new
#endif

const int OUT_BIT_DEPTH = 16;

png_infop in_info_ptr;
png_bytepp in_row_pointers;
png_structp in_png_ptr;
png_byte in_bit_depth = 24;
int depth_ratio = 3;
int out_depth_ratio = 2;
bool invert;

png_infop out_info_ptr;
png_bytepp out_row_pointers;
png_structp out_png_ptr;
png_byte** buffer;
//png_byte** buffer2;

int width = 0, height = 0;

bool read_png(const char* file_name)
{
    FILE* fp;
    if (fopen_s(&fp, file_name, "rb")) {
        return false;
    }
    in_png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    in_info_ptr = png_create_info_struct(in_png_ptr);
    if (!in_png_ptr || !in_info_ptr) {
        printf("Failed to create read pointers\n");
        return false;
    }
    png_init_io(in_png_ptr, fp);
    png_read_png(in_png_ptr, in_info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    in_row_pointers = png_get_rows(in_png_ptr, in_info_ptr);
    if (!in_row_pointers) {
        printf("Failed to get rows.\n");
        return false;
    }

    height = png_get_image_height(in_png_ptr, in_info_ptr);
    width = png_get_image_width(in_png_ptr, in_info_ptr);

    if (fp != NULL) {
        fclose(fp);
    }
    
    return true;
}

void write_png(const char* file_name)
{
    FILE* fp;
    fopen_s(&fp, file_name, "wb");

    png_init_io(out_png_ptr, fp);

    png_set_IHDR(out_png_ptr, out_info_ptr, width + 1, height + 1, OUT_BIT_DEPTH,
        PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    
    png_set_rows(out_png_ptr, out_info_ptr, buffer);
    png_write_png(out_png_ptr, out_info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_write_end(out_png_ptr, out_info_ptr);

    png_free_data(out_png_ptr, out_info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&out_png_ptr, &out_info_ptr);
    

    if (fp != NULL) {
        fclose(fp);
    }


    // Second buffer for visual dubugging
    //FILE* fp2;
    //fopen_s(&fp2, "output/0initialpoints.png", "wb");

    //png_structp out_png_ptr2 = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    //png_infop out_info_ptr2 = png_create_info_struct(out_png_ptr2);


    //png_init_io(out_png_ptr2, fp2);

    //png_set_IHDR(out_png_ptr2, out_info_ptr2, width + 1, height + 1, OUT_BIT_DEPTH,
    //    PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
    //    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);


    //png_set_rows(out_png_ptr2, out_info_ptr2, buffer2);
    //png_write_png(out_png_ptr2, out_info_ptr2, PNG_TRANSFORM_IDENTITY, NULL);
    //png_write_end(out_png_ptr2, out_info_ptr2);

    //png_free_data(out_png_ptr2, out_info_ptr2, PNG_FREE_ALL, -1);
    //png_destroy_write_struct(&out_png_ptr2, &out_info_ptr2);

    //if (fp2 != NULL) {
    //    fclose(fp2);
    //}


}




int main(int argc, char* argv[]) {
    if (argc != 6) {
        printf("Incorrect argc, need [start frame] [end frame] [line count] [sample size] [invert]\n");
        return 0;
    }
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    for (int i = 1; i < argc; ++i) {
        if (strlen(argv[i]) == 0) {
            return 1;
        }
    }
    int startf = (int)strtol((const char*)argv[1], NULL, 10);
    int endf = (int)strtol((const char*)argv[2], NULL, 10);
    int LINE_COUNT = (int)strtol((const char*)argv[3], NULL, 10);
    int SAMPLE_SIZE = (int)strtol((const char*)argv[4], NULL, 10);
    invert = (bool)strtol((const char*)argv[5], NULL, 10); // Unimplemented

    if (endf < startf) {
        printf("Invalid [endf], must be >= startf.\n");
        return 0;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    
    char filename_buffer[128];
    char read_filename_buffer[128];
    bool first = true;

    for (int f = startf; f <= endf; ++f) {

        snprintf(read_filename_buffer, 128, "res/BadApple/%i.png", f);
        if (!read_png(read_filename_buffer) || height == 0 || width == 0) {
            printf("Bad File\n");
            return 0;
        }

        out_png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        out_info_ptr = png_create_info_struct(out_png_ptr);
   
        if (first) {
            buffer = (png_byte**)png_malloc(out_png_ptr, height * OUT_BIT_DEPTH);
            //buffer2 = (png_byte**)png_malloc(out_png_ptr, height * OUT_BIT_DEPTH);
        }

        long blank_count = 0;
        for ( int y = 0; y < height; ++y) {

            if (first) {
                buffer[y] = (png_byte*)png_malloc(out_png_ptr, width * OUT_BIT_DEPTH);
                //buffer2[y] = (png_byte*)png_malloc(out_png_ptr, width * OUT_BIT_DEPTH);
            }

            for ( int x = 0; x < width; ++x) {

                buffer[y][x * out_depth_ratio] = 255;// in_row_pointers[y][(int)(x * depth_ratio)];
                //buffer2[y][x * out_depth_ratio] = 255;// in_row_pointers[y][(int)(x * depth_ratio)];
                if (get_pixel(in_row_pointers, x, y, depth_ratio) == 255) {
                    ++blank_count;
                }
                
            }
        }

        --width;
        --height;

        // White png if input is white
        if (blank_count == (height + 1) * (width+1)) {
            snprintf(filename_buffer, 128, "output/BA%i.png", f);
            write_png(filename_buffer);

            png_free_data(in_png_ptr, in_info_ptr, PNG_FREE_ALL, -1);
            png_destroy_read_struct(&in_png_ptr, &in_info_ptr, NULL);
            printf("%i/%i\n", f, endf);
            continue;
        }

        first = false;

        

        int x0 = 0, y0 = 0;
        int x1 = 0, y1 = 0, xe = 0, ye = 0;
        
        float temp = 0, b = FLT_MAX;
        point p;
        for (int l = 0; l < LINE_COUNT; ++l) {

            // Get a dark pixel    
            
            p = find_darkest_point(in_row_pointers, width, height, depth_ratio);
            x0 = p.x; 
            y0 = p.y;
            //printf("%ix%i\n", p.x, p.y);
            b = FLT_MAX; temp = 0;
            // Creating random lines and checking it's value, line with lowest value will be drawn.
            for (int iter = 0; iter < SAMPLE_SIZE; ++iter) {

                xe = x0 + (int)(1000.0 * cos(double(random(31415)) / 10000.0));
                ye = y0 + (int)(1000.0 * sin(double(random(31415)) / 10000.0));
                temp = calculate_line_value(in_row_pointers, buffer, width, height, x0, y0, xe, ye);
                if (temp < b) {
                    //printf("%f < %f\n", temp, b);
                    b = temp;
                    x1 = xe;
                    y1 = ye;
                }

            }
            //printf("%i: Draw (%i,%i) (%i,%i) %g\n", l, x0, y0, xe, ye, b);

            //// check x0 y0
            //buffer2[y0][x0 * out_depth_ratio] = get_pixel(in_row_pointers,x0,y0,depth_ratio);
            //continue;
            
            draw_line(in_row_pointers, buffer, width, height, x0, y0, x1, y1);

            //printf("%i/%i\n", l, LINE_COUNT);
        }

        //for (int y = 0; y < height; ++y) {

        //    for (int x = 0; x < width; ++x) {

        //        buffer2[y][x * out_depth_ratio] =  in_row_pointers[y][(int)(x * depth_ratio)];

        //    }
        //}

        snprintf(filename_buffer, 128, "output/BA%i.png", f);
        write_png(filename_buffer);
        
        png_free_data(in_png_ptr, in_info_ptr, PNG_FREE_ALL, -1);
        png_destroy_read_struct(&in_png_ptr, &in_info_ptr, NULL);

        printf("%i/%i\n", f, endf);
    }
    
    for (int i = 0; i < height + 1; ++i) {
        free(buffer[i]);
        buffer[i] = NULL;

        //free(buffer2[i]);
        //buffer2[i] = NULL;
    }
    //free(buffer2);
    free(buffer);
    
	return 0;
}



//for (int y = 0; y < height; y++) {
//    png_bytep row = row_pointers[y];
//
//    for (int x = 0; x < width; x++) {
//        png_bytep px = &(row[x * 4]);
//
//        for (int i = 0; i < 3; ++i)
//            px[i] *= 0.5;//+= wanted_color[i];
//    }
//}
