/*
 * Copyright (c) 2017 D3 Engineering
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 /**
 * Main application for V4L2 capture and OpenGL display tests and utilities.
 * @file main.c
 *
 * @copyright
 * Copyright (c) 2017 D3 Engineering
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/queue.h>

#include "options.h"
#include "capture.h"
#include "display.h"
#include "log.h"
int VERBOSE = LOG_INFO;

struct options g_program_options = {0};

//ali

#include <assert.h>
#include <iostream>
#include <vector>
#include "tensorflow/c/c_api.h"



TF_Buffer* read_file(const char* file); 
void free_buffer(void* data, size_t length) {                                             
        free(data);  
}
TF_Buffer* read_file(const char* file) {                                                  
  FILE *f = fopen(file, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);                                                                  
  fseek(f, 0, SEEK_SET);  //same as rewind(f);                                            

  void* data = malloc(fsize);                                                             
  fread(data, fsize, 1, f);
  fclose(f);

  TF_Buffer* buf = TF_NewBuffer();                                                        
  buf->data = data;
  buf->length = fsize;                                                                    
  buf->data_deallocator = free_buffer;                                                    
  return buf;
} 
//ali */	
/* Setup the catpure display function as a progam usage. */
static struct usage capture_display = {
		.name="CAPTURE_DISPLAY",
		.description = "Capture V4L2 buffers and OpenGLES YUV shader display",
		.function = capture_and_display,
};

/**
 * Initialize the queue of program tests before tests are added.
 * @note The contstructor attribute is a GCC extension.
 */
__attribute__((constructor (PRIORITY_SETUP))) void init_global(void)
{
	TAILQ_INIT(&g_program_options.usage_head);
}

/**
 * Initialize the default application function capture_and_display.
 * @note The contstructor attribute is a GCC extension.
 */
__attribute__((constructor (PRIORITY_DEFAULTS))) void add_default_usage(void)
{
	insert_usage(&capture_display, true);
}

/**
 * Print the available program command line options.
 * @param argv the list of program arguments.
 */
void usage(char * const argv[])
{
	printf("%s - V4L2 capture, OpenGL Display and test\n", argv[0]);
	printf("-d <device>, --device v4l2 device for streaming\n");
	printf("-s <sub-device>, --subdevice v4l2 subdevice device for options\n");
	printf("-p #,  --test-pattern # test pattern to capture instead of live video\n");
	printf("-u TYPE,  --usage TYPE program use to return\n");
	printf("\tSupport values:\n");
	struct usage *usage;
	TAILQ_FOREACH(usage, &g_program_options.usage_head, usage_entry)
		printf("\t%s - %s\n", usage->name, usage->description);
	printf("-v, --verbose\n");
	printf("-e , --eglimage  !=1 for zero copy buffer =1 for copy v4L2 buffer to memory(glTexSubImage2D)  default=zero copy\n");
	printf("-0, --win_width  the width  for display/view window  default=1440\n");
	printf("-1, --win_height the height for display/view window  default=810\n");
	printf("-2, --im_width  the width  for texture  default=1920\n");
	printf("-3, --im_height the height for texture  default=1080\n");
	printf("-4 , --rgbtext  !=1 first copy image to frame buffer texture as rgb then render to display  =1 for copy to display directly\n");
	printf("-5 , --rgbinput  !=1 camera input is rgb  =1 camera input is nv12\n");
}

/**
 * Assign the command line options that should be used by default for options not set by the user.
 * @param opt data structure that contains all user options.
 */
void set_default_options(struct options *opt)
{
	opt->capture_count = DEFAULT_COUNT;
	opt->dev_name = (char*)DEFAULT_DEVICE;
	opt->subdev_name = (char*)DEFAULT_SUBDEVICE;
	opt->buffer_count = 3;//DEFAULT_BUFFER_COUNT;
	opt->program_use = opt->default_usage;
	opt->dma_export = false;//true;
	opt->ddump=false;
	opt->eglimage=false;
	opt->rgbtext=true;//false;
	opt->rgbinput=true;
	if(opt->ddump){
		opt->im_width=1920;
		opt->im_height=1080;
		opt->win_width=0;//720;
		opt->win_height=0;//360;
	}
	else{
		
		opt->im_width=320;//640;//1280;//1920;
		opt->im_height=240;//480;//960;//1080;
		opt->win_width=0;//640;//1440;//1920;//1280;//640;//1440;//1920;//1280;//640;//960;//640;//1920;;//
		opt->win_height=0;//480;//720;//1080;//960;//480;//720;//1080;//960;//480;//540;//480;// 1080;
	}
}


