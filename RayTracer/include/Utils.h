#ifndef UTILS_H
#define UTILS_H


//Used for outputting error messages
void error_callback(int err_code, const char* err_str);

//Called after initiation every FrameBuffer, displays the status and outputs false if there's a problem
bool check_FB_Status();


void init_Quad(unsigned int shaderID, unsigned int  & quadVAO, unsigned int  & quadVBO);

#endif