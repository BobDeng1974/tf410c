//ali

#include <assert.h>
#include <iostream>
#include <vector>
#include "tensorflow/c/c_api.h"
#include "tfdragon.h"


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

//Dragon_TFSession DrTfSession;


//ali */
int Dragon_TFSession::LoadGraph(float* pfdata)
{
	//---------------------------------------------------------------------------------------
	// Graph definition from unzipped https://storage.googleapis.com/download.tensorflow.org/models/inception5h.zip
	// which is used in the Go, Java and Android examples                                   
	TF_Buffer* graph_def = read_file("/home/linaro/label/output_graph.pb");                      
	TF_Graph* graph = TF_NewGraph();
	pdata=pfdata;
	// Import graph_def into graph                                                          
	TF_Status* status = TF_NewStatus();                                                     
	TF_ImportGraphDefOptions* opts = TF_NewImportGraphDefOptions();                         
	TF_GraphImportGraphDef(graph, graph_def, opts, status);
	TF_DeleteImportGraphDefOptions(opts);
	if (TF_GetCode(status) != TF_OK) {
		fprintf(stderr, "ERROR: Unable to import graph %s", TF_Message(status));        
		return -1;
	}       
	fprintf(stdout, "Successfully imported graph");
	/*
	// Setup graph inputs
	std::vector<TF_Output> inputs;
	std::vector<TF_Tensor*> input_values;
	*/
	// Add the placeholders you would like to feed, e.g.:
	TF_Operation* placeholder = TF_GraphOperationByName(graph, "input");
	inputs.push_back({placeholder, 0});
	ninputs=inputs.size();
	// Create a new tensor pointing to that memory:
	int 	inputwidth=224;
	int	inputheight=224;
	int 	*imNumPt = new int(1);
	const 	int64_t tensorDims[4] = {1,inputheight,inputwidth,3};
	const 	int 	num_bytes=inputheight * inputwidth * 3* sizeof(float);
	const 	int 	num_bytes_out = 5 * sizeof(float);

	int64_t out_dims[] = {1, 5};
	//float *pdata = new float[224*224*3];

	TF_Tensor* tensor = TF_NewTensor(TF_FLOAT, tensorDims, 4, pfdata/*&disp->render_ctx.f_rgbbuf[0]*/,num_bytes , NULL, imNumPt);
	input_values.push_back(tensor);
	// Optionally, you can check that your input_op and input tensors are correct
	  // by using some of the functions provided by the C API.
	std::cout << "Input op info: " << TF_OperationNumOutputs(placeholder) << "\n";
	std::cout << "Input data info: " << TF_Dim(tensor, 0) << "\n";
	fprintf(stdout, "success 1"); 

	// Setup graph outputs
	//std::vector<TF_Output> outputs;
	// Add the node outputs you would like to fetch, e.g.:
	TF_Operation* output_op = TF_GraphOperationByName(graph, "final_result");
	//fprintf(stdout, "success 2");
	outputs.push_back({output_op, 0});
	//fprintf(stdout, "success 3");
	output_values.resize(outputs.size(), nullptr);
	noutputs=outputs.size();
	// Similar to creating the input tensor, however here we don't yet have the
	// output values, so we use TF_AllocateTensor()
	TF_Tensor* output_value = TF_AllocateTensor(TF_FLOAT, out_dims, 2, num_bytes_out);
	output_values.push_back(output_value);

	// As with inputs, check the values for the output operation and output tensor
	std::cout << "Output: " << TF_OperationName(output_op) << "\n";
	std::cout << "Output info: " << TF_Dim(output_value, 0) << "\n";


	// Run `graph`
	TF_SessionOptions* sess_opts = TF_NewSessionOptions();
	session = TF_NewSession(graph, sess_opts, status);
	//assert(TF_GetCode(status) == TF_OK);
	if (TF_GetCode(status) != TF_OK) {
		  fprintf(stderr, "ERROR: Unable to new session %s", TF_Message(status));        
		  return 1;
	}
}