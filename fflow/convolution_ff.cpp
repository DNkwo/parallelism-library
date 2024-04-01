// compile with
// g++ convolution_ff.cpp -o convolution_ff -lpng -pthread -I/path/to/fastflow

#include <ff/farm.hpp>
#include <ff/pipeline.hpp>
#include <png.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <string>
#include <fstream>
#include <sys/time.h>

using namespace std;
using namespace ff;


double get_current_time()
{
  static int start = 0, startu = 0;
  struct timeval tval;
  double result;
  
  if (gettimeofday(&tval, NULL) == -1)
    result = -1.0;
  else if(!start) {
    start = tval.tv_sec;
    startu = tval.tv_usec;
    result = 0.0;
  }
  else
    result = (double) (tval.tv_sec - start) + 1.0e-6*(tval.tv_usec - startu);
  
  return result;
}

unsigned short *read_image(const char *fileName, png_uint_32 height) {
  int i, header_size = 8, is_png;
  char header[8];
  FILE *fp = fopen(fileName,"rb");
  png_structp png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr), end_info = png_create_info_struct(png_ptr);
  png_bytep raw_data;
  png_bytepp row_pointers;
  png_uint_32 row_bytes;

  fread (header, 1, header_size, fp);
  is_png = !png_sig_cmp((png_bytep)header,0,header_size);
  if (!is_png) { printf("not a png\n"); return(NULL);}
  png_init_io(png_ptr,fp);
  png_set_sig_bytes(png_ptr,header_size);
  png_read_info(png_ptr, info_ptr);
  row_bytes = png_get_rowbytes(png_ptr, info_ptr);
  raw_data = (png_bytep) png_malloc(png_ptr, height*row_bytes);
  row_pointers = (png_bytepp) png_malloc(png_ptr, height*sizeof(png_bytep));
  for (i=0; i<height; i++)
    row_pointers[i] = raw_data+i*row_bytes;
  png_set_rows(png_ptr, info_ptr, row_pointers);
  png_read_rows(png_ptr,row_pointers,NULL,height);

  return (unsigned short *)raw_data;

}

string_p get_image_name(int i) {
	string_p image_n = new string("images/image" + to_string(i) + ".png");
    // *image_name = "images/image" + to_string(i) + ".png";
    // strcpy(image_name_c, image_name.c_str());
	cout << *image_n << endl;
    return image_n;
}
typedef struct {
  unsigned short* image;
  unsigned short* mask;
} task_t;

task_t read_image_and_mask(string_p image_name_p) {
  task_t task;
  task.image = read_image(image_name_p->c_str(), dim);
  task.mask = new unsigned short[mask_dim*mask_dim]();
  float val = 1.0f/(mask_dim * 2.0f - 1.0f);
  unsigned short y = mask_dim/2;
  for(int j=0; j < mask_dim; j++) 
    task.mask[y*mask_dim + j] = val;
  unsigned short x = mask_dim/2;
  for(int j=0; j < mask_dim; j++) 
    task.mask[j*mask_dim + x] = val;

  return task;

}

unsigned short * process_image(task_t task) {
  unsigned short *in_image = task.image;
  unsigned short *mask = task.mask;
	unsigned short * out_image = new unsigned short[dim*dim];
  int vstep = mask_dim/2;
  int hstep = mask_dim/2;
  float sumFX;
  int left,right,top,bottom,mask_index,index;
  for(int x = 0; x < dim; x++)
    for(int y = 0; y < dim; y++) {
      left    = (x           <  vstep) ? 0         : (x - vstep);
      right   = ((x + vstep - 1) >= dim) ? dim - 1 : (x + vstep - 1); 
      top     = (y           <  hstep) ? 0         : (y - hstep);
      bottom  = ((y + hstep - 1) >= dim)? dim - 1  : (y + hstep - 1); 
      sumFX = 0;
      
      for(int j = left; j <= right; j++)
        for(int k = top ; k <= bottom; k++) {
          mask_index = (k - (y - hstep)) * mask_dim  + (j - (x - vstep));
          index     = k                 * dim      + j;
          sumFX += ((float)in_image[index] * mask[mask_index]);
        }
      
      sumFX += 0.5f;

      out_image[y*dim + x] = (unsigned short) sumFX;
    }

    return out_image;
}

struct ReadImageTask: ff_node_t<string_p, task_t> {
  task_t* svc(string_p* image_name_p) {
    task_t task = read_image_and_mask(*image_name_p);
    return new task_t(task);
  }
};

struct ProcessImageTask: ff_node_t<task_t, unsigned short*> {
  unsigned short* svc(task_t* task) {
    unsigned short* out_image = process_image(*task);
    delete task;
    return out_image;
  }
};

struct WriteImageTask: ff_node_t<unsigned short*, void> {
  void svc(unsigned short* image) {
    // You can add code here to write the processed image to a file if needed
    delete[] image;
  }
};

struct Emitter: ff_node_t<int> {
  Emitter(int nr_images) : nr_images(nr_images) {}

  int svc(int) {
    for (int i = 0; i < nr_images; i++) {
      string_p image_name_p = get_image_name(i);
      ff_send_out(image_name_p);
    }
    return EOS;
  }

  int nr_images;
};

int main(int argc, char* argv[]) {
  // ... (keep the existing code for parsing command-line arguments and initializing variables)

  double beginning = get_current_time();

  ff_Farm<> read_farm(true, 2);
  vector<ff_node*> read_workers;
  for (int i = 0; i < 2; ++i) {
    read_workers.push_back(new ReadImageTask());
  }
  read_farm.add_workers(read_workers);

  ff_Farm<> process_farm(true, 6);
  vector<ff_node*> process_workers;
  for (int i = 0; i < 6; ++i) {
    process_workers.push_back(new ProcessImageTask());
  }
  process_farm.add_workers(process_workers);

  Emitter emitter(nr_images);
  WriteImageTask writer;

  ff_Pipe<> pipe(emitter, read_farm, process_farm, writer);
  pipe.run_and_wait_end();

  double end = get_current_time();

  cout << "Runtime is " << end - beginning << endl;

  // ... (keep the existing code for cleanup and returning)
}