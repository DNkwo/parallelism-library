// compile with
// g++ convolution.cpp -lpng

#include "para-pat/include/Farm.hpp"
#include "para-pat/include/Pipeline.hpp"
#include "para-pat/include/Pipe.hpp"

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


int dim, nr_cpu_w, nr_gpu_w;
const int mask_dim=8;

unsigned short **images;
unsigned short **masks;
unsigned short **out_images;
int nr_images=100;

typedef std::string* string_p;

using namespace std;

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

void* read_image_and_mask(void* arg) {
  string_p image_name_p = static_cast<string_p>(arg);
  task_t* task = new task_t;
  task->image = read_image(image_name_p->c_str(), dim);
  task->mask = new unsigned short[mask_dim*mask_dim]();
  float val = 1.0f/(mask_dim * 2.0f - 1.0f);
  unsigned short y = mask_dim/2;
  for(int j=0; j < mask_dim; j++) 
    task->mask[y*mask_dim + j] = val;
  unsigned short x = mask_dim/2;
  for(int j=0; j < mask_dim; j++) 
    task->mask[j*mask_dim + x] = val;

  return task;

}

void* process_image(void* arg) {
  task_t* task = static_cast<task_t*>(arg); //extract arguments from struct
  unsigned short *in_image = task->image;
  unsigned short *mask = task->mask;
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

int i=0;

int main(int argc, char * argv[]) {

  string_p image_name_p;
  unsigned int nr_images, pattern, do_chunking, min_chunk_size;
  int i=0;
  // if (argc<3)
  //  std::cerr << "use: " << argv[0] << " <imageSize> <nrImages> [<chunking>]\n";
  dim = 1024 ; // atoi(argv[1]);
  nr_images = 50 ; // atoi(argv[2]);
  
  images = new unsigned short *[nr_images];
  masks = new unsigned short *[nr_images];
  out_images = new unsigned short *[nr_images];
  unsigned short * mask = new unsigned short ;
  int N[nr_images];

  for (int i=0; i<nr_images; i++) {
	  N[i] = i;
	 out_images[i] = new unsigned short[dim*dim];
  }


  Pipeline pipeline;

  // Pipe pipe1(read_image_and_mask);
  Pipe pipe2(read_image_and_mask);

  Farm farm1(2, process_image);
  // Farm farm2(6, process_image); // Adjust the number of workers as needed

  pipeline.addStage(&pipe2);
  pipeline.addStage(&farm1);


  i = 0;

  const int num_runs = 3;
  double total_time = 0.0;

    for (int run = 0; run < num_runs; ++run) {
      
        ThreadSafeQueue<Task> inputQueue;
        for (i = 0; i < nr_images; i++) {
          string_p image_name_p = get_image_name(N[i]);
          Task task(image_name_p);
          inputQueue.enqueue(task);
        }

        double beginning = get_current_time();
        ThreadSafeQueue<Result> outputQueue = pipeline.execute(inputQueue);
        double end = get_current_time();

        double run_time = (end - beginning);

        std::cout << "Time taken for run: " << run << " is " <<  run_time << " seconds" << std::endl;

        total_time += run_time;

        // Process the results (if needed)
        i = 0;
        while (!outputQueue.empty()) {
            Result result;
            if (outputQueue.dequeue(result)) {
                out_images[i] = static_cast<unsigned short*>(result.data);
                i++;
            }
        }
    }

    double average_runtime = total_time / num_runs;
    std::cout << "Average Runtime: " << average_runtime << " seconds" << std::endl;

  return 0;
}