/**
 * Parse the command line arguments and assign the results to the option structure.
 * @param opt data structure that contains all user options.
 * @param argc number of commmand line arguments.
 * @param argv array of command line argument strings.
 * @return 0 on success or negative on error parsing arguments.
 */
int get_options(struct options *opt, int argc, char * const argv[])
{
	int o;
	int found = false;
	struct usage *program_use;
	static struct option long_options[] = {
		{"device", 			required_argument, 	0, CAPTURE_DEV  },
		{"subdevice", 		required_argument, 	0, CAPTURE_SUBDEV  },
		{"count", 			required_argument,	0, CAPTURE_COUNT },
		{"usage",			required_argument,	0, PROGRAM_USE },
		{"help",			no_argument, 		0, 'h'},
		{"verbose",			optional_argument,	0, 'v'},
		{"eglimage",			required_argument,	0, 'e'},
		{"win_width",			required_argument,	0, '0'},
		{"win_height",			required_argument,	0, '1'},
		{"im_width",			required_argument,	0, '2'},
		{"im_height",			required_argument,	0, '3'},
		{"rgbtext",			required_argument,	0, '4'},
		{"rgbinput",			required_argument,	0, '5'},
		{0},
	};

	set_default_options(opt);

	while(1)
	{	// : means required_argument
		o = getopt_long(argc, argv, "d:s:p:n:u:hve:0:1:2:3:4:5:", long_options, NULL);
		if (o == -1) break;

		switch (o)
		{
			case 	'e':
				if(atoi(optarg)==1)
					opt->eglimage=false;
				break;
			
			
			case 	'0':
				opt->win_width=atoi(optarg);
				printf("win width=%d\n",opt->win_width);
				break;
			
			case 	'1':
				opt->win_height=atoi(optarg);
				printf("win height=%d\n",opt->win_height);
				break;
			
			case 	'2':
				opt->im_width=atoi(optarg);
				break;
			
			case 	'3':
				opt->im_height=atoi(optarg);
				break;

			case 	'4':
				if(atoi(optarg)==1)
					opt->rgbtext=false;
				break;
			case 	'5':
				if(atoi(optarg)==1)
					opt->rgbinput=false;
				break;
			
			case CAPTURE_DEV:
				opt->dev_name = optarg;
				printf("111111111111%s\n",opt->dev_name);
				break;

			case CAPTURE_SUBDEV:
				opt->subdev_name = optarg;
				break;

			case CAPTURE_COUNT:
				opt->capture_count = atoi(optarg);
				if (opt->capture_count <= 0)
				{
					LOGS_ERR("Unable to set capture count to %d using default %d",
						opt->capture_count, DEFAULT_COUNT);
					opt->capture_count = DEFAULT_COUNT;
				}
				break;

			case PROGRAM_USE:
				/*
				 * Search the queue for the user requested progam use.
				 * If a matching name is found save the pointer to that routine.
				 */
				TAILQ_FOREACH(program_use, &g_program_options.usage_head, usage_entry)
				{
					if (strcmp(optarg, program_use->name) == 0)
					{
						found = true;
						opt->program_use = program_use;
						break;
					}
				}
				if (!found)
				{
					printf("unknown program use %s\n", optarg);
					usage(argv);
					return -1;
				}
				break;

			case 'v':
				if (optarg) VERBOSE = atoi(optarg);
				else   		VERBOSE = LOG_ALL;
				break;

			case '?':
			case 'h':
				usage(argv);
				exit(0);
				break;

			default:
				printf("unknown argument %c", o);
				usage(argv);
				return -1;
		}
	}
	return 0;
}

/**
 * Parse command line options and run the test that the user selects.
 */
