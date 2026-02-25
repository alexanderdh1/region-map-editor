#pragma once

struct GLFWwindow;

GLFWwindow* createWindow(int width, int height, const char* title);
void destroyWindow(GLFWwindow* window);