int main(int argc, char * const argv[])
{
	struct capture_context cap_context;
	struct capture_context *cap = &cap_context;
	struct display_context disp_context;
	struct display_context *disp = &disp_context;
	int ret = 0;
	
	
	memset(disp, 0, sizeof(*disp));
	memset(cap, 0, sizeof(*cap));

	ret = get_options(&g_program_options, argc, argv);
	if (ret) return ret;
	system("/home/linaro/410c_camera_support/opengles_capture/cam_reset.sh");
	struct options *opt = &g_program_options ;
	char * mydev= opt->dev_name;
	mydev+=10;
	if(*mydev=='3'){
		opt->ddump=false;
		system("/home/linaro/410c_camera_support/opengles_capture/nv12_320.sh");
		opt->im_width=320;//640;//1280;
		opt->im_height=240;//480;//960;
		if(opt->win_width==0)
			opt->win_width=320;//640;
		if(opt->win_height==0)
			opt->win_height=240;//480;
	}	
	else if (*mydev=='0'){
		opt->ddump=true;
		system("/home/linaro/410c_camera_support/opengles_capture/ddump.sh");
		opt->im_width=1920;
		opt->im_height=1080;
		if(opt->win_width==0)
			opt->win_width=960;
		if(opt->win_height==0)
			opt->win_height=540;
	}
	else {
		opt->ddump=true;
		system("/home/linaro/410c_camera_support/opengles_capture/ddump.sh");
		opt->im_width=640;
		opt->im_height=480;
		opt->splane=true;
		if(opt->win_width==0)
			opt->win_width=640;
		if(opt->win_height==0)
			opt->win_height=480;
	}
	
	
//---------------------------------------------------------------------------------------
  // Graph definition from unzipped https://storage.googleapis.com/download.tensorflow.org/models/inception5h.zip
  // which is used in the Go, Java and Android examples                                   
  TF_Buffer* graph_def = read_file("/home/linaro/label/output_graph.pb");                      
  TF_Graph* graph = TF_NewGraph();
// Import graph_def into graph                                                          
  TF_Status* status = TF_NewStatus();                                                     
  TF_ImportGraphDefOptions* opts = TF_NewImportGraphDefOptions();                         
  TF_GraphImportGraphDef(graph, graph_def, opts, status);
  TF_DeleteImportGraphDefOptions(opts);
  if (TF_GetCode(status) != TF_OK) {
          fprintf(stderr, "ERROR: Unable to import graph %s", TF_Message(status));        
          return 1;
  }       
  fprintf(stdout, "Successfully imported graph");
// Setup graph inputs
std::vector<TF_Output> inputs;
std::vector<TF_Tensor*> input_values;

// Add the placeholders you would like to feed, e.g.:
TF_Operation* placeholder = TF_GraphOperationByName(graph, "input");
inputs.push_back({placeholder, 0});

// Create a new tensor pointing to that memory:
int 	inputwidth=224;
int	inputheight=224;
int 	*imNumPt = new int(1);
const 	int64_t tensorDims[4] = {1,inputheight,inputwidth,3};
const 	int 	num_bytes=inputheight * inputwidth * 3* sizeof(float);
const int num_bytes_out = 5 * sizeof(float);

int64_t out_dims[] = {1, 5};
//float mmf[224*224*3];

TF_Tensor* tensor = TF_NewTensor(TF_FLOAT, tensorDims, 4, &disp->render_ctx.f_rgbbuf[0],num_bytes , NULL, imNumPt);
input_values.push_back(tensor);
// Optionally, you can check that your input_op and input tensors are correct
  // by using some of the functions provided by the C API.
  std::cout << "Input op info: " << TF_OperationNumOutputs(placeholder) << "\n";
  std::cout << "Input data info: " << TF_Dim(tensor, 0) << "\n";
fprintf(stdout, "success 1"); 

// Setup graph outputs
std::vector<TF_Output> outputs;
// Add the node outputs you would like to fetch, e.g.:
TF_Operation* output_op = TF_GraphOperationByName(graph, "final_result");
//fprintf(stdout, "success 2");
outputs.push_back({output_op, 0});
//fprintf(stdout, "success 3");
std::vector<TF_Tensor*> output_values(outputs.size(), nullptr);

// Similar to creating the input tensor, however here we don't yet have the
  // output values, so we use TF_AllocateTensor()
  TF_Tensor* output_value = TF_AllocateTensor(TF_FLOAT, out_dims, 2, num_bytes_out);
  output_values.push_back(output_value);

  // As with inputs, check the values for the output operation and output tensor
  std::cout << "Output: " << TF_OperationName(output_op) << "\n";
  std::cout << "Output info: " << TF_Dim(output_value, 0) << "\n";


// Run `graph`
TF_SessionOptions* sess_opts = TF_NewSessionOptions();
TF_Session* session = TF_NewSession(graph, sess_opts, status);
//assert(TF_GetCode(status) == TF_OK);
if (TF_GetCode(status) != TF_OK) {
          fprintf(stderr, "ERROR: Unable to new session %s", TF_Message(status));        
          return 1;
  }
  /*
TF_SessionRun(session, nullptr, &inputs[0], &input_values[0], inputs.size(),
              &outputs[0], &output_values[0], outputs.size(), nullptr, 0, nullptr, status);
  // Assign the values from the output tensor to a variable and iterate over them
  float* out_vals = static_cast<float*>(TF_TensorData(output_values[0]));
  for (int i = 0; i < 5; ++i)
  {
      std::cout << "Output values info: " << *out_vals++ << "\n";
  }

  fprintf(stdout, "Successfully run session\n");

//void* output_data = TF_TensorData(output_values[0]);
//assert(TF_GetCode(status) == TF_OK);
if (TF_GetCode(status) != TF_OK) {
          fprintf(stderr, "ERROR: Unable to run session %s", TF_Message(status));        
          return 1;
  } 
  TF_CloseSession( session, status );
  TF_DeleteSession( session, status );
  TF_DeleteSessionOptions( sess_opts );      
  TF_DeleteStatus(status);
  TF_DeleteImportGraphDefOptions(opts);
  TF_DeleteGraph(graph);                                                                  
  return 0;
  */
//---------------------------------------------------------------------------------------	
  /*
TF_Status* s = TF_NewStatus();
TF_Status * status = TF_NewStatus();
  
	
TF_Graph* graph = TF_NewGraph();
	
  TF_SessionOptions * options = TF_NewSessionOptions();
  TF_Session * session = TF_NewSession( graph, options, status );
  char hello[] = "Hello TensorFlow!";
  TF_Tensor * tensor = TF_AllocateTensor( TF_STRING, 0, 0, 8 + TF_StringEncodedSize( strlen( hello ) ) );
  TF_Tensor * tensorOutput;
  TF_OperationDescription * operationDescription = TF_NewOperation( graph, "Const", "hello" );
  TF_Operation * operation; 
  struct TF_Output output;

  TF_StringEncode( hello, strlen( hello ), 8 + ( char * ) TF_TensorData( tensor ), TF_StringEncodedSize( strlen( hello ) ), status );
  memset( TF_TensorData( tensor ), 0, 8 );
  TF_SetAttrTensor( operationDescription, "value", tensor, status );
  TF_SetAttrType( operationDescription, "dtype", TF_TensorType( tensor ) );
  operation = TF_FinishOperation( operationDescription, status );

  output.oper = operation;
  output.index = 0;

  TF_SessionRun( session, 0,
                 0, 0, 0,  // Inputs
                 &output, &tensorOutput, 1,  // Outputs
                 &operation, 1,  // Operations
                 0, status );

  printf( "status code: %i\n", TF_GetCode( status ) );
  printf( "%s\n", ( ( char * ) TF_TensorData( tensorOutput ) ) + 9 );

  TF_CloseSession( session, status );
  TF_DeleteSession( session, status );
  TF_DeleteStatus( status );
  TF_DeleteSessionOptions( options );  

  return 0;
  
}



/*
//------------------------------------------------------------------------------------------------------------------
TensorBuffer* 	TensorCApi::Buffer(const Tensor& tensor) 
		{ 
			return tensor.buf_; 
		}
Tensor 		TensorCApi::MakeTensor(TF_DataType type, const TensorShape& shape, TensorBuffer* buf) 
		{
			return Tensor(static_cast<DataType>(type), shape, buf);
		}
// Create an empty tensor of type 'dtype'. 'shape' can be arbitrary, but has to
 // result in a zero-sized tensor.
class TensorCApi {
public:
 static TensorBuffer* Buffer(const Tensor& tensor);
 static Tensor MakeTensor(TF_DataType type, const TensorShape& shape, TensorBuffer* buf);
}
// Put an image in the cameraImg mat
cv::resize(image->image, cameraImg, cv::Size(inputwidth, inputheight), 0, 0, cv::INTER_AREA);
// Create a new tensor pointing to that memory:
const int64_t tensorDims[4] = {1,inputheight,inputwidth,3};
int *imNumPt = new int(1);
TF_Tensor* tftensor = TF_NewTensor(TF_DataType::TF_UINT8, tensorDims, 4, cameraImg.data, inputheight * inputwidth * 3, NULL, imNumPt);
Tensor inputImg = tensorflow::TensorCApi::MakeTensor(tftensor->dtype, tftensor->shape, tftensor->buffer);
//------------------------------------------------------------------------------------------------------------------

#include <stdio.h>                                                                        
#include <stdlib.h>                                                                       
#include <tensorflow/c/c_api.h>                                                           


TF_Buffer* read_file(const char* file);                                                   

void free_buffer(void* data, size_t length) {                                             
        free(data);                                                                       
}                                              

// Graph definition from unzipped https://storage.googleapis.com/download.tensorflow.org/models/inception5h.zip
  // which is used in the Go, Java and Android examples                                   
  TF_Buffer* graph_def = read_file("tensorflow_inception_graph.pb");                      
  TF_Graph* graph = TF_NewGraph();
// Import graph_def into graph                                                          
  TF_Status* status = TF_NewStatus();                                                     
  TF_ImportGraphDefOptions* opts = TF_NewImportGraphDefOptions();                         
  TF_GraphImportGraphDef(graph, graph_def, opts, status);
  TF_DeleteImportGraphDefOptions(opts);
  if (TF_GetCode(status) != TF_OK) {
          fprintf(stderr, "ERROR: Unable to import graph %s", TF_Message(status));        
          return 1;
  }       
  fprintf(stdout, "Successfully imported graph");                                         
  TF_DeleteStatus(status);
  TF_DeleteBuffer(graph_def);                                                             

  // Use the graph                                                                        
  TF_DeleteGraph(graph);                                                                  
  return 0;



//------------------------------------------------------------------------------------------------------------------
const char* graph_def_data; // <-- your serialized GraphDef here
TF_Buffer graph_def = {graph_def_data, strlen(graph_def_data), nullptr};
// Import `graph_def` into `graph`
TF_ImportGraphDefOptions* import_opts = TF_NewImportGraphDefOptions();
TF_ImportGraphDefOptionsSetPrefix(import_opts, "import");
TF_GraphImportGraphDef(graph, &graph_def, import_opts, s);
assert(TF_GetCode(s) == TF_OK);

// Setup graph inputs
std::vector<TF_Output> inputs;
std::vector<TF_Tensor*> input_values;
// Add the placeholders you would like to feed, e.g.:
TF_Operation* placeholder = TF_GraphOperationByName(graph, "import/my_placeholder");
inputs.push_back({placeholder, 0});
TF_Tensor* tensor = TF_NewTensor(/*...*/
/*
input_values.push_back(tensor);

// Setup graph outputs
std::vector<TF_Output> outputs;
// Add the node outputs you would like to fetch, e.g.:
TF_Operation* output_op = TF_GraphOperationByName(graph, "import/my_output");
outputs.push_back({output_op, 0});
std::vector<TF_Tensor*> output_values(outputs.size(), nullptr);

// Run `graph`
TF_SessionOptions* sess_opts = TF_NewSessionOptions();
TF_Session* session = TF_NewSession(graph, sess_opts, s);
assert(TF_GetCode(s) == TF_OK);
TF_SessionRun(session, nullptr,
              &inputs[0], &input_values[0], inputs.size(),
              &outputs[0], &output_values[0], outputs.size(),
              nullptr, 0, nullptr, s);

void* output_data = TF_TensorData(output_values[0]);
assert(TF_GetCode(s) == TF_OK);

// If you have a more complicated workflow, I suggest making scoped wrapper
// classes that call these in their destructors
for (int i = 0; i < inputs.size(); ++i) TF_DeleteTensor(input_values[i]);
for (int i = 0; i < outputs.size(); ++i) TF_DeleteTensor(output_values[i]);
TF_CloseSession(session, s);
TF_DeleteSession(session, s);
TF_DeleteSessionOptions(sess_opts);
TF_DeleteImportGraphDefOptions(import_opts);
TF_DeleteGraph(graph);
TF_DeleteStatus(s);
*/
	ret = g_program_options.program_use->function(cap, disp, &g_program_options,
		session, &inputs[0], &input_values[0], inputs.size(),
		&outputs[0], &output_values[0], outputs.size());
TF_CloseSession( session, status );
  TF_DeleteSession( session, status );
  TF_DeleteSessionOptions( sess_opts );      
  TF_DeleteStatus(status);
  TF_DeleteImportGraphDefOptions(opts);
  TF_DeleteGraph(graph);        
	return ret;
}